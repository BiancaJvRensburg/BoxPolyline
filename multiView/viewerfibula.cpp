#include "viewerfibula.h"

ViewerFibula::ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffset) : Viewer (parent, camera, sliderMax)
{
    polyRotation = 0;
}

// Set the start of the polyline and update the distances.
void ViewerFibula::updateFibPolyline(const Vec& firstPoint, const std::vector<double>& distances){
    poly.setFirstPoint(firstPoint);
    setDistances(distances);
}

// Reset distances and place the planes on the updated polyline. Note: this is the end of the initial cut procedure
void ViewerFibula::updateDistances(const std::vector<double>& distances){
    setDistances(distances);
    repositionPlanesOnPolyline();
}

// Re-initialise the polyline to a straight line with corresponding distances. TODO don't reset it in a straight line but along the current tragectory
void ViewerFibula::setDistances(const std::vector<double> &distances){
    std::vector<Vec> newPoints;
    newPoints.push_back(Vec(0,0,0));
    for(unsigned int i=0; i<distances.size(); i++) newPoints.push_back(newPoints[i] + Vec(distances[i], 0, 0));
    updatePolyline(newPoints);
}

// This is activated by the mandible
void ViewerFibula::bendPolylineNormals(std::vector<Vec>& normals, const std::vector<double>& distances){
    updateFibPolyline(poly.getMeshPoint(0), distances);     // Reset the distances

    // Save the normals for when we want to move the polyline by hand
    planeNormals.clear();
    for(unsigned int i=0; i<normals.size(); i++) planeNormals.push_back(normals[i]);

    setPlanesInPolyline(normals);
}

// This is activated when a plane in the fibula is moved by hand
void ViewerFibula::bendPolyline(unsigned int id, Vec v){
    poly.bendFibula(id, v);     // Bend the polyline

    // Get the normals
    std::vector<Vec> normals;
    for(unsigned int i=0; i<planeNormals.size(); i++) normals.push_back(planeNormals[i]);

    setPlanesInPolyline(normals);       // Set the planes

    //poly.restoreBoxRotations();
}

// Match the planes with the polyline
void ViewerFibula::setPlanesInPolyline(std::vector<Vec> &normals){
    setPlaneOrientations(normals);
    repositionPlanesOnPolyline();
    update();
}

void ViewerFibula::setPlaneOrientations(std::vector<Vec> &normals){
    poly.getRelatvieNormals(normals);       // Convert the normals from being relative to the boxes to world coordinates to set the planes' orientations

    for(unsigned int i=2; i<normals.size()-2; i+=2) ghostPlanes[i/2-1]->setFrameFromBasis(normals[i], normals[i+1], cross(normals[i], normals[i+1]));
    leftPlane->setFrameFromBasis(normals[0],normals[1],cross(normals[0],normals[1]));
    unsigned long long lastIndex = normals.size()-2;
    rightPlane->setFrameFromBasis(normals[lastIndex],normals[lastIndex+1],cross(normals[lastIndex],normals[lastIndex+1]));
}

void ViewerFibula::updatePlaneOrientations(std::vector<Vec> &normals){
    setPlaneOrientations(normals);
    update();
}

void ViewerFibula::initGhostPlanes(Movable s){
    deleteGhostPlanes();        // delete any previous ghost planes
    double size = leftPlane->getSize();
    for(unsigned int i=0; i<(poly.getNbPoints()-4)*2; i++){     // -2 for total nb of planes, another -2 for nb of ghost planes
        Plane *p1 = new Plane(size, s, .5f, i/2+2);
        ghostPlanes.push_back(p1);
        ghostPlanes[i]->setPosition(poly.getMeshPoint((i+2)/2));
        ghostPlanes[i]->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
    }

    // Connect the planes' movement. If bend the polyline if a plane is moved by hand.
    connect(&(leftPlane->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &ViewerFibula::bendPolyline);
    connect(&(rightPlane->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &ViewerFibula::bendPolyline);
    for(unsigned int i=0; i<ghostPlanes.size(); i++) connect(&(ghostPlanes[i]->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &ViewerFibula::bendPolyline);        // connnect the ghost planes
}

void ViewerFibula::constructCurve(){
    nbU = 2000;
    curve.init(control.size(), control);
    curve.generateCatmull(nbU);

    isCurve = true;
    initCurvePlanes(Movable::STATIC);
}

void ViewerFibula::toggleIsPolyline(){
    Vec n = poly.getWorldTransform(poly.getNormal()-poly.getBinormal());

    // Lower the polyline to where the bottom corner of each plane is
    if(!leftPlane->getIsPoly()) for(unsigned int i=0; i<poly.getNbPoints(); i++) poly.lowerPoint(i, n*leftPlane->getSize());
    else for(unsigned int i=0; i<poly.getNbPoints(); i++) poly.lowerPoint(i, -n*leftPlane->getSize());

    // Change the way the position of the curve point in the plane
    leftPlane->toggleIsPoly();
    rightPlane->toggleIsPoly();
    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->toggleIsPoly();

    repositionPlanesOnPolyline();       // The planes have to be reset to their new polyline positions
}

// Position the planes on their corresponding polyline points
void ViewerFibula::repositionPlanesOnPolyline(){
    leftPlane->setPosition(poly.getMeshPoint(leftPlane->getID()));
    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->setPosition(poly.getMeshPoint(ghostPlanes[i]->getID())); //ghostPlanes[i]->setPosition(poly.getMeshPoint((ghostPlanes[i]->getID()-1)/2+2));
    rightPlane->setPosition(poly.getMeshPoint(rightPlane->getID()));
}

void ViewerFibula::constructPolyline(const std::vector<double>& distances, const std::vector<Vec>& newPoints){
    isCut = true;       // if we have a polyline, it means that the fibula is cut. TODO The location of this may change when we actually cut the mesh
    rotatePolyline();   // Rotate the polyline so it matches the fibula as closely as it can

    poly.reinit(newPoints.size());      // Initialise the polyline
    updateFibPolyline(curve.getPoint(curveIndexL), distances);      // Make the polyline start at the left plane (which doesn't move) TODO change this so the left plane in the polyline doesnt move. The start must be at the start of the mesh.

    initPolyPlanes(Movable::DYNAMIC);       // Set the planes on the polyline

    toggleIsPolyline();     // The polyline is currently going through the centre of the planes, change it to the corner

    Q_EMIT okToPlacePlanes(newPoints);      // Tell the mandible it can now place its planes on the polyline. (Send its points back).
}

// Move the planes when its not cut (when the polyline doesn't exist)
void ViewerFibula::movePlanes(double distance){
    curveIndexR = curve.indexForLength(curveIndexL, distance);
    repositionPlane(rightPlane, curveIndexR);
    update();
}

// Rotate the polyline to match the fibula curve
void ViewerFibula::rotatePolyline(){
    Vec v = curve.getPoint(nbU-1) - curve.getPoint(0);      // Get the vector which best represents the polyline curve
    double alpha = angle(poly.getWorldTransform(poly.getTangent()), v);     // Get the angle between the curve vector and the polyline's current position
    Quaternion q(-poly.getBinormal(), alpha);
    poly.rotate(q);     // Rotate the polyline around its binormal to match the curve
}

// Rotate the boxes
void ViewerFibula::rotatePolylineOnAxisFibula(double r){
    for(unsigned int i=0; i<poly.getNbPoints()-1; i++) poly.rotateBox(i, r);
    update();
}

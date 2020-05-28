#include "viewerfibula.h"

ViewerFibula::ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffset) : Viewer (parent, camera, sliderMax)
{
    polyRotation = 0;
}

void ViewerFibula::initSignals(){
    connect(&mesh, &Mesh::sendInfoToManible, this, &ViewerFibula::recieveFromFibulaMesh);
}

// Set the start of the polyline and update the distances.
void ViewerFibula::updateFibPolyline(const Vec& firstPoint, const std::vector<double>& distances){
    poly.setFirstPoint(firstPoint);
    setDistances(distances);
}

// Reset distances and place the planes on the updated polyline. Note: this is the end of the initial cut procedure
void ViewerFibula::updateDistances(const std::vector<double>& distances){
    setDistances(distances);
    projectToMesh(distances);
    repositionPlanesOnPolyline();

    //offsetPolyline();       // set the space between the ghost planes to 1mm

    positionBoxes();
}

// Re-initialise the polyline to a straight line with corresponding distances
void ViewerFibula::setDistances(const std::vector<double> &distances){
    Vec direction(1,0,0);

    std::vector<Vec> newPoints;
    newPoints.push_back(Vec(0,0,0));
    for(unsigned int i=0; i<distances.size(); i++) newPoints.push_back(newPoints[i] + distances[i]*direction);
    updatePolyline(newPoints);
    //poly.resetBoxes();
}

// This is activated by the mandible
void ViewerFibula::bendPolylineNormals(std::vector<Vec>& normals, const std::vector<double>& distances){
    updateFibPolyline(poly.getMeshPoint(0), distances);     // Reset the distances

    // Save the normals for when we want to move the polyline by hand
    planeNormals.clear();
    for(unsigned int i=0; i<normals.size(); i++) planeNormals.push_back(normals[i]);

    projectToMesh(distances);

    setPlanesInPolyline(normals);

    //offsetPolyline();       // set the space between the ghost planes to 1mm
    positionBoxes();
}

// This is activated when a plane in the fibula is bent (so when its projected onto the mesh)
void ViewerFibula::bendPolyline(unsigned int id, Vec v){
    poly.bendFibula(id, v);     // Bend the polyline
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

// Update the relationship between the planes and the boxes when the mandible boxes are rotated
void ViewerFibula::updatePlaneOrientations(std::vector<Vec> &normals){
    //planeNormals.clear();
    //for(unsigned int i=0; i<normals.size(); i++) planeNormals.push_back(normals[i]);
    setPlanesInPolyline(normals);
}

void ViewerFibula::initGhostPlanes(Movable s){
    deleteGhostPlanes();        // delete any previous ghost planes
    //double size = leftPlane->getSize();
    double size = 20.;
    for(unsigned int i=0; i<(poly.getNbPoints()-4)*2; i++){     // -2 for total nb of planes, another -2 for nb of ghost planes
        Plane *p1 = new Plane(size, s, .5f, (i+1)/2+1);
        ghostPlanes.push_back(p1);
        //ghostPlanes[i]->setPosition(poly.getMeshPoint((i+2)/2));
        ghostPlanes[i]->setPosition(poly.getMeshBoxPoint((i+2)/2));
        ghostPlanes[i]->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
    }

    leftPlane->setSize(size);
    rightPlane->setSize(size);

    // Connect the planes' movement. If bend the polyline if a plane is moved by hand.
    /*connect(&(leftPlane->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &ViewerFibula::bendPolyline);
    connect(&(rightPlane->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &ViewerFibula::bendPolyline);*/
    // for(unsigned int i=0; i<ghostPlanes.size(); i++) connect(&(ghostPlanes[i]->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &ViewerFibula::bendPolyline);        // connnect the ghost planes
}

void ViewerFibula::constructCurve(){
    nbU = 2000;
    curve.init(control.size(), control);
    curve.generateCatmull(nbU);

    isCurve = true;
    initCurvePlanes(Movable::STATIC);
}

void ViewerFibula::toggleIsPolyline(){
    Vec n(0,1,1); //= poly.getWorldTransform(poly.getNormal());//-poly.getBinormal());
    n.normalize();

    // Lower the polyline to where the bottom corner of each plane is
    if(!leftPlane->getIsPoly()) for(unsigned int i=0; i<poly.getNbPoints(); i++) poly.lowerPoint(i, n*leftPlane->getSize());
    else for(unsigned int i=0; i<poly.getNbPoints(); i++) poly.lowerPoint(i, -n*leftPlane->getSize());

    // Change the way the position of the curve point in the plane
    leftPlane->toggleIsPoly();
    rightPlane->toggleIsPoly();
    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->toggleIsPoly();

    repositionPlanesOnPolyline();       // The planes have to be reset to their new polyline positions
}

// Project the polyline onto the fibula mesh
void ViewerFibula::projectToMesh(const std::vector<double>& distances){
    rotatePolyToCurve();        // rotate the polyline so its facing the centre of the block

    unsigned int nbU = 50;
    constructSegmentPoints(nbU);     // construct the new polyline with segments

    std::vector<Vec> outputPoints;
    mesh.mlsProjection(segmentPoints, outputPoints);

    std::vector<unsigned int> segIndexes;
    for(unsigned int i=0; i<poly.getNbPoints(); i++) segIndexes.push_back(i*nbU);

    double epsilon = 0.05;
    unsigned int maxIterations = 10;
    matchDistances(distances, segIndexes, outputPoints, epsilon, maxIterations);

    for(unsigned int i=0; i<poly.getNbPoints(); i++){
        bendPolyline(i, outputPoints[segIndexes[i]]);
    }

    // lower the entire polyline by 5mm
    //poly.lowerPolyline(Vec(0,-1,-1), 7.5);
}

// Check if the projected distances match the actual distance. If not, modify it
void ViewerFibula::matchDistances(const std::vector<double> &distances, std::vector<unsigned int> &segIndexes, std::vector<Vec> &outputPoints, double epsilon, const unsigned int &searchRadius){
    for(unsigned int i=1; i<poly.getNbPoints()-2; i++){
        segIndexes[i+1] = getClosestDistance(i, distances[i], segIndexes, outputPoints, searchRadius);
    }
}

// Get the closest index to the target distance. Search within an index radius
unsigned int ViewerFibula::getClosestDistance(unsigned int index, const double &targetDistance, std::vector<unsigned int> &segIndexes, std::vector<Vec> &outputPoints, unsigned int searchRadius){
    double minDist = DBL_MAX;
    unsigned int minIndex = segIndexes[index+1];
    unsigned int indexDist = segIndexes[index+1] - segIndexes[index];
    if(indexDist < searchRadius) searchRadius = indexDist;      // if the search radius is too large

    for(unsigned int i=segIndexes[index+1]-searchRadius; i<segIndexes[index+1]+searchRadius; i++){
        double d = abs(euclideanDistance(outputPoints[segIndexes[index]], outputPoints[i]) - targetDistance);
        if(minDist > d){
            minDist = d;
            minIndex = i;
        }
    }

    return minIndex;
}

// Position the planes on their corresponding polyline points
void ViewerFibula::repositionPlanesOnPolyline(){
    leftPlane->setPosition(poly.getMeshBoxPoint(leftPlane->getID()));
    for(unsigned int i=0; i<ghostPlanes.size(); i+=2){
        ghostPlanes[i]->setPosition(poly.getMeshBoxEnd(ghostPlanes[i]->getID()));
        ghostPlanes[i+1]->setPosition(poly.getMeshBoxPoint(ghostPlanes[i+1]->getID()));
    }
    rightPlane->setPosition(poly.getMeshBoxPoint(rightPlane->getID()));
}

void ViewerFibula::constructPolyline(const std::vector<double>& distances, const std::vector<Vec>& newPoints){
    isCut = true;       // if we have a polyline, it means that the fibula is cut. TODO The location of this may change when we actually cut the mesh
    rotatePolyline();   // Rotate the polyline so it matches the fibula as closely as it can

    poly.reinit(newPoints.size());      // Initialise the polyline
    updateFibPolyline(curve.getPoint(curveIndexL), distances);      // Make the polyline start at the left plane (which doesn't move) TODO change this so the left plane in the polyline doesnt move. The start must be at the start of the mesh.

    initPolyPlanes(Movable::DYNAMIC);       // Set the planes on the polyline

   toggleIsPolyline();     // The polyline is currently going through the centre of the planes, change it to the corner

   //rotatePolyToCurve();

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

    //rotatePolyToCurve();
}

// Rotate the polyline to its angled towards the curve
void ViewerFibula::rotatePolyToCurve(){
    Vec fp = poly.getLocalCoordinates(curve.getPoint(0));
    // set the x coordinate to zero
    fp.x = 0;
    fp.normalize();
    /*Vec diagonal = poly.getNormal() - poly.getBinormal();
    diagonal.normalize();*/
    Vec diagonal(0,1,1);
    diagonal.normalize();
    double theta = angle(diagonal, fp);
    Quaternion r(poly.getTangent(), theta);
    poly.rotate(r);
}

// Rotate the boxes
void ViewerFibula::rotatePolylineOnAxisFibula(double r){
    bool isOriginallyCut = isCut;
    if(isOriginallyCut) uncut();
    //for(unsigned int i=0; i<poly.getNbPoints()-1; i++) poly.rotateBox(i, r);
    poly.rotateBox(2,r);
    if(isOriginallyCut) cut();
    update();
}

void ViewerFibula::rotatePolylineOnAxe(double r){
    bool isOriginallyCut = isCut;
    if(isOriginallyCut) uncut();
    r = r*M_PI/180.;
    r -= prevRotation;
    prevRotation = r;
    poly.rotateOnAxis(r, poly.getMeshBoxMiddle(0));
    if(isOriginallyCut) cut();
    update();
}

void ViewerFibula::constructSegmentPoints(unsigned int nbU){
    segmentPoints.clear();
    std::vector<Vec> directions;
    poly.getDirections(directions);

    for(unsigned int j=0; j<poly.getNbPoints()-1; j++){  // extend from the previous point
        double length = (poly.getMeshPoint(j) - poly.getMeshPoint(j+1)).norm();
        for(unsigned int i=0; i<nbU; i++){
            double u = static_cast<double>(i)/static_cast<double>(nbU);     // 0 <= u < 1
            Vec v = poly.getMeshPoint(j) + directions[j]*u*length;
            segmentPoints.push_back(v);
        }
    }

    // add the last control point point
    segmentPoints.push_back(poly.getMeshPoint(poly.getNbPoints()-1));
}

double ViewerFibula::euclideanDistance(const Vec &a, const Vec &b){
    return sqrt(pow(a.x-b.x, 2.) + pow(a.y-b.y, 2.) + pow(a.z-b.z, 2.));
}

void ViewerFibula::cut(){
    for(unsigned int i=0; i<ghostPlanes.size(); i++){
        mesh.addPlane(ghostPlanes[i]);
    }

    mesh.setIsCut(Side::EXTERIOR, true, true);
    isCut = true;

    update();
}

void ViewerFibula::recieveFromFibulaMesh(std::vector<int> &planes, std::vector<Vec> &verticies, std::vector<std::vector<int>> &triangles, std::vector<int>& colours, std::vector<Vec> &normals, int nbColours){
    // Convert to box coordinates
    for(unsigned int i=0; i<verticies.size(); i++){
        if(planes[i]==0){
            normals[i] = poly.getLocalBoxTransform(leftPlane->getID(), normals[i]);
            verticies[i] = poly.getLocalBoxCoordinates(leftPlane->getID(), verticies[i]);
        }
        else if(planes[i]==1){
            normals[i] = poly.getLocalBoxTransform(rightPlane->getID(), normals[i]);
            verticies[i] = poly.getLocalBoxCoordinates(rightPlane->getID(), verticies[i]);
        }
        else {
            int fibPlane = planes[i]-2;
            normals[i] = poly.getLocalBoxTransform(ghostPlanes[static_cast<unsigned int>(fibPlane)]->getID(), normals[i]);
            verticies[i] = poly.getLocalBoxCoordinates(ghostPlanes[static_cast<unsigned int>(fibPlane)]->getID(), verticies[i]);
        }
    }

    Q_EMIT sendToManible(planes, verticies, triangles, colours, normals, nbColours);
}

void ViewerFibula::uncut(){
    mesh.setIsCut(Side::EXTERIOR, false, false);
    isCut = false;

    update();
}

// Get the distance off the offset from the mesh. This is half the triangle.
double ViewerFibula::getOffsetDistance(double angle){
    return 0.5 / std::tan(angle);
}

// Get the angle between a plane and its corresponding box
double ViewerFibula::getBoxPlaneAngle(Plane &p){
    Vec binormal(0,1,0);
    return angle(p.getMeshVectorFromLocal(binormal), poly.getMeshBoxTransform(p.getID(), binormal));
}

// Move the polyline so it leaves a gap of 1mm between the two ghost planes
void ViewerFibula::offsetPolyline(){
    /*Vec direction =  -(poly.getNormal()+poly.getBinormal());
    direction.normalize();

    for(unsigned int i=0; i<ghostPlanes.size(); i+=2){
        double alpha = (getBoxPlaneAngle(*ghostPlanes[i]) + getBoxPlaneAngle(*ghostPlanes[i+1])) / 2.;     // get the average, the two should be the same in theory
        double distance = getOffsetDistance(alpha);     // get the distance by which we need to move the planes
        poly.lowerPoint(ghostPlanes[i+1]->getID(), direction*distance);
        Vec p = poly.getMeshPoint(ghostPlanes[i+1]->getID());
        bendPolyline(ghostPlanes[i+1]->getID(), p);
    }

    repositionPlanesOnPolyline();*/
}

Vec ViewerFibula::getOffsetDistanceToMeshBorder(std::vector<Vec> &projections, Plane &p){
    Vec minN(0,0,0);
    for(unsigned int i=0; i<projections.size(); i++){
        projections[i] = p.getLocalCoordinates(projections[i]);
        if(minN.x > projections[i].x) minN.x = projections[i].x;
        if(minN.y > projections[i].y) minN.y = projections[i].y;
    }

    minN = p.getMeshVectorFromLocal(minN);
    minN = poly.getLocalTransform(minN);

    minN.x = 0;

    return minN - Vec(0,0.001, 0.001);
}

// Bend the polyline with a vector defined in the polyline's frame
void ViewerFibula::bendWithRelativeVector(Plane &p, Vec v){
    poly.lowerPoint(p.getID(), v);
    Vec point = poly.getMeshPoint(p.getID());
    bendPolyline(p.getID(), point);
}

void ViewerFibula::positionBoxes(){
    for(unsigned int i=0; i<ghostPlanes.size(); i++){
        mesh.addPlane(ghostPlanes[i]);
    }

    std::vector<Vec> projections;
    if(ghostPlanes.size()!=0) projections = mesh.getMinNormalForPlane(0, 2);
    else projections = mesh.getMinNormalForPlane(0,1);
    Vec minN = getOffsetDistanceToMeshBorder(projections, *leftPlane);
    bendWithRelativeVector(*leftPlane, minN);


    for(unsigned int index=0; index<ghostPlanes.size(); index+=2){
        projections = mesh.getMinNormalForPlane(index+2, index+3);
        //testPoints = projections;

        minN = getOffsetDistanceToMeshBorder(projections, *ghostPlanes[index]);
        Vec direction =  -(poly.getNormal()+poly.getBinormal());
        direction.normalize();
        double alpha = (getBoxPlaneAngle(*ghostPlanes[index]) + getBoxPlaneAngle(*ghostPlanes[index+1])) / 2.;     // get the average, the two should be the same in theory
        double distance = getOffsetDistance(alpha);    // get the distance by which we need to move the planes

        bendWithRelativeVector(*ghostPlanes[index+1], direction*distance + minN);
    }

    if(ghostPlanes.size()!=0) projections = mesh.getMinNormalForPlane(1, ghostPlanes.size()+1);
    else projections = mesh.getMinNormalForPlane(1,0);

    minN = getOffsetDistanceToMeshBorder(projections, *rightPlane);
    bendWithRelativeVector(*rightPlane, minN);
    mesh.deleteGhostPlanes();

    Q_EMIT requestNewNorms();
}

void ViewerFibula::tryOffsetAngle(){

    update();
}

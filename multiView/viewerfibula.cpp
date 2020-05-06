#include "viewerfibula.h"

ViewerFibula::ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffset) : Viewer (parent, camera, sliderMax)
{
    polyRotation = 0;
}

void ViewerFibula::updateFibPolyline(const Vec& firstPoint, const std::vector<double>& distances){
    std::vector<Vec> newPoints;
    poly.setFirstPoint(firstPoint);
    newPoints.push_back(Vec(0,0,0));
    for(unsigned int i=0; i<distances.size(); i++) newPoints.push_back(newPoints[i] + Vec(distances[i], 0, 0));
    updatePolyline(newPoints);
}

void ViewerFibula::bendPolylineNormals(std::vector<Vec>& normals, const std::vector<double>& distances){
    updateFibPolyline(poly.getMeshPoint(0), distances);

    planeNormals.clear();
    for(unsigned int i=0; i<normals.size(); i++) planeNormals.push_back(normals[i]);

    setPlanesInPolyline(normals);
}

void ViewerFibula::bendPolyline(unsigned int id, Vec v){
    poly.bendFibula(id, v);

    std::vector<Vec> normals;
    for(unsigned int i=0; i<planeNormals.size(); i++) normals.push_back(planeNormals[i]);

    setPlanesInPolyline(normals);

    //poly.restoreBoxRotations();
}

void ViewerFibula::setPlanesInPolyline(std::vector<Vec> &normals){
    setPlaneOrientations(normals);
    repositionPlanesOnPolyline();
    update();
}

void ViewerFibula::setPlaneOrientations(std::vector<Vec> &normals){
    poly.getRelatvieNormals(normals);

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
    deleteGhostPlanes();
    double size = leftPlane->getSize();
    for(unsigned int i=0; i<(poly.getNbPoints()-4)*2; i++){     // -2 for total nb of planes, another -2 for nb of ghost planes
        Plane *p1 = new Plane(size, s, .5f, i/2+2);
        ghostPlanes.push_back(p1);
        ghostPlanes[i]->setPosition(poly.getMeshPoint((i+2)/2));
        ghostPlanes[i]->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
    }

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

    if(!leftPlane->getIsPoly()) for(unsigned int i=0; i<poly.getNbPoints(); i++) poly.lowerPoint(i, n*leftPlane->getSize());
    else for(unsigned int i=0; i<poly.getNbPoints(); i++) poly.lowerPoint(i, -n*leftPlane->getSize());

    leftPlane->toggleIsPoly();
    rightPlane->toggleIsPoly();
    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->toggleIsPoly();

    repositionPlanesOnPolyline();
}

void ViewerFibula::repositionPlanesOnPolyline(){
    leftPlane->setPosition(poly.getMeshPoint(leftPlane->getID()));
    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->setPosition(poly.getMeshPoint(ghostPlanes[i]->getID())); //ghostPlanes[i]->setPosition(poly.getMeshPoint((ghostPlanes[i]->getID()-1)/2+2));
    rightPlane->setPosition(poly.getMeshPoint(rightPlane->getID()));
}

void ViewerFibula::constructPolyline(const std::vector<double>& distances, const std::vector<Vec>& newPoints){
    isCut = true;
    rotatePolyline();

    poly.reinit(newPoints.size());
    updateFibPolyline(curve.getPoint(curveIndexL), distances);

    initPolyPlanes(Movable::DYNAMIC);

    toggleIsPolyline();

    Q_EMIT okToPlacePlanes(newPoints);
}

void ViewerFibula::updateDistances(const std::vector<double>& distances){
    std::vector<Vec> newPoints;
    newPoints.push_back(Vec(0,0,0));
    for(unsigned int i=0; i<distances.size(); i++) newPoints.push_back(newPoints[i] + Vec(distances[i], 0, 0));
    poly.updatePoints(newPoints);
    repositionPlanesOnPolyline();
}

void ViewerFibula::movePlanes(double distance){     // move the planes when its not cut
    curveIndexR = curve.indexForLength(curveIndexL, distance);
    repositionPlane(rightPlane, curveIndexR);
    update();
}

void ViewerFibula::rotatePolyline(){
    Vec v = curve.getPoint(nbU-1) - curve.getPoint(0);
    double alpha = angle(poly.getWorldTransform(poly.getTangent()), v);
    Quaternion q(-poly.getBinormal(), alpha);
    poly.rotate(q);
}

void ViewerFibula::rotatePolylineOnAxisFibula(double r){
    for(unsigned int i=0; i<poly.getNbPoints()-1; i++) poly.rotateBox(i, r);
    update();
}

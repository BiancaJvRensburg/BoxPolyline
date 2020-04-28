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

    poly.getRelatvieNormals(normals);

    for(unsigned int i=4; i<normals.size()-4; i+=2){
        ghostPlanes[i/2-2]->setFrameFromBasis(normals[i], normals[i+1], cross(normals[i], normals[i+1]));
        ghostPlanes[i/2-2]->setPosition(poly.getMeshPoint(ghostPlanes[i/2-2]->getID()));//(ghostPlanes[i/2-2]->getID()+3)/2));
    }

    leftPlane->setFrameFromBasis(normals[0],normals[1],cross(normals[0],normals[1]));
    leftPlane->setPosition(poly.getMeshPoint(leftPlane->getID()));

    unsigned long long lastIndex = normals.size()-2;
    rightPlane->setFrameFromBasis(normals[lastIndex],normals[lastIndex+1],cross(normals[lastIndex],normals[lastIndex+1]));
    rightPlane->setPosition(poly.getMeshPoint(rightPlane->getID()));

    update();
}

void ViewerFibula::bendPolyline(unsigned int id, Vec v){
    poly.bendFibula(id, v);

    std::vector<Vec> normals;
    for(unsigned int i=0; i<planeNormals.size(); i++) normals.push_back(planeNormals[i]);
    poly.getRelatvieNormals(normals);

    for(unsigned int i=4; i<normals.size()-4; i+=2){
        ghostPlanes[i/2-2]->setFrameFromBasis(normals[i], normals[i+1], cross(normals[i], normals[i+1]));
        ghostPlanes[i/2-2]->setPosition(poly.getMeshPoint(ghostPlanes[i/2-2]->getID()));//(ghostPlanes[i/2-2]->getID()+3)/2));
    }

    leftPlane->setFrameFromBasis(normals[0],normals[1],cross(normals[0],normals[1]));
    leftPlane->setPosition(poly.getMeshPoint(leftPlane->getID()));

    unsigned long long lastIndex = normals.size()-2;
    rightPlane->setFrameFromBasis(normals[lastIndex],normals[lastIndex+1],cross(normals[lastIndex],normals[lastIndex+1]));
    rightPlane->setPosition(poly.getMeshPoint(rightPlane->getID()));

    update();
}

void ViewerFibula::initGhostPlanes(Movable s){
    deleteGhostPlanes();
    double size = 20.;
    for(unsigned int i=0; i<(poly.getNbPoints()-4)*2; i++){     // -2 for total nb of planes, another -2 for nb of ghost planes
        Plane *p1 = new Plane(size, s, .5f, i/2+2);
        ghostPlanes.push_back(p1);
        ghostPlanes[i]->setPosition(poly.getMeshPoint((i+2)/2));
        ghostPlanes[i]->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
    }

    for(unsigned int i=0; i<ghostPlanes.size(); i++) connect(&(ghostPlanes[i]->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &Viewer::bendPolyline);        // connnect the ghost planes

    /*for(unsigned int i=0; i<tempFibPlanes.size(); i++) delete tempFibPlanes[i];
    tempFibPlanes.clear();

    for(unsigned int i=1; i<poly.getNbPoints()-1; i++){
        Vec pos(0,0,0);
        Plane *p1 = new Plane(1., Movable::DYNAMIC, pos, .5f, i);
        p1->setPosition(poly.getPoint(i));
        p1->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
        tempFibPlanes.push_back(p1);
    }*/
}

void ViewerFibula::initCurve(){
    control.push_back(Vec(108.241, 69.6891, -804.132));
    control.push_back(Vec(97.122, 82.1788, -866.868));
    control.push_back(Vec(93.5364, 90.1045, -956.126));
    control.push_back(Vec(83.3966, 92.5807, -1069.7));
    control.push_back(Vec(80.9, 90.1, -1155));
    control.push_back(Vec(86.4811, 90.9929, -1199.7));

    constructCurve();
}

void ViewerFibula::constructCurve(){
    nbU = 2000;
    curve.init(control.size(), control);
    curve.generateCatmull(nbU);
    //connect(curve, &Curve::curveReinitialised, this, &Viewer::updatePlanes);
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

void ViewerFibula::rotatePolylineOnAxis(int position){
    int pos = position - polyRotation;
    polyRotation = position;
    double alpha = static_cast<double>(pos) / 180. * M_PI;

    const Vec tangent = poly.getWorldTransform(Vec(1,0,0));

    Vec n = poly.getWorldTransform(poly.getNormal());
    Vec b = poly.getWorldTransform(-poly.getBinormal());
    poly.rotateOnAxis(alpha, poly.getMeshPoint(0)-(n+b)*leftPlane->getSize());

    leftPlane->rotate(Quaternion(leftPlane->getLocalVector(tangent), alpha));
    rightPlane->rotate(Quaternion(rightPlane->getLocalVector(tangent), alpha));
    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->rotate( Quaternion(ghostPlanes[i]->getLocalVector(tangent), alpha));

    repositionPlanesOnPolyline();

    update();
}

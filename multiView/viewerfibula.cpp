#include "viewerfibula.h"

ViewerFibula::ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffset) : Viewer (parent, camera, sliderMax)
{

}

void ViewerFibula::updateFibPolyline(const Vec& firstPoint, const std::vector<double>& distances){
    std::vector<Vec> newPoints;
    newPoints.push_back(firstPoint);
    for(unsigned int i=0; i<distances.size(); i++) newPoints.push_back(newPoints[i] + Vec(distances[i], 0, 0));
    updatePolyline(newPoints);
}

void ViewerFibula::bendPolylineNormals(const std::vector<Vec>& normals, const std::vector<double>& distances){
    updateFibPolyline(poly.getMeshPoint(0), distances);

    for(unsigned int i=4; i<normals.size()-4; i+=2){
        Vec v = poly.getWorldTransform(normals[i]);
        Vec a = poly.getWorldTransform(normals[i+1]);
        ghostPlanes[i/2-2]->setFrameFromBasis(v, a, cross(v, a));
        ghostPlanes[i/2-2]->setPosition(poly.getMeshPoint((ghostPlanes[i/2-2]->getID()+3)/2));

        // rotate 90 degrees around the normal to line up z with the tangent (which stays fixed)
        ghostPlanes[i/2-2]->rotate(Quaternion(ghostPlanes[i/2-2]->getLocalVector(Vec(0,1,0)), M_PI/2.));
    }

    Vec n = poly.getWorldTransform(normals[0]);
    Vec b = poly.getWorldTransform(normals[1]);

    leftPlane->setFrameFromBasis(n,b,cross(n,b));
    leftPlane->setPosition(poly.getMeshPoint(leftPlane->getID()));

    unsigned long long lastIndex = normals.size()-2;
    n = poly.getWorldTransform(normals[lastIndex]);
    b = poly.getWorldTransform(normals[lastIndex+1]);

    rightPlane->setFrameFromBasis(n,b,cross(n,b));
    rightPlane->setPosition(poly.getMeshPoint(rightPlane->getID()));

    leftPlane->rotate(Quaternion(leftPlane->getLocalVector(Vec(0,1,0)), M_PI/2.));
    rightPlane->rotate(Quaternion(rightPlane->getLocalVector(Vec(0,1,0)), M_PI/2.));

    /*for(unsigned int i=0; i<tempNorms.size(); i+=2){
        Vec v = poly.getWorldTransform(tempNorms[i]);
        Vec a = poly.getWorldTransform(tempNorms[i+1]);
        tempFibPlanes[i/2]->setFrameFromBasis(v, a, cross(v, a));
        tempFibPlanes[i/2]->setPosition(poly.getPoint(((i/2)+1)));

        tempFibPlanes[i/2]->rotate(Quaternion(tempFibPlanes[i/2]->getLocalVector(Vec(0,1,0)), M_PI/2.));
    }*/

    update();
}

void ViewerFibula::bendPolyline(unsigned int id, Vec v){

}

void ViewerFibula::initGhostPlanes(Movable s){
    deleteGhostPlanes();
    double size = 20.;
    for(unsigned int i=0; i<(poly.getNbPoints()-4)*2; i++){     // -2 for total nb of planes, another -2 for nb of ghost planes
        Plane *p1 = new Plane(size, s, .5f, i+1);
        //p1->toggleIsPoly();
        ghostPlanes.push_back(p1);
        ghostPlanes[i]->setPosition(poly.getPoint((i+2)/2));
        ghostPlanes[i]->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
    }

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
    double size = 20.;

    Vec n(0,0,1);
    Vec b(0,-1,0);
    n = poly.getWorldTransform(n);
    b = poly.getWorldTransform(b);

    for(unsigned int i=0; i<poly.getNbPoints(); i++) poly.lowerPoint(i, (n+b)*size);

    leftPlane->toggleIsPoly();
    rightPlane->toggleIsPoly();
    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->toggleIsPoly();

    repositionPlanesOnPolyline();
}

void ViewerFibula::repositionPlanesOnPolyline(){
    leftPlane->setPosition(poly.getMeshPoint(leftPlane->getID()));
    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->setPosition(poly.getMeshPoint(ghostPlanes[i]->getID()));
    rightPlane->setPosition(poly.getMeshPoint(rightPlane->getID()));
}

void ViewerFibula::constructPolyline(const std::vector<double>& distances, const std::vector<Vec>& newPoints){
    updateFibPolyline(curve.getPoint(curveIndexL), distances);

    initPolyPlanes(Movable::STATIC);

    toggleIsPolyline();

    Q_EMIT okToPlacePlanes(newPoints);
}

#include "viewerfibula.h"

ViewerFibula::ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffset) : Viewer (parent, camera, sliderMax)
{

}

void ViewerFibula::updateFibPolyline(const std::vector<Vec>& newPoints){
    updatePolyline(newPoints);
}

void ViewerFibula::bendPolylineNormals(const std::vector<Vec>& normals){
    poly.updateNormals(normals);

    // reinitialise the planes (create new ones each time)

    for(unsigned int i=0; i<normals.size(); i++){
        Vec binormal(0,1,0);
        Vec z = cross(normals[i], binormal);
        if(i%2==0) ghostPlanes[i]->setFrameFromBasis(normals[i], binormal, z);
        else ghostPlanes[i]->setFrameFromBasis(normals[i], -binormal, -z);
        ghostPlanes[i]->setPosition(poly.getPoint((i+2)/2));
    }

    update();
}

void ViewerFibula::bendPolyline(unsigned int id, Vec v){

}

void ViewerFibula::initGhostPlanes(){
    deleteGhostPlanes();
    for(unsigned int i=0; i<(poly.getNbPoints()-2)*2; i++){     // -2 for total nb of planes, another -2 for nb of ghost planes
        Vec pos(0,0,0);
        Plane *p1 = new Plane(1., Movable::STATIC, pos, .5f, i+1);
        ghostPlanes.push_back(p1);
        ghostPlanes[i]->setPosition(poly.getPoint((i+2)/2));
        if(i%2==0) ghostPlanes[i]->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
        else ghostPlanes[i]->setFrameFromBasis(Vec(0,0,1), Vec(0,1,0), Vec(-1,0,0));
    }
}

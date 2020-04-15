#include "viewerfibula.h"

ViewerFibula::ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffset) : Viewer (parent, camera, sliderMax)
{

}

void ViewerFibula::updateFibPolyline(const std::vector<Vec>& newPoints){
    updatePolyline(newPoints);
}

void ViewerFibula::bendPolylineNormals(const std::vector<Vec>& normals){
    poly.updateNormals(normals);

    // reinitialise the planes
    deleteGhostPlanes();
    for(unsigned int i=0; i<normals.size(); i++){
        Vec pos(0,0,0);
        Vec binormal(0,1,0);
        Vec z = cross(normals[i], binormal);
        Plane *p1 = new Plane(1., Movable::STATIC, pos, .5f);
        p1->setFrameFromBasis(normals[i], binormal, z);
        p1->setPosition(poly.getPoint((i+2)/2));
        ghostPlanes.push_back(p1);
    }
}

void ViewerFibula::bendPolyline(Vec& v){

}

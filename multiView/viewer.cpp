#include "viewer.h"
#include <QGLViewer/manipulatedFrame.h>

Viewer::Viewer(QWidget *parent, StandardCamera *cam, int sliderMax) : QGLViewer(parent) {
    Camera *c = camera();       // switch the cameras
    setCamera(cam);
    delete c;
}

void Viewer::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glMultMatrixd(manipulatedFrame()->matrix());

    poly.draw();

    glPopMatrix();
}

void Viewer::toUpdate(){
    update();
}

void Viewer::init() {
  setMouseTracking(true);
  restoreStateFromFile();
  setBackgroundColor(QColor("gray"));

  viewerFrame = new ManipulatedFrame();
  setManipulatedFrame(viewerFrame);
  setAxisIsDrawn(false);
  poly.init(viewerFrame);
  bendPolyline();

  // Camera without mesh
  Vec centre(0,0,0);
  float radius(5.);
  updateCamera(centre, radius);

  glEnable(GL_LIGHTING);
  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
  glLineWidth (1.0f);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
}

void swap(int& a, int& b){
    int temp = a;
    a = b;
    b = temp;
}

int Viewer::partition(int sorted[], int start, int end){
    /*int p = sorted[end];
    int index = start - 1;

    for(int i=start; i<end; i++){
        double tangentAngleA = angle(curve->tangent(sorted[i]-1), curve->tangent(sorted[i]));
        double tangentAngleP = angle(curve->tangent(p-1), curve->tangent(p));

        if(tangentAngleA >= tangentAngleP){
            index++;
            swap(sorted[index], sorted[i]);
        }
    }
    swap(sorted[index+1], sorted[end]);
    return index+1;*/
}

void Viewer::quicksort(int sorted[], int start, int end){
    /*if(start < end){
        int p = partition(sorted, start, end);
        quicksort(sorted, start, p-1);
        quicksort(sorted, p+1, end);
    }*/
}

void Viewer::updateCamera(const Vec& center, float radius){
    camera()->setSceneCenter(center);
    camera()->setSceneRadius(static_cast<double>(radius*1.05f));
    camera()->setZClippingCoefficient(static_cast<double>(radius/8.0f));
    camera()->showEntireScene();
}

double Viewer::segmentLength(const Vec a, const Vec b){
    return sqrt( pow((b.x - a.x), 2) + pow((b.y - a.y), 2) + pow((b.z - a.z), 2));
}

void Viewer::updatePolyline(const std::vector<Vec> &newPoints){
    poly.update(newPoints);
    //Q_EMIT updatePolyline(newPoints);
    update();
}

void Viewer::extendPolyline(int position){
    std::vector<Vec> newPoints;
    for(unsigned int i=0; i<poly.getNbPoints(); i++) newPoints.push_back(Vec(i*position, 0, 0));
    updatePolyline(newPoints);
}

void Viewer::bendPolyline(){
    Vec newPos(1,0,1);
    //Vec newPos(1,0,0);
    poly.bend(0, newPos);
}

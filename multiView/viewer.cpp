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
    glMultMatrixd(viewerFrame->matrix());

    poly.draw();

    for(unsigned int i=0; i<ghostPlanes.size(); i++){
        glColor4f(0., 1., 1., ghostPlanes[i]->getAlpha());
        ghostPlanes[i]->draw();
    }

    for(unsigned int i=0; i<tempPlanes.size(); i++){
        glColor4f(1., 0., 1., tempPlanes[i]->getAlpha());
        tempPlanes[i]->draw();
    }

    for(unsigned int i=0; i<tempFibPlanes.size(); i++){
        glColor4f(1., 0., 1., tempFibPlanes[i]->getAlpha());
        tempFibPlanes[i]->draw();
    }
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

  // Camera without mesh
  Vec centre(0,0,0);
  float radius(5.);
  updateCamera(centre, radius);

  glEnable(GL_LIGHTING);
  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
  glLineWidth (1.0f);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  initGhostPlanes();
}

void Viewer::tempBend(){
    Vec newPos(.3,0,.5);
    bendPolyline(1, newPos);
    update();
}

void Viewer::initGhostPlanes(){
    deleteGhostPlanes();
    for(unsigned int i=1; i<poly.getNbPoints()-1; i++){
        Vec pos(0,0,0);
        Plane *p = new Plane(1., Movable::DYNAMIC, pos, .5f, i);
        p->setPosition(poly.getPoint(i));
        p->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
        ghostPlanes.push_back(p);
    }

    for(unsigned int i=0; i<tempPlanes.size(); i++) delete tempPlanes[i];
    tempPlanes.clear();

    for(unsigned int i=1; i<poly.getNbPoints()-1; i++){
        Vec pos(0,0,0);
        Plane *p1 = new Plane(1., Movable::DYNAMIC, pos, .5f, i);
        p1->setPosition(poly.getPoint(i));
        p1->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
        tempPlanes.push_back(p1);

        Plane *p2 = new Plane(1., Movable::DYNAMIC, pos, .5f, i);
        p2->setPosition(poly.getPoint(i));
        p2->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
        tempPlanes.push_back(p2);
    }



    for(unsigned int i=0; i<ghostPlanes.size(); i++) connect(&(ghostPlanes[i]->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &Viewer::bendPolyline);        // connnect the ghost planes
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
    camera()->setSceneRadius(static_cast<double>(radius/2.f*1.005f));
    camera()->setZClippingCoefficient(static_cast<double>(radius));
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

void Viewer::bendPolyline(unsigned int pointIndex, Vec v){
    std::vector<double> distances;
    poly.getDistances(distances);

    std::vector<Vec> relativeNorms;
    std::vector<Vec> planeNormals;     // the x,y,z vectors of each frame
    std::vector<Vec> planeBinormals;
    std::vector<Vec> tempNorms;
    poly.bend(pointIndex, v, relativeNorms, planeNormals, planeBinormals);
    //Q_EMIT polylineBent(relativeNorms);

    // set the planes' orientations
    for(unsigned int i=0; i<ghostPlanes.size(); i++){
        //Vec binormal(0,1,0);
        ghostPlanes[i]->setPosition(poly.getPoint(i+1));
        ghostPlanes[i]->setFrameFromBasis(planeNormals[i], planeBinormals[i], cross(planeNormals[i],planeBinormals[i]));

        Vec n(1,0,0);
        Vec b(0,1,0);
        tempNorms.push_back(n);
        tempNorms.push_back(b);
        tempNorms.push_back(cross(n,b));
    }

    /*for(unsigned int i=0; i<ghostPlanes.size(); i++){       // convert the relative norms into coordinates relative to the ghost planes
        for(unsigned int j=0; j<6; j++){
            relativeNorms[i*6+j] = ghostPlanes[i]->getLocalVector(relativeNorms[i*6+j]);
        }
    }*/




    for(unsigned int i=0; i<tempPlanes.size(); i++){
        //Vec binormal(0,1,0);
        tempPlanes[i]->setPosition(poly.getPoint((i/2)+1));
        tempPlanes[i]->setFrameFromBasis(relativeNorms[i*3], relativeNorms[i*3+1], cross(relativeNorms[i*3], relativeNorms[i*3+1]));

        // convert the normal and binormal into these coordinates
        Vec n(1,0,0);
        Vec b(0,1,0);
        n = ghostPlanes[i/2]->getMeshVectorFromLocal(n);
        b = ghostPlanes[i/2]->getMeshVectorFromLocal(b);



        // convert in relation to tempPlanes
        relativeNorms[i*3] = tempPlanes[i]->getLocalVector(n);
        relativeNorms[i*3+1] = tempPlanes[i]->getLocalVector(b);


        // std::cout << "Dot product a : " << relativeNorms[i*6]*relativeNorms[i*6+1] << std::endl;
        //std::cout << "Dot product b : " << relativeNorms[i*6+3]*relativeNorms[i*6+4] << std::endl;

        //for(unsigned int j=0; j<6;j++) std::cout << i << "," << j << " : " << relativeNorms[i*6+j].x << " " << relativeNorms[i*6+j].y << " " << relativeNorms[i*6+j].z << std::endl;
    }


    Q_EMIT polylineBent(relativeNorms, distances, tempNorms);

}

void Viewer::deleteGhostPlanes(){
    for(unsigned int i=0; i<ghostPlanes.size(); i++) delete ghostPlanes[i];
    ghostPlanes.clear();
}

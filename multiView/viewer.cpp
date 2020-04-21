#include "viewer.h"
#include "meshreader.h"
#include <QGLViewer/manipulatedFrame.h>

Viewer::Viewer(QWidget *parent, StandardCamera *cam, int sliderMax) : QGLViewer(parent) {
    Camera *c = camera();       // switch the cameras
    setCamera(cam);
    delete c;
    isCurve = false;
    this->sliderMax = sliderMax;
}

void Viewer::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glMultMatrixd(viewerFrame->matrix());

    poly.draw();

    if(isCurve){
        glColor4f(0., 1., 0., leftPlane->getAlpha());
        leftPlane->draw();
        glColor4f(1., 0, 0., leftPlane->getAlpha());
        rightPlane->draw();

        for(unsigned int i=0; i<ghostPlanes.size(); i++){
            glColor4f(0., 1., 1., ghostPlanes[i]->getAlpha());
            ghostPlanes[i]->draw();
        }
    }

    curve.draw();

    /*for(unsigned int i=0; i<tempPlanes.size(); i++){
        glColor4f(1., 0., 1., tempPlanes[i]->getAlpha());
        tempPlanes[i]->draw();
    }*/

    /*for(unsigned int i=0; i<tempFibPlanes.size(); i++){
        glColor4f(1., 0., 1., tempFibPlanes[i]->getAlpha());
        tempFibPlanes[i]->draw();
    }*/
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
  poly.init(viewerFrame, 2);

  // Camera without mesh
  Vec centre(0,0,0);
  float radius(15.);
  updateCamera(centre, radius);

  glEnable(GL_LIGHTING);
  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
  glLineWidth (1.0f);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  //initCurve();
  //initPlanes();
}

void Viewer::tempBend(){
    //Vec newPos(.3,0,.5);
    std::vector<Vec> newPoints;
    newPoints.push_back(Vec(-10,-10,-10));
    newPoints.push_back(Vec(-2,0,0));
    newPoints.push_back(Vec(4,10,10));
    newPoints.push_back(Vec(6,10,10));
    newPoints.push_back(Vec(8,0,0));
    newPoints.push_back(Vec(10,-10,-10));
    //bendPolyline(1, newPos);
    constructPolyline(newPoints);
    update();
}

void Viewer::initCurvePlanes(Movable s){
    curveIndexR = nbU - 1;
    curveIndexL = 0;
    float size = 20.0;

    leftPlane = new Plane(static_cast<double>(size), s, 0.5, 0);
    rightPlane = new Plane(static_cast<double>(size), s, 0.5, 1);

    repositionPlane(leftPlane, curveIndexL);
    repositionPlane(rightPlane, curveIndexR);
}

void Viewer::repositionPlane(Plane *p, unsigned int index){
    p->setPosition(curve.getPoint(index));
    matchPlaneToFrenet(p, index);
}

void Viewer::matchPlaneToFrenet(Plane *p, unsigned int index){
    Vec x, y, z;
    curve.getFrame(index,z,x,y);
    p->setFrameFromBasis(x,y,z);
}

void Viewer::initPolyPlanes(Movable s){
    Vec p = poly.getPoint(1);
    leftPlane->setPosition(p);
    leftPlane->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));

    rightPlane->setPosition(poly.getPoint(static_cast<unsigned int>(poly.getNbPoints())-2));
    rightPlane->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));

    initGhostPlanes(s);
}

void Viewer::initGhostPlanes(Movable s){
    std::cout << "Initialsiing ghost planes" << std::endl;
    deleteGhostPlanes();
    for(unsigned int i=1; i<static_cast<unsigned int>(poly.getNbPoints()-1); i++){
        Plane *p = new Plane(1., s, .5f, i);
        p->setPosition(poly.getPoint(i));
        p->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
        ghostPlanes.push_back(p);
    }

    /*for(unsigned int i=0; i<tempPlanes.size(); i++) delete tempPlanes[i];
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
    }*/

    for(unsigned int i=0; i<ghostPlanes.size(); i++) connect(&(ghostPlanes[i]->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &Viewer::bendPolyline);        // connnect the ghost planes
}

void Viewer::updateCamera(const Vec& center, float radius){
    camera()->setSceneCenter(center);
    camera()->setSceneRadius(static_cast<double>(radius*1.05f));
    camera()->setZClippingCoefficient(static_cast<double>(radius));
    camera()->showEntireScene();
}

void Viewer::constructPolyline(const std::vector<Vec> &polyPoints){
    poly.reinit(polyPoints.size());
    std::vector<double> dists;
    poly.getDistances(dists);
    Q_EMIT constructPoly(dists, polyPoints);
}

void Viewer::placePlanes(const std::vector<Vec> &polyPoints){
    initPolyPlanes(Movable::DYNAMIC);
    for(unsigned int i=0; i<poly.getNbPoints(); i++) bendPolyline(i, polyPoints[i]);
}

double Viewer::segmentLength(const Vec a, const Vec b){
    return sqrt( pow((b.x - a.x), 2) + pow((b.y - a.y), 2) + pow((b.z - a.z), 2));
}

void Viewer::updatePolyline(const std::vector<Vec> &newPoints){
    poly.update(newPoints);
    //update();
}

void Viewer::bendPolyline(unsigned int pointIndex, Vec v){
    std::vector<double> distances;
    poly.getDistances(distances);

    std::vector<Vec> relativeNorms;
    std::vector<Vec> planeNormals;     // the x,y,z vectors of each frame
    std::vector<Vec> planeBinormals;

    poly.bend(pointIndex, v, relativeNorms, planeNormals, planeBinormals);

    // set the planes' orientations
    for(unsigned int i=0; i<ghostPlanes.size(); i++){
        ghostPlanes[i]->setPosition(poly.getPoint(i+1));
        ghostPlanes[i]->setFrameFromBasis(planeNormals[i], planeBinormals[i], cross(planeNormals[i],planeBinormals[i]));
    }

    leftPlane->setPosition(poly.getPoint(0));
    leftPlane->setFrameFromBasis(relativeNorms[0], relativeNorms[1], cross(relativeNorms[0], relativeNorms[1]));

    unsigned long long lastIndex = relativeNorms.size()-1;
    rightPlane->setPosition(poly.getPoint(static_cast<unsigned int>(poly.getNbPoints()-1)));
    rightPlane->setFrameFromBasis(relativeNorms[lastIndex-1], relativeNorms[lastIndex], cross(relativeNorms[lastIndex-1], relativeNorms[lastIndex]));


    for(unsigned int i=0; i<ghostPlanes.size()*2; i++){
        Plane tempPlane(1., Movable::STATIC, 0, 0);
        //tempPlanes[i]->setPosition(poly.getPoint((i/2)+1));
        tempPlane.setFrameFromBasis(relativeNorms[i*2], relativeNorms[i*2+1], cross(relativeNorms[i*2], relativeNorms[i*2+1]));

        // convert the normal and binormal into these coordinates
        Vec n(1,0,0);
        Vec b(0,1,0);
        n = ghostPlanes[i/2]->getMeshVectorFromLocal(n);
        b = ghostPlanes[i/2]->getMeshVectorFromLocal(b);

        // convert in relation to tempPlanes
        /*relativeNorms[i*2] = tempPlanes[i]->getLocalVector(n);
        relativeNorms[i*2+1] = tempPlanes[i]->getLocalVector(b);*/
        relativeNorms[i*2] = tempPlane.getLocalVector(n);
        relativeNorms[i*2+1] = tempPlane.getLocalVector(b);
    }

    Q_EMIT polylineBent(relativeNorms, distances);

}

void Viewer::initCurve(){
    control.push_back(Vec(-56.9335, -13.9973, 8.25454));

    control.push_back(Vec(-50.8191, -20.195, -19.53));
    control.push_back(Vec(-40.155, -34.5957, -50.7005));
    control.push_back(Vec(-27.6007, -69.2743, -67.6769));

    control.push_back(Vec(0, -85.966, -68.3154));

    control.push_back(Vec(26.7572, -69.0705, -65.6261));
    control.push_back(Vec(40.3576, -34.3609, -50.7634));
    control.push_back(Vec(46.2189, -21.3245, -17.9009));

    control.push_back(Vec(52.3669, -15.4613, 8.70223));

    constructCurve();
}

void Viewer::constructCurve(){
    curve.init(control.size(), control);
    nbU = 100;
    curve.generateCatmull(nbU);
    isCurve = true;
    initCurvePlanes(Movable::DYNAMIC);
}

void Viewer::deleteGhostPlanes(){
    for(unsigned int i=0; i<ghostPlanes.size(); i++) delete ghostPlanes[i];
    ghostPlanes.clear();
}

void Viewer::readJSON(const QJsonArray &controlArray){
    control.clear();

    for(int i=0; i<controlArray.size(); i++){
        QJsonArray singleControl = controlArray[i].toArray();
        control.push_back(Vec(singleControl[0].toDouble(), singleControl[1].toDouble(), singleControl[2].toDouble()));
    }

    constructCurve();
}

void Viewer::openOFF(QString filename) {
    std::vector<Vec3Df> &vertices = mesh.getVertices();
    std::vector<Triangle> &triangles = mesh.getTriangles();

    FileIO::openOFF(filename.toStdString(), vertices, triangles);

    mesh.init();

    // Set the camera
    Vec3Df center;
    double radius;
    MeshTools::computeAveragePosAndRadius(vertices, center, radius);
    updateCamera(Vec(center), static_cast<float>(radius));

    update();
}

void Viewer::cutMesh(){
    bool isNumberRecieved;
    int nbGhostPlanes;
    int nbPieces = QInputDialog::getInt(this, "Cut mesh", "Number of pieces", 0, 1, 10, 1, &isNumberRecieved, Qt::WindowFlags());
    if(isNumberRecieved) nbGhostPlanes = nbPieces-1;
    else return;

    // Construct the polyline : First and last planes are immovable and are at the ends of the meshes
    std::vector<Vec> polylinePoints;
    polylinePoints.push_back(curve.getPoint(0));        // the start of the curve
    polylinePoints.push_back(curve.getPoint(curveIndexL));  // the left plane
    for(unsigned int i=0; i<nbGhostPlanes; i++){        // temporary
        polylinePoints.push_back(curve.getPoint(curveIndexL+(i+1)*5));
    }
    polylinePoints.push_back(curve.getPoint(curveIndexR));      // the right plane
    polylinePoints.push_back(curve.getPoint(nbU-1));        // the end of the curve

    constructPolyline(polylinePoints);

    // Create the ghost planes
    //initGhostPlanes(nbGhostPlanes, Movable::DYNAMIC);

    update();
}

void Viewer::moveLeftPlane(int position){
    double percentage = static_cast<double>(position) / static_cast<double>(sliderMax);
    unsigned int index = static_cast<unsigned int>(percentage * static_cast<double>(nbU) );

    if(curve.indexForLength(curveIndexR, -constraint) > index){  // Only move if we're going backwards or we haven't met the other plane
        curveIndexL = index;
        if(curveIndexL >= nbU) curveIndexL = nbU-1;     // shouldn't ever happen
    }
    else if( curveIndexL == curve.indexForLength(curveIndexR, -constraint) ){
        return;       // already in the correct position
    }
    else curveIndexL = curve.indexForLength(curveIndexR, -constraint);     // get the new position

    movePlane(leftPlane, true, curveIndexL);
}

void Viewer::moveRightPlane(int position){
    double percentage = static_cast<double>(position) / static_cast<double>(sliderMax);
    unsigned int index = nbU - 1 - static_cast<unsigned int>(percentage * static_cast<double>(nbU) );

    if( index > curve.indexForLength(curveIndexL, constraint)){        // its within the correct boundaries
        curveIndexR = index;
        if(curveIndexR >= nbU) curveIndexR = nbU-1; // shouldn't ever happen
    }
    else if(curveIndexR == curve.indexForLength(curveIndexL, constraint)) return;
    else curveIndexR = curve.indexForLength(curveIndexL, constraint);

    movePlane(rightPlane, false, curveIndexR);
}

void Viewer::movePlane(Plane *p, bool isLeft, unsigned int curveIndex){
    repositionPlane(p, curveIndex);
    update();
}

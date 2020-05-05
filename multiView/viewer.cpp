#include "viewer.h"
#include "meshreader.h"
#include <QGLViewer/manipulatedFrame.h>

Viewer::Viewer(QWidget *parent, StandardCamera *cam, int sliderMax) : QGLViewer(parent) {
    Camera *c = camera();       // switch the cameras
    setCamera(cam);
    delete c;
    isCurve = false;
    this->sliderMax = sliderMax;
    this->isCut = false;
    this->isDrawMesh = true;
}

void Viewer::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glMultMatrixd(viewerFrame->matrix());

    if(isCut) poly.draw();

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

    if(isDrawMesh) mesh.draw();

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
  poly.init(viewerFrame, 1);

  // Camera without mesh
  Vec centre(0,0,0);
  float radius(15.);
  updateCamera(centre, radius);

  glEnable(GL_LIGHTING);
  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
  glLineWidth (1.0f);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
}

void Viewer::initCurvePlanes(Movable s){
    curveIndexR = nbU - 1;
    curveIndexL = 0;
    float size = 30.0;

    leftPlane = new Plane(static_cast<double>(size), s, 0.5, 0);
    rightPlane = new Plane(static_cast<double>(size), s, 0.5, 1);

    mesh.addPlane(leftPlane);
    mesh.addPlane(rightPlane);

    repositionPlane(leftPlane, curveIndexL);
    repositionPlane(rightPlane, curveIndexR);

    endLeft = new Plane(static_cast<double>(size), Movable::STATIC, 0.5, 0);
    endRight = new Plane(static_cast<double>(size), Movable::STATIC, 0.5, 2);
    repositionPlane(endLeft, 0);
    repositionPlane(endRight, nbU-1);
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
    leftPlane->setID(1);
    rightPlane->setID(poly.getNbPoints()-2);
    endRight->setID(poly.getNbPoints()-1);

    leftPlane->setPosition(poly.getMeshPoint(leftPlane->getID()));
    leftPlane->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));

    rightPlane->setPosition(poly.getMeshPoint(rightPlane->getID()));
    rightPlane->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));

    initGhostPlanes(s);
}

void Viewer::toggleIsPolyline(){
    Vec direction = Vec(1,1,0);

    if(!leftPlane->getIsPoly()){
        endLeft->toggleIsPoly();
        poly.lowerPoint(endLeft->getID(), endLeft->getMeshVectorFromLocal(-direction)*leftPlane->getSize());
        endRight->toggleIsPoly();
        poly.lowerPoint(endRight->getID(), endRight->getMeshVectorFromLocal(-direction)*leftPlane->getSize());

        leftPlane->toggleIsPoly();
        poly.lowerPoint(leftPlane->getID(), leftPlane->getMeshVectorFromLocal(-direction)*leftPlane->getSize());
        rightPlane->toggleIsPoly();
        poly.lowerPoint(rightPlane->getID(), rightPlane->getMeshVectorFromLocal(-direction)*leftPlane->getSize());
        for(unsigned int i=0; i<ghostPlanes.size(); i++) {
            ghostPlanes[i]->toggleIsPoly();
            poly.lowerPoint(i+2, ghostPlanes[i]->getMeshVectorFromLocal(-direction)*leftPlane->getSize());
        }
    }
    else{
        endLeft->toggleIsPoly();
        poly.lowerPoint(endLeft->getID(), endLeft->getMeshVectorFromLocal(direction)*leftPlane->getSize());
        endRight->toggleIsPoly();
        poly.lowerPoint(endRight->getID(), endRight->getMeshVectorFromLocal(direction)*leftPlane->getSize());

        leftPlane->toggleIsPoly();
        poly.lowerPoint(leftPlane->getID(), leftPlane->getMeshVectorFromLocal(direction)*leftPlane->getSize());
        rightPlane->toggleIsPoly();
        poly.lowerPoint(rightPlane->getID(), rightPlane->getMeshVectorFromLocal(direction)*leftPlane->getSize());
        for(unsigned int i=0; i<ghostPlanes.size(); i++) {
            ghostPlanes[i]->toggleIsPoly();
            poly.lowerPoint(i+2, ghostPlanes[i]->getMeshVectorFromLocal(direction)*leftPlane->getSize());
        }
    }

    repositionPlanesOnPolyline();
}

void Viewer::repositionPlanesOnPolyline(){
    leftPlane->setPosition(poly.getMeshPoint(leftPlane->getID()));
    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->setPosition(poly.getMeshPoint(ghostPlanes[i]->getID()));
    rightPlane->setPosition(poly.getMeshPoint(rightPlane->getID()));
    endLeft->setPosition(poly.getMeshPoint(endLeft->getID()));
    endRight->setPosition(poly.getMeshPoint(endRight->getID()));
}

void Viewer::initGhostPlanes(Movable s){
    double size = leftPlane->getSize();

    for(unsigned int i=2; i<static_cast<unsigned int>(poly.getNbPoints()-2); i++){
        Plane *p = new Plane(size, s, .5f, i);
        p->setPosition(poly.getPoint(i));
        p->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
        ghostPlanes.push_back(p);
    }

    connect(&(leftPlane->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &Viewer::bendPolyline);
    connect(&(rightPlane->getCurvePoint()), &CurvePoint::curvePointTranslated, this, &Viewer::bendPolyline);

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

void Viewer::deconstructPolyline(){
    poly.reinit(1);
    deleteGhostPlanes();
    toggleIsPolyline();
    repositionPlane(leftPlane, curveIndexL);
    repositionPlane(rightPlane, curveIndexR);
}

void Viewer::placePlanes(const std::vector<Vec> &polyPoints){
    initPolyPlanes(Movable::DYNAMIC);
    for(unsigned int i=0; i<poly.getNbPoints(); i++) bendPolyline(i, polyPoints[i]);   
    toggleIsPolyline();
    std::vector<double> distances;
    poly.getDistances(distances);
    Q_EMIT toUpdateDistances(distances);        // the distances are no longer the same because the polyline has been lowered

    poly.recalculateOrientations();
    poly.resetBoxes();
}

double Viewer::segmentLength(const Vec a, const Vec b){
    return sqrt( pow((b.x - a.x), 2) + pow((b.y - a.y), 2) + pow((b.z - a.z), 2));
}

void Viewer::updatePolyline(const std::vector<Vec> &newPoints){
    poly.updatePoints(newPoints);
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
        ghostPlanes[i]->setPosition(poly.getPoint(ghostPlanes[i]->getID()));
        ghostPlanes[i]->setFrameFromBasis(planeNormals[i+1], planeBinormals[i+1], cross(planeNormals[i+1],planeBinormals[i+1]));
    }

    leftPlane->setPosition(poly.getPoint(leftPlane->getID()));
    leftPlane->setFrameFromBasis(planeNormals[0], planeBinormals[0], cross(planeNormals[0], planeBinormals[0]));

    rightPlane->setPosition(poly.getPoint(static_cast<unsigned int>(poly.getNbPoints()-2)));
    rightPlane->setFrameFromBasis(planeNormals.back(), planeBinormals.back(), cross(planeNormals.back(), planeBinormals.back()));

    unsigned long long lastIndex = relativeNorms.size()-2;
    endLeft->setFrameFromBasis(relativeNorms[0], relativeNorms[1], cross(relativeNorms[0], relativeNorms[1]));
    endRight->setFrameFromBasis(relativeNorms[lastIndex], relativeNorms[lastIndex+1], cross(relativeNorms[lastIndex], relativeNorms[lastIndex+1]));

    // Get the rotation of the planes in terms of the relative normals
    std::vector<Vec> norms;
    poly.getRelativePlane(*leftPlane, norms);
    relativeNorms[2] = norms[2];
    relativeNorms[3] = norms[3];

    unsigned int index = 4;
    for(unsigned int i=0; i<ghostPlanes.size(); i++){
        std::vector<Vec> norms;
        poly.getRelativePlane(*ghostPlanes[i], norms);
        for(unsigned int j=0; j<norms.size(); j++){
            relativeNorms[index] = norms[j];
            index++;
        }
    }

    lastIndex = relativeNorms.size()-4;
    poly.getRelativePlane(*rightPlane, norms);
    relativeNorms[lastIndex] = norms[0];
    relativeNorms[lastIndex+1] = norms[1];

    /*Plane tempPlane(1., Movable::STATIC, 0, 0);
    tempPlane.setFrameFromBasis(relativeNorms[2], relativeNorms[3], cross(relativeNorms[2], relativeNorms[3]));

    Vec n(1,0,0);
    Vec b(0,1,0);
    n = leftPlane->getMeshVectorFromLocal(n);
    b = leftPlane->getMeshVectorFromLocal(b);

    relativeNorms[2] = tempPlane.getLocalVector(n);
    relativeNorms[3] = tempPlane.getLocalVector(b);

    for(unsigned int i=0; i<ghostPlanes.size()*2; i++){
        Plane tempPlane(1., Movable::STATIC, 0, 0);
        tempPlane.setFrameFromBasis(relativeNorms[(i+2)*2], relativeNorms[(i+2)*2+1], cross(relativeNorms[(i+2)*2], relativeNorms[(i+2)*2+1]));

        // convert the normal and binormal into these coordinates
        Vec n(1,0,0);
        Vec b(0,1,0);
        n = ghostPlanes[i/2]->getMeshVectorFromLocal(n);
        b = ghostPlanes[i/2]->getMeshVectorFromLocal(b);

        // convert in relation to tempPlanes
        relativeNorms[(i+2)*2] = tempPlane.getLocalVector(n);
        relativeNorms[(i+2)*2+1] = tempPlane.getLocalVector(b);
    }

    lastIndex = relativeNorms.size()-4;
    tempPlane.setFrameFromBasis(relativeNorms[lastIndex], relativeNorms[lastIndex+1], cross(relativeNorms[lastIndex], relativeNorms[lastIndex+1]));

    n = Vec(1,0,0);
    b = Vec(0,1,0);
    n = rightPlane->getMeshVectorFromLocal(n);
    b = rightPlane->getMeshVectorFromLocal(b);

    relativeNorms[lastIndex] = tempPlane.getLocalVector(n);
    relativeNorms[lastIndex+1] = tempPlane.getLocalVector(b);*/

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
    float radius;
    mesh.computeBB(center, radius);
    updateCamera(Vec(center),radius);

    update();
}

void Viewer::cutMesh(){
    bool isNumberRecieved;
    unsigned int nbGhostPlanes;
    int nbPieces = QInputDialog::getInt(this, "Cut mesh", "Number of pieces", 0, 1, 10, 1, &isNumberRecieved, Qt::WindowFlags());
    if(isNumberRecieved) nbGhostPlanes = static_cast<unsigned int>(nbPieces)-1;
    else return;

    isCut = true;

    // Construct the polyline : First and last planes are immovable and are at the ends of the meshes
    std::vector<Vec> polylinePoints;
    polylinePoints.push_back(curve.getPoint(0));        // the start of the curve
    polylinePoints.push_back(curve.getPoint(curveIndexL));  // the left plane

    std::vector<unsigned int> ghostLocations;
    findGhostLocations(nbGhostPlanes, ghostLocations);
    for(unsigned int i=0; i<ghostLocations.size(); i++) polylinePoints.push_back(curve.getPoint(ghostLocations[i]));

    polylinePoints.push_back(curve.getPoint(curveIndexR));      // the right plane
    polylinePoints.push_back(curve.getPoint(nbU-1));        // the end of the curve

    constructPolyline(polylinePoints);

    update();
}

void Viewer::uncutMesh(){
    isCut = false;
    deconstructPolyline();
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

    movePlane(leftPlane, curveIndexL);
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

    movePlane(rightPlane, curveIndexR);
}

void Viewer::movePlane(Plane *p, unsigned int curveIndex){
    repositionPlane(p, curveIndex);

    if(isCut){
        toggleIsPolyline();
        bendPolyline(p->getID(), curve.getPoint(curveIndex));
        toggleIsPolyline();
    }
    else{
        double distance = curve.discreteChordLength(curveIndexL, curveIndexR);
        Q_EMIT planeMoved(distance);
    }

    update();
}

void swap(unsigned int& a, unsigned int& b){
    unsigned int temp = a;
    a = b;
    b = temp;
}

unsigned int Viewer::partition(std::vector<unsigned int>& sorted, unsigned int start, unsigned int end){
    unsigned int p = sorted[end];
    unsigned int index = start - 1;

    for(unsigned int i=start; i<end; i++){
        double tangentAngleA = angle(curve.tangent(sorted[i]-1), curve.tangent(sorted[i]));
        double tangentAngleP = angle(curve.tangent(p-1), curve.tangent(p));

        if(tangentAngleA >= tangentAngleP){
            index++;
            swap(sorted[index], sorted[i]);
        }
    }
    swap(sorted[index+1], sorted[end]);
    return index+1;
}

void Viewer::quicksort(std::vector<unsigned int>& sorted, int start, int end){
    if(start < end){
        unsigned int p = partition(sorted, start, end);
        quicksort(sorted, start, static_cast<int>(p)-1);
        quicksort(sorted, static_cast<int>(p)+1, end);
    }
}

void Viewer::findGhostLocations(unsigned int nbGhostPlanes, std::vector<unsigned int>& ghostLocation){
    unsigned int finalNb = nbGhostPlanes;        // the number we can actually fit in
    std::vector<unsigned int> maxIndicies(nbGhostPlanes);

    const unsigned int startI = curve.indexForLength(curveIndexL, constraint);
    const unsigned int endI = curve.indexForLength(curveIndexR, -constraint);

    if(endI > startI){         // if there's enough space for a plane
        const unsigned int searchArea = endI - startI;       // the space that's left between the left and right planes after the constraint is taken into account
        std::vector<unsigned int> sorted(static_cast<unsigned long long>(searchArea));
        for(unsigned int i=0; i<searchArea; i++) sorted[i] = startI+i;       // the possible indexes for the plane

        quicksort(sorted, 0, searchArea-1);      // Sort the indicies according to their tangent angles

        maxIndicies[0] = sorted[0];
        unsigned int sortedIndex = 1;

        for(unsigned int i=1; i<nbGhostPlanes; i++){
            // the constraint (don't take it if it's too close to another existing plane)
            bool tooClose;
            do{
                tooClose = false;
                for(int j=static_cast<int>(i)-1; j>=0; j--){
                    if(sortedIndex < static_cast<unsigned int>(searchArea) && curve.discreteLength(static_cast<unsigned int>(maxIndicies[static_cast<unsigned int>(j)]),static_cast<unsigned int>(sorted[static_cast<unsigned int>(sortedIndex)]))<constraint){
                        tooClose = true;
                        break;
                    }
                }
                if(tooClose) sortedIndex++;
            }while(tooClose);

            if(sortedIndex >= static_cast<unsigned int>(searchArea)){      // if we leave the search area, stop
                finalNb = i;
                break;
            }
            maxIndicies[i] = sorted[sortedIndex];
            sortedIndex++;  // move with i
        }

        // sort the planes
        for(unsigned int i=0; i<finalNb; i++){
            for(unsigned int j=i+1; j<finalNb; j++){
                if(maxIndicies[i] > maxIndicies[j]) swap(maxIndicies[i], maxIndicies[j]);
            }
        }

        ghostLocation.clear();
        for(unsigned int i=0; i<finalNb; i++) ghostLocation.push_back(maxIndicies[i]);       // get the location for each ghost plane
    }
}

double Viewer::angle(Vec a, Vec b){
    double na = a.norm();
    double nb = b.norm();
    double ab = a*b;

    double val = ab / (na*nb);
    if(val >= static_cast<double>(1)) val = 1;          // protection from floating point errors (comparing it to an epsilon didn't work)
    else if(val < static_cast<double>(-1)) val = -1;
    return acos(val);
}

void Viewer::rotatePolylineOnAxis(int position){
    double r = position/360.;
    for(unsigned int i=0; i<poly.getNbPoints()-1; i++) poly.rotateBox(i, r*2.*M_PI);
    update();
}

void Viewer::toggleDrawMesh(){
    isDrawMesh = !isDrawMesh;
    update();
}

void Viewer::toggleDrawPlane(){
    leftPlane->toggleIsVisible();
    rightPlane->toggleIsVisible();
    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->toggleIsVisible();
    update();
}

void Viewer::setMeshAlpha(int position){
    float p = static_cast<float>(position)/100.f;
    mesh.setAlpha(p);
    update();
}

void Viewer::setBoxAlpha(int position){
    float p = static_cast<float>(position)/100.f;
    poly.setAlpha(p);
    update();
}

void Viewer::setPlaneAlpha(int position){
    float p = static_cast<float>(position)/100.f;

    for(unsigned int i=0; i<ghostPlanes.size(); i++) ghostPlanes[i]->setAlpha(p);
    leftPlane->setAlpha(p);
    rightPlane->setAlpha(p);
    update();
}

void Viewer::cut(){
    /*mesh.setIsCut(Side::INTERIOR, true, true);

    update();*/
}

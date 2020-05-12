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

    if(isDrawMesh) mesh.draw();

    curve.draw();

    glPointSize(5.);
    glColor3b(1., 0, 0);
     glBegin(GL_POINTS);
    for(unsigned int i=0; i<outTemp.size(); i++){
            glVertex3d(outTemp[i].x, outTemp[i].y, outTemp[i].z);
    }
    for(unsigned int i=0; i<segmentPoints.size(); i++){
            glVertex3d(segmentPoints[i].x, segmentPoints[i].y, segmentPoints[i].z);
    }
     glEnd();

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

    if(isCut) poly.draw();


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

// Initialise the position and rotations of each plane on the polyline and set their IDs
void Viewer::initPolyPlanes(Movable s){
    leftPlane->setID(1);
    rightPlane->setID(poly.getNbPoints()-2);
    endRight->setID(poly.getNbPoints()-1);

    leftPlane->setPosition(poly.getMeshPoint(leftPlane->getID()));
    leftPlane->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));      // TODO this could be done with an existing function?

    rightPlane->setPosition(poly.getMeshPoint(rightPlane->getID()));
    rightPlane->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));

    initGhostPlanes(s);
}

void Viewer::toggleIsPolyline(){
    Vec direction = Vec(1,1,0);

    if(!leftPlane->getIsPoly()){
        direction = -Vec(1,1,0);
    }

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

    if(leftPlane->getIsPoly()){     // if the polyline now exits, project it onto the mesh
        std::vector<Vec> outputPoints;
        std::vector<Vec> inputPoints;
        for(unsigned int i=0; i<poly.getNbPoints(); i++) inputPoints.push_back(poly.getMeshPoint(i));
        mesh.mlsProjection(inputPoints, outputPoints);
        updatePolyline(outputPoints);
        outTemp = outputPoints;
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
    double size = leftPlane->getSize();     // match everythign to the size of the left plane

    for(unsigned int i=2; i<static_cast<unsigned int>(poly.getNbPoints()-2); i++){
        Plane *p = new Plane(size, s, .5f, i);
        p->setPosition(poly.getPoint(i));
        p->setFrameFromBasis(Vec(0,0,1), Vec(0,-1,0), Vec(1,0,0));
        ghostPlanes.push_back(p);
    }

    // Connect all planes' movement (except the two end planes which we don't see). If a plane is moved, bend the polyline.
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
    poly.reinit(polyPoints.size());     // re-initialise the polyline
    std::vector<double> dists;
    poly.getDistances(dists);       // the distances should all be 1 here
    Q_EMIT constructPoly(dists, polyPoints);        // The fibula polyline has to be initialised before we can start bending the polylines
}

void Viewer::deconstructPolyline(){
    poly.reinit(1);
    deleteGhostPlanes();
    toggleIsPolyline();
    repositionPlane(leftPlane, curveIndexL);
    repositionPlane(rightPlane, curveIndexR);
}

/*
 * Physically place planes on the polyline
 * Bend the polyline so it corresponds to the correct point
*/
void Viewer::placePlanes(const std::vector<Vec> &polyPoints){
    initPolyPlanes(Movable::DYNAMIC);       // Create the planes
    std::vector<Vec> norms, binorms;
    for(unsigned int i=0; i<poly.getNbPoints()-1; i++) simpleBend(i, polyPoints[i], norms, binorms);        // Bend the polyline point by point WITHOUT updating rest
    bendPolyline(poly.getNbPoints()-1, polyPoints[poly.getNbPoints()-1]);       // update the fibula and set the planes for all points on the last point
    toggleIsPolyline();     // Change the polyline from going through the centre of the planes to going through the corner
    std::vector<double> distances;
    poly.getDistances(distances);
    Q_EMIT toUpdateDistances(distances);        // the distances are no longer the same because the polyline has been lowered, so update it in the fibula

    poly.resetBoxes();      // Set the boxes to the polyline
}

double Viewer::segmentLength(const Vec a, const Vec b){
    return sqrt( pow((b.x - a.x), 2) + pow((b.y - a.y), 2) + pow((b.z - a.z), 2));
}

void Viewer::updatePolyline(const std::vector<Vec> &newPoints){
    poly.updatePoints(newPoints);
    poly.resetBoxes();
}


/*
 * Modify the polyline point pointIndex to v
 * Activated when a plane is moved by hand in the mandible
*/
void Viewer::bendPolyline(unsigned int pointIndex, Vec v){
    std::vector<Vec> planeNormals;
    std::vector<Vec> planeBinormals;

    simpleBend(pointIndex, v, planeNormals, planeBinormals);

    // Update the planes
    repositionPlanesOnPolyline();
    setPlaneOrientations(planeNormals, planeBinormals);

    // Get the mandible polyine planes in relation to the boxes so this info can be sent to the fibula
    std::vector<Vec> relativeNorms;
    getPlaneBoxOrientations(relativeNorms);

    // Get the new distances between each point in the mandible
    std::vector<double> distances;
    poly.getDistances(distances);

    Q_EMIT polylineBent(relativeNorms, distances);      // Send the new normals and distances to the fibula TODO only send over the info for the corresponding point
}

void Viewer::simpleBend(const unsigned int &pointIndex, Vec v,std::vector<Vec> &planeNormals, std::vector<Vec> &planeBinormals){
    poly.bend(pointIndex, v, planeNormals, planeBinormals);     // Bend the polyline and get the planeNormals and planeBinormals which will be used to set the planes' orientations
}

void Viewer::setPlaneOrientation(Plane &p, std::vector<Vec> &norms, std::vector<Vec> &binorms){
    p.setFrameFromBasis(norms[p.getID()], binorms[p.getID()], cross(norms[p.getID()], binorms[p.getID()]));
}

void Viewer::setPlaneOrientations(std::vector<Vec> &norms, std::vector<Vec> &binorms){
    setPlaneOrientation(*leftPlane, norms, binorms);
    setPlaneOrientation(*rightPlane, norms, binorms);
    setPlaneOrientation(*endLeft, norms, binorms);
    setPlaneOrientation(*endRight, norms, binorms);
    for(unsigned int i=0; i<ghostPlanes.size(); i++) setPlaneOrientation(*ghostPlanes[i], norms, binorms);
}

// Get the rotation of the planes in terms of the boxes
void Viewer::getPlaneBoxOrientations(std::vector<Vec> &relativeNorms){
    relativeNorms.clear();
    std::vector<Vec> norms;
    poly.getRelativePlane(*leftPlane, norms);
    relativeNorms.push_back(norms[2]);
    relativeNorms.push_back(norms[3]);

    for(unsigned int i=0; i<ghostPlanes.size(); i++){
        std::vector<Vec> norms;
        poly.getRelativePlane(*ghostPlanes[i], norms);
        for(unsigned int j=0; j<norms.size(); j++){
            relativeNorms.push_back(norms[j]);
        }
    }

    poly.getRelativePlane(*rightPlane, norms);
    relativeNorms.push_back(norms[0]);
    relativeNorms.push_back(norms[1]);
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
    findGhostLocations(nbGhostPlanes, ghostLocations);      // find the curve indexes on which the ghost planes must be placed
    for(unsigned int i=0; i<ghostLocations.size(); i++) polylinePoints.push_back(curve.getPoint(ghostLocations[i]));        // get the world location of these indexes

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

// Find where the ghost planes should be placed
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
    double r = (position/360.*2.*M_PI);
    //for(unsigned int i=0; i<poly.getNbPoints()-1; i++) poly.rotateBox(i, r);

    // send over the new norms
    std::vector<Vec> norms;
    getPlaneBoxOrientations(norms);
    Q_EMIT toUpdatePlaneOrientations(norms);
    Q_EMIT toRotatePolylineOnAxis(r);
    update();
}

void Viewer::toggleDrawMesh(){
    isDrawMesh = !isDrawMesh;
    update();
}

void Viewer::toggleWireframe(){
    poly.toggleIsWireframe();
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

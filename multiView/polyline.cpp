#include "polyline.h"
#include "plane.h"

Polyline::Polyline()
{
    frame = ManipulatedFrame();
}

void Polyline::init(const Frame *const refFrame, unsigned int nbPoints){
    frame.setReferenceFrame(refFrame);
    reinit(nbPoints);
}

void Polyline::reinit(unsigned int nbPoints){
    points.clear();
    boxes.clear();
    segmentNormals.clear();
    segmentBinormals.clear();
    cuttingLines.clear();
    cuttingBinormals.clear();

    for(unsigned int i=0; i<nbPoints; i++) points.push_back(Vec(i, 0, 0));
    for(unsigned int i=0; i<points.size()-1; i++) segmentNormals.push_back(normal);
    for(unsigned int i=0; i<points.size()-1; i++) segmentBinormals.push_back(binormal);
    for(unsigned int i=1; i<points.size()-1; i++) cuttingLines.push_back(normal);
    for(unsigned int i=1; i<points.size()-1; i++) originalCuttingLines.push_back(normal);
    for(unsigned int i=1; i<points.size()-1; i++) cuttingBinormals.push_back(binormal);
    for(unsigned int i=0; i<points.size()-1; i++){
        boxes.push_back(Box());
        boxes[i].init(frame.referenceFrame());
    }
}

void Polyline::draw(){
    glPushMatrix();
    glMultMatrixd(frame.matrix());

    QGLViewer::drawAxis(20.);

    // The polyline
    glLineWidth(5.);
    glColor3f(0,0,1);
    glBegin(GL_LINES);
    for(unsigned int i=0; i<points.size()-1; i++){
        glVertex3d(points[i].x, points[i].y, points[i].z);
        glVertex3d(points[i+1].x, points[i+1].y, points[i+1].z);
    }
    glEnd();

    // The points
    glColor3f(0,1,0);
    glPointSize(10.);
    glBegin(GL_POINTS);
    for(unsigned int i=0; i<points.size(); i++) glVertex3d(points[i].x, points[i].y, points[i].z);
    glEnd();

    for(unsigned int i=0; i<boxes.size(); i++) boxes[i].draw();

    glPopMatrix();
}

void Polyline::drawBox(unsigned int index){
    double size = 25.;
    const Vec& p0 = points[index];
    const Vec& p1 = points[index+1];
    Vec p2 = points[index] + segmentBinormals[index]*size;
    Vec p3 = points[index+1] + segmentBinormals[index]*size;
    Vec p4 = points[index] + segmentNormals[index]*size;
    Vec p5 = points[index+1] + segmentNormals[index]*size;
    Vec p6 = points[index] + (segmentNormals[index] +  segmentBinormals[index])*size;
    Vec p7 = points[index+1] + (segmentNormals[index] +  segmentBinormals[index])*size;

    glBegin(GL_LINES);
        /*glVertex3d(p0.x, p0.y, p0.z);
        glVertex3d(p1.x, p1.y, p1.z);*/

        glVertex3d(p0.x, p0.y, p0.z);
        glVertex3d(p4.x, p4.y, p4.z);

        glVertex3d(p0.x, p0.y, p0.z);
        glVertex3d(p2.x, p2.y, p2.z);

        glVertex3d(p5.x, p5.y, p5.z);
        glVertex3d(p1.x, p1.y, p1.z);

        glVertex3d(p3.x, p3.y, p3.z);
        glVertex3d(p1.x, p1.y, p1.z);

        glVertex3d(p7.x, p7.y, p7.z);
        glVertex3d(p5.x, p5.y, p5.z);

        glVertex3d(p7.x, p7.y, p7.z);
        glVertex3d(p3.x, p3.y, p3.z);

        glVertex3d(p7.x, p7.y, p7.z);
        glVertex3d(p6.x, p6.y, p6.z);

        glVertex3d(p4.x, p4.y, p4.z);
        glVertex3d(p6.x, p6.y, p6.z);

        glVertex3d(p2.x, p2.y, p2.z);
        glVertex3d(p6.x, p6.y, p6.z);

        glVertex3d(p4.x, p4.y, p4.z);
        glVertex3d(p5.x, p5.y, p5.z);

        glVertex3d(p2.x, p2.y, p2.z);
        glVertex3d(p3.x, p3.y, p3.z);

    glEnd();
}

void Polyline::updatePoints(const std::vector<Vec> &newPoints){
    points.clear();
    for(unsigned int i=0; i<newPoints.size(); i++) points.push_back(newPoints[i]);
    for(unsigned int i=0; i<boxes.size(); i++) resetBox(i);
}

double Polyline::angle(const Vec &a, const Vec &b){
    double na = a.norm();
    double nb = b.norm();
    double ab = a*b;

    double val = ab / (na*nb);
    if(val >= static_cast<double>(1)) val = 1;          // protection from floating point errors (comparing it to an epsilon didn't work)
    else if(val < static_cast<double>(-1)) val = -1;
    return acos(val);
}

Vec Polyline::projection(Vec &a, Vec &planeNormal){
    double alpha = (a * planeNormal);
    return a - planeNormal * alpha;
}

void Polyline::resetBox(unsigned int index){
    boxes[index].setPosition(points[index]);
    const Vec& n = segmentNormals[index];
    const Vec& b = segmentBinormals[index];
    boxes[index].setFrameFromBasis(-cross(n,b),b,n);
    double length = euclideanDistance(points[index], points[index+1]);
    boxes[index].setLength(length);
}

void Polyline::bend(unsigned int index, Vec &newPosition, std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals){
    if(index >= points.size()) return;

    points[index] = getLocalCoordinates(newPosition);

    if(index!=0){
        recalculateBinormal(index-1, points[index-1], points[index]);
        resetBox(index-1);
    }
    if(index!=points.size()-1){
        recalculateBinormal(index, points[index], points[index+1]);
        resetBox(index);
     }

    getCuttingAngles(relativeNorms, planeNormals, planeBinormals);
}

void Polyline::bendFibula(unsigned int index, Vec &newPosition){
    if(index >= points.size()) return;

    points[index] = getLocalCoordinates(newPosition);

    if(index!=0){
        recalculateBinormal(index-1, points[index-1], points[index]);
        resetBox(index-1);
    }
    if(index!=points.size()-1){
        recalculateBinormal(index, points[index], points[index+1]);
        resetBox(index);
    }

}

void Polyline::recalculateNormal(unsigned int index, const Vec &origin, const Vec &newPosition){
    Vec pos = newPosition - origin;
    pos.normalize();

    segmentNormals[index] = -cross(pos, segmentBinormals[index]);
    segmentNormals[index].normalize();
}

void Polyline::recalculateBinormal(unsigned int index, const Vec &origin, const Vec &newPosition){
   // Calculate an orthogonal vector on the plane
    Vec pos = newPosition - origin;
    pos.normalize();
    pos.z = 0;        // The new polyline projected in the z plane

    double theta = angle(pos, tangent);        // get the angle which the tangent rotated
    if(pos.y <0) theta = -theta;        // rotate the opposite way

    double x = binormal.x * cos(theta) - binormal.y * sin(theta);
    double y = binormal.x * sin(theta) + binormal.y * cos(theta);
    segmentBinormals[index] = Vec(x,y,0);
    segmentBinormals[index].normalize();

    recalculateNormal(index, newPosition, origin);
 }

Vec Polyline::vectorQuaternionRotation(double theta, const Vec &axis, const Vec &vectorToRotate){
    Quaternion r(cos(theta/2.0)*axis.x, cos(theta/2.0)*axis.y, cos(theta/2.0)*axis.z, sin(theta/2.0));      // rotation

    Frame f = Frame();
    initialiseFrame(f);
    f.rotate(r);

    return f.localInverseTransformOf(vectorToRotate);
}

void Polyline::initialiseFrame(Frame &f){
    Quaternion q = Quaternion();            // the base
    q.setFromRotatedBasis(getWorldTransform(Vec(1,0,0)), getWorldTransform(Vec(0,1,0)),getWorldTransform(Vec(0,0,1)));
    f.setOrientation(q);
}

void Polyline::getCuttingAngles(std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals){
    cuttingLines.clear();
    cuttingBinormals.clear();
    originalCuttingLines.clear();
    relativeNorms.clear();
    planeNormals.clear();
    planeBinormals.clear();

    for(unsigned int i=0; i<segmentNormals.size()-1; i++){
        Vec v = segmentNormals[i] + segmentNormals[i+1];
        v /= 2.0;
        v.normalize();
        cuttingLines.push_back(v);
        originalCuttingLines.push_back(v);

        Vec b = segmentBinormals[i] + segmentBinormals[i+1];
        b /= 2.0;
        b.normalize();
        cuttingBinormals.push_back(b);

        relativeNorms.push_back(segmentNormals[i]);
        relativeNorms.push_back(segmentBinormals[i]);
        relativeNorms.push_back(segmentNormals[i+1]);
        relativeNorms.push_back(segmentBinormals[i+1]);
    }

    for(unsigned int i=0; i<cuttingLines.size(); i++){
        double theta = angle(cuttingLines[i], cuttingBinormals[i]);
        double alpha = M_PI / 2.0 - theta + M_PI;
        Vec axis = cross(cuttingLines[i], cuttingBinormals[i]);
        cuttingLines[i] = vectorQuaternionRotation(alpha, axis, cuttingLines[i]);

        planeNormals.push_back(cuttingLines[i]);            // save for the mandible
        planeBinormals.push_back(cuttingBinormals[i]);
    }
}

void Polyline::getDistances(std::vector<double> &distances){
    distances.clear();

    for(unsigned int i=0; i<points.size()-1; i++){
        distances.push_back(euclideanDistance(points[i], points[i+1]));
    }
}

double Polyline::euclideanDistance(const Vec &a, const Vec &b){
    return sqrt(pow(a.x-b.x, 2.) + pow(a.y-b.y, 2.) + pow(a.z-b.z, 2.));
}

void Polyline::lowerPoint(unsigned int index, const Vec &toLower){
    points[index] += toLower;
}

void Polyline::resetBoxes(){
    for(unsigned int i=0; i<boxes.size(); i++) resetBox(i);
}

void Polyline::getRelatvieNormals(std::vector<Vec> &relativeNorms){
    Plane temp(0.5, Movable::STATIC, 0, 0);
    temp.setFrameFromBasis(segmentNormals[0], segmentBinormals[0], cross(segmentNormals[0], segmentBinormals[0]));
    relativeNorms[0] = getWorldTransform(temp.getMeshVectorFromLocal(relativeNorms[0]));
    relativeNorms[1] = getWorldTransform(temp.getMeshVectorFromLocal(relativeNorms[1]));

    for(unsigned int i=1; i<segmentNormals.size()-1; i++){
        temp.setFrameFromBasis(segmentNormals[i], segmentBinormals[i], cross(segmentNormals[i], segmentBinormals[i]));
        for(int j=-2; j<2; j++) relativeNorms[i*4+j] = getWorldTransform(temp.getMeshVectorFromLocal(relativeNorms[i*4+j]));
    }
}

void Polyline::rotateBox(unsigned int i, double angle){
    boxes[i].rotateOnAxis(angle);
}

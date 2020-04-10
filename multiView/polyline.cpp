#include "polyline.h"

Polyline::Polyline()
{
    frame = ManipulatedFrame();

    for(unsigned int i=0; i<4; i++) points.push_back(Vec(i, 0, 0));
    for(unsigned int i=0; i<points.size()-1; i++) segmentNormals.push_back(normal);
}

void Polyline::init(const Frame *const refFrame){
    frame.setReferenceFrame(refFrame);
}

void Polyline::draw(){
    glPushMatrix();
    glMultMatrixd(frame.matrix());

    // The polyline
    glLineWidth(5.);
    glColor3f(0.,0.,1.);
    glBegin(GL_LINES);
    for(unsigned int i=0; i<points.size()-1; i++){
        glVertex3f(points[i].x, points[i].y, points[i].z);
        glVertex3f(points[i+1].x, points[i+1].y, points[i+1].z);
    }
    glEnd();

    // The normals
    glColor3f(1., 0., 0.);
    glBegin(GL_LINES);
    Vec endPoint = points[0]+0.5*segmentNormals[0];
    glVertex3f(points[0].x, points[0].y, points[0].z);
    glVertex3f(endPoint.x, endPoint.y, endPoint.z);
    for(unsigned int i=1; i<points.size()-1; i++){
        for(unsigned int j=0; j<2; j++){
            endPoint = points[i]+0.5*segmentNormals[i-j];
            glVertex3f(points[i].x, points[i].y, points[i].z);
            glVertex3f(endPoint.x, endPoint.y, endPoint.z);
        }
    }
    endPoint = points.back()+0.5*segmentNormals.back();
    glVertex3f(points.back().x, points.back().y, points.back().z);
    glVertex3f(endPoint.x, endPoint.y, endPoint.z);
    glEnd();

    // The points
    glColor3f(0.,1.,0.);
    glPointSize(10.);
    glBegin(GL_POINTS);
    for(unsigned int i=0; i<points.size(); i++) glVertex3f(points[i].x, points[i].y, points[i].z);
    glEnd();

    // The cutting lines
    glPopMatrix();
    glColor3f(0.,1.,1.);
    glBegin(GL_LINES);
    for(unsigned int i=0; i<cuttingLines.size(); i++){
        endPoint = points[i+1]+0.5*cuttingLines[i];
        glVertex3f(points[i+1].x, points[i+1].y, points[i+1].z);
        glVertex3f(endPoint.x, endPoint.y, endPoint.z);
    }
    glEnd();
}

void Polyline::update(const std::vector<Vec> &newPoints){
    points.clear();
    for(unsigned int i=0; i<newPoints.size(); i++) points.push_back(newPoints[i]);
}

/*void Polyline::bendOnNormal(unsigned int index, double newZ){
    if(points.size()>index) return;

    points[index] = Vec(0,0, newZ);
}*/

double Polyline::getBendAngle(Vec &a, Vec &b){
    Vec yPlane(0,1,0);

    Vec projA = projection(a, yPlane);
    Vec projB = projection(b, yPlane);
    return angle(projA, projB);
}

double Polyline::angle(Vec &a, Vec &b){
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

void Polyline::bend(unsigned int index, Vec &newPosition){
    if(index >= points.size()-1) return;


    //Vec pointToBend = points[index] - origin;

    /*double bendAngle = getBendAngle(pointToBend, pos);

    rotateSegment(index, bendAngle, binormal);*/
    const Vec &origin = points[index+1];
    points[index] = newPosition;
    if(index!=0) recalculateNormal(index-1, newPosition, points[index-1]);
    recalculateNormal(index, origin, newPosition);

    getCuttingAngles();
}

void Polyline::recalculateNormal(unsigned int index, const Vec &origin, const Vec &newPosition){
    Vec pos = newPosition - origin;
    segmentNormals[index] = -cross(pos, binormal);
    segmentNormals[index].normalize();
}

void Polyline::rotateSegment(unsigned int index, double theta, const Vec &axis){
    Quaternion r(cos(theta/2.0)*axis.x, cos(theta/2.0)*axis.y, cos(theta/2.0)*axis.z, sin(theta/2.0));      // rotation

    Quaternion q = Quaternion();            // the base
    q.setFromRotatedBasis(Vec(1,0,0),Vec(0,1,0),Vec(0,0,1));
    Frame f = Frame();
    f.setOrientation(q);
    f.rotate(r);

    segmentNormals[index] = f.localInverseTransformOf(normal);
}

void Polyline::getCuttingAngles(){
    cuttingLines.clear();

    for(unsigned int i=0; i<segmentNormals.size()-1; i++){
        Vec v = segmentNormals[i] + segmentNormals[i+1];
        v /= 2.0;
        v.normalize();
        cuttingLines.push_back(v);
    }
}

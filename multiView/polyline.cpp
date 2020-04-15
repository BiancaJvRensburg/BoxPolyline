#include "polyline.h"

Polyline::Polyline()
{
    frame = ManipulatedFrame();

    for(unsigned int i=0; i<4; i++) points.push_back(Vec(i, 0, 0));
    for(unsigned int i=0; i<points.size()-1; i++) segmentNormals.push_back(normal);
    for(unsigned int i=1; i<points.size()-1; i++) cuttingLines.push_back(normal);
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
        glVertex3d(points[i].x, points[i].y, points[i].z);
        glVertex3d(points[i+1].x, points[i+1].y, points[i+1].z);
    }
    glEnd();

    // The normals
    /*glColor3f(1., 0., 0.);
    glBegin(GL_LINES);
    Vec endPoint = points[0]+0.5*displayNormals[0];
    glVertex3d(points[0].x, points[0].y, points[0].z);
    glVertex3d(endPoint.x, endPoint.y, endPoint.z);
    for(unsigned int i=1; i<points.size()-1; i++){
        endPoint = points[i]+0.5*displayNormals[2*i-1];
        glVertex3d(points[i].x, points[i].y, points[i].z);
        glVertex3d(endPoint.x, endPoint.y, endPoint.z);
        endPoint = points[i]+0.5*displayNormals[2*i];
        glVertex3d(points[i].x, points[i].y, points[i].z);
        glVertex3d(endPoint.x, endPoint.y, endPoint.z);
    }
    endPoint = points.back()+0.5*segmentNormals.back();
    glVertex3d(points.back().x, points.back().y, points.back().z);
    glVertex3d(endPoint.x, endPoint.y, endPoint.z);
    glEnd();*/

    // The points
    glColor3f(0.,1.,0.);
    glPointSize(10.);
    glBegin(GL_POINTS);
    for(unsigned int i=0; i<points.size(); i++) glVertex3d(points[i].x, points[i].y, points[i].z);
    glEnd();

    // The cutting lines
    /*glPopMatrix();
    glColor3f(0.,1.,1.);
    glBegin(GL_LINES);
    for(unsigned int i=0; i<cuttingLines.size(); i++){
        endPoint = points[i+1]+0.5*cuttingLines[i];
        glVertex3d(points[i+1].x, points[i+1].y, points[i+1].z);
        glVertex3d(endPoint.x, endPoint.y, endPoint.z);
    }
    glEnd();*/
}

void Polyline::update(const std::vector<Vec> &newPoints){
    points.clear();
    for(unsigned int i=0; i<newPoints.size(); i++) points.push_back(newPoints[i]);
}

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

void Polyline::bend(unsigned int index, Vec &newPosition, std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals){
    if(index >= points.size()-1) return;

    const Vec &origin = points[index+1];
    points[index] = newPosition;
    if(index!=0) recalculateNormal(index-1, newPosition, points[index-1]);
    recalculateNormal(index, origin, newPosition);

    getCuttingAngles(relativeNorms, planeNormals);

    for(unsigned int i=0; i<segmentNormals.size(); i++){
        displayNormals.push_back(segmentNormals[i]);
        displayNormals.push_back(segmentNormals[i]);        // one for each end of the box
    }
}

void Polyline::bendNormals(unsigned int index, Vec &newPosition){
    /*if(index >= points.size()-1) return;

    const Vec &origin = points[index+1];
    if(index!=0) recalculateNormal(index-1, newPosition, points[index-1]);
    recalculateNormal(index, origin, newPosition);

    std::vector<Vec> relativeNorms;
    getCuttingAngles(relativeNorms);*/
}

void Polyline::recalculateNormal(unsigned int index, const Vec &origin, const Vec &newPosition){
    Vec pos = newPosition - origin;
    segmentNormals[index] = -cross(pos, binormal);
    segmentNormals[index].normalize();
}

void Polyline::rotateSegment(unsigned int index, double theta, const Vec &axis){
    Quaternion r(cos(theta/2.0)*axis.x, cos(theta/2.0)*axis.y, cos(theta/2.0)*axis.z, sin(theta/2.0));      // rotation

    Frame f = Frame();
    initialiseFrame(f);
    f.rotate(r);

    segmentNormals[index] = f.localInverseTransformOf(normal);
}

void Polyline::initialiseFrame(Frame &f){
    Quaternion q = Quaternion();            // the base
    q.setFromRotatedBasis(Vec(1,0,0),Vec(0,1,0),Vec(0,0,1));
    f.setOrientation(q);
}

void Polyline::getCuttingAngles(std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals){
    cuttingLines.clear();

    for(unsigned int i=0; i<segmentNormals.size()-1; i++){
        Vec v = segmentNormals[i] + segmentNormals[i+1];
        v /= 2.0;
        v.normalize();
        cuttingLines.push_back(v);
    }

    // get the relative normals
    relativeNorms.clear();

    for(unsigned int i=0; i<cuttingLines.size(); i++){
        Frame f = Frame();
        initialiseFrame(f);
        Quaternion q = Quaternion(cuttingLines[i], normal);
        f.rotate(q);

        relativeNorms.push_back(f.localInverseTransformOf(segmentNormals[i]));      // save for the fibula
        relativeNorms.push_back(f.localInverseTransformOf(segmentNormals[i+1]));

        planeNormals.push_back(cuttingLines[i]);            // save for the mandible
    }

}

void Polyline::updateNormals(const std::vector<Vec> &relativeNorms){
    displayNormals.clear();

    displayNormals.push_back(segmentNormals[0]);
    for(unsigned int i=0; i<relativeNorms.size(); i++){
        displayNormals.push_back(relativeNorms[i]);
    }
    displayNormals.push_back(segmentNormals.back());
}

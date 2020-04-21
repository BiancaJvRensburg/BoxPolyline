#include "polyline.h"

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
    segmentNormals.clear();
    segmentBinormals.clear();
    cuttingLines.clear();
    cuttingBinormals.clear();

    for(unsigned int i=0; i<nbPoints; i++) points.push_back(Vec(i, 0, 0));
    for(unsigned int i=0; i<points.size()-1; i++) segmentNormals.push_back(normal);
    for(unsigned int i=0; i<points.size()-1; i++) segmentBinormals.push_back(binormal);
    for(unsigned int i=1; i<points.size()-1; i++) cuttingLines.push_back(normal);
    for(unsigned int i=1; i<points.size()-1; i++) cuttingBinormals.push_back(binormal);
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
   /* glColor3f(1., 0., 0.);
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

    glColor3f(1., 1., 0.);
        glBegin(GL_LINES);
        endPoint = points[0]+0.5*segmentBinormals[0];
        glVertex3f(points[0].x, points[0].y, points[0].z);
        glVertex3f(endPoint.x, endPoint.y, endPoint.z);
        for(unsigned int i=1; i<points.size()-1; i++){
            for(unsigned int j=0; j<2; j++){
                endPoint = points[i]+0.5*segmentBinormals[i-j];
                glVertex3f(points[i].x, points[i].y, points[i].z);
                glVertex3f(endPoint.x, endPoint.y, endPoint.z);
            }
        }
        endPoint = points.back()+0.5*segmentBinormals.back();
        glVertex3f(points.back().x, points.back().y, points.back().z);
        glVertex3f(endPoint.x, endPoint.y, endPoint.z);
        glEnd();*/


    // The points
    glColor3f(0.,1.,0.);
    glPointSize(10.);
    glBegin(GL_POINTS);
    for(unsigned int i=0; i<points.size(); i++) glVertex3d(points[i].x, points[i].y, points[i].z);
    glEnd();

    // Binormal
    /*glColor3f(0.,1.,0.);
    glPointSize(10.);
    glBegin(GL_LINES);
    for(unsigned int i=0; i<points.size(); i++){
        endPoint = points[i]+0.5*segmentNormals[i];
        glVertex3d(points[i].x, points[i].y, points[i].z);
        glVertex3f(endPoint.x, endPoint.y, endPoint.z);
    }
    glEnd();*/

    // The cutting lines
    /*glPopMatrix();
    glColor3f(0.,1.,1.);
    glBegin(GL_LINES);
    for(unsigned int i=0; i<cuttingLines.size(); i++){
        Vec endPoint = points[i+1]+0.5*cuttingLines[i];
        glVertex3d(points[i+1].x, points[i+1].y, points[i+1].z);
        glVertex3d(endPoint.x, endPoint.y, endPoint.z);
    }
    glEnd();

    glPopMatrix();
        glColor3f(1.,0.,1.);
        glBegin(GL_LINES);
        for(unsigned int i=0; i<cuttingLines.size(); i++){
            Vec endPoint = points[i+1]+0.5*cuttingBinormals[i];
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

void Polyline::bend(unsigned int index, Vec &newPosition, std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals){
    if(index >= points.size()) return;

    points[index] = newPosition;

    if(index!=0) recalculateBinormal(index-1, points[index-1], points[index]);
    if(index!=points.size()-1) recalculateBinormal(index, points[index], points[index+1]);

    /*if(index!=0) recalculateNormal(index-1, points[index], points[index-1]);
    recalculateNormal(index, origin, points[index]);*/

    getCuttingAngles(relativeNorms, planeNormals, planeBinormals);
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
    q.setFromRotatedBasis(Vec(1,0,0),Vec(0,1,0),Vec(0,0,1));
    f.setOrientation(q);
}

void Polyline::getCuttingAngles(std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals){
    cuttingLines.clear();
    cuttingBinormals.clear();
    relativeNorms.clear();
    planeNormals.clear();
    planeBinormals.clear();

    for(unsigned int i=0; i<segmentNormals.size()-1; i++){
        Vec v = segmentNormals[i] + segmentNormals[i+1];
        v /= 2.0;
        v.normalize();
        cuttingLines.push_back(v);
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

Vec Polyline::averageVector(const std::vector<Vec> &v){
    Vec r(0,0,0);
    double totalNb = v.size();

    for(unsigned int i=0; i<v.size(); i++) r += v[i];
    r /= totalNb;

    return r;
}

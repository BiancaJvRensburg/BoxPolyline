#include "polyline.h"

Polyline::Polyline()
{
    frame = ManipulatedFrame();

    for(unsigned int i=0; i<4; i++) points.push_back(Vec(i, 0, 0));
    for(unsigned int i=0; i<points.size()-1; i++) segmentNormals.push_back(normal);
    for(unsigned int i=0; i<points.size()-1; i++) segmentBinormals.push_back(binormal);
    for(unsigned int i=1; i<points.size()-1; i++) cuttingLines.push_back(normal);
    for(unsigned int i=1; i<points.size()-1; i++) cuttingBinormals.push_back(binormal);
    for(unsigned int i=0; i<points.size()-1; i++){
        boxes.push_back(Frame());
        initialiseFrame(boxes[i]);
        boxes[i].setPosition(points[i]);
    }
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

    /*for(unsigned int i=0; i<boxes.size(); i++){
        glPushMatrix();
        glMultMatrixd(boxes[i].matrix());
        glColor3f(0.,1.,1.);
        glBegin(GL_LINES);
        for(unsigned int i=0; i<points.size()-1; i++){
            double size = 0.3;
            glVertex3d(0, 0, 0);
            glVertex3d(size, 0, 0);
            glVertex3d(0, 0, 0);
            glVertex3d(0, size, 0);
            glVertex3d(0, 0, 0);
            glVertex3d(0, 0, size);
        }
        glEnd();
        glPopMatrix();
    }*/

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
        glEnd();

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
    glPopMatrix();
    glColor3f(0.,1.,1.);
    glBegin(GL_LINES);
    for(unsigned int i=0; i<cuttingLines.size(); i++){
        endPoint = points[i+1]+0.5*cuttingLines[i];
        glVertex3d(points[i+1].x, points[i+1].y, points[i+1].z);
        glVertex3d(endPoint.x, endPoint.y, endPoint.z);
    }
    glEnd();

    glPopMatrix();
        glColor3f(0.,1.,1.);
        glBegin(GL_LINES);
        for(unsigned int i=0; i<cuttingLines.size(); i++){
            Vec endPoint = points[i+1]+0.5*cuttingBinormals[i];
            glVertex3d(points[i+1].x, points[i+1].y, points[i+1].z);
            glVertex3d(endPoint.x, endPoint.y, endPoint.z);
        }
        glEnd();
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
    if(index >= points.size()-1) return;

    const Vec &origin = points[index+1];

   /* Vec seg1b = points[index] - points[index-1];
    Vec seg2b = points[index+1] - points[index];

    std::cout << "Seg 1 before : " << seg1b.x << "," << seg1b.y << "," << seg1b.z << std::endl;
    std::cout << "Seg 2 before : " << seg2b.x << "," << seg2b.y << "," << seg2b.z << std::endl;*/

    points[index] = newPosition;
   /* boxes[index].setPosition(newPosition);

    Vec seg1a = points[index] - points[index-1];
    Vec seg2a = points[index+1] - points[index];

    std::cout << "Seg 1 before : " << seg1a.x << "," << seg1a.y << "," << seg1a.z << std::endl;
    std::cout << "Seg 2 before : " << seg2a.x << "," << seg2a.y << "," << seg2a.z << std::endl;


    Quaternion q(seg1b, seg1a);
    Quaternion q2(seg2b, seg2a);

    boxes[index-1].setOrientation(q);
    boxes[index].setOrientation(q2);*/

    if(index!=0) recalculateBinormal(index-1, points[index-1], points[index]);
    recalculateBinormal(index, points[index], points[index+1]);

    if(index!=0) recalculateNormal(index-1, newPosition, points[index-1]);
    recalculateNormal(index, origin, newPosition);

    getCuttingAngles(relativeNorms, planeNormals, planeBinormals);

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
    //pos.y = 0;
    segmentNormals[index] = -cross(pos, segmentBinormals[index]);
    segmentNormals[index].normalize();
    //std::cout << "Normal " << index << " : " << segmentNormals[index].x << "," << segmentNormals[index].y << "," << segmentNormals[index].z << std::endl;
    //recalculateBinormal(index, origin, newPosition);

    //std::cout << "Dot product : " << segmentNormals[index]*segmentBinormals[index] << std::endl;
}

void Polyline::recalculateBinormal(unsigned int index, const Vec &origin, const Vec &newPosition){
   // Calculate an orthogonal vector on the plane
    Vec pos = newPosition - origin;
    pos.normalize();
    pos.z = 0;        // The new polyline projected in the z plane
    //std::cout << "Pos " << index << " : " << pos.x << "," << pos.y << "," << pos.z << std::endl;

    double theta = angle(pos, tangent);        // get the angle which the tangent rotated
    if(pos.y <0) theta = -theta;        // rotate the opposite way
    //if(theta >= 180) theta -= 180.;
    //std::cout << "angle : " << theta*180./M_PI << std::endl;
    //rotateSegment(segmentBinormals[index], theta, normal, binormal);
    double x = binormal.x * cos(theta) - binormal.y * sin(theta);
    double y = binormal.x * sin(theta) + binormal.y * cos(theta);
    segmentBinormals[index] = Vec(x,y,0);
    //segmentBinormals[index].normalize();
    //std::cout << "Binormal " << index << " : " << segmentBinormals[index].x << "," << segmentBinormals[index].y << "," << segmentBinormals[index].z << std::endl;


    /* Vec seg1 = points[index-1] - points[index];
    Vec seg2 = points[index+1] - points[index];
    seg1.y = 0;
    seg2.y = 0;
    seg1.normalize();
    seg2.normalize();

    pointBinormals[index] = (seg1 + seg2) / 2.;*/

    /*Vec pos = newPosition - origin;
    //pos.z = 0;
    pos.normalize();
    segmentBinormals[index] = cross(pos, segmentNormals[index]);
    segmentBinormals[index].normalize();*/
   // std::cout << "Binormal " << index << " : " << segmentBinormals[index].x << "," << segmentBinormals[index].y << "," << segmentBinormals[index].z << std::endl;
}

void Polyline::rotateSegment(Vec &seg, double theta, const Vec &axis, const Vec &axisToMove){
    seg = vectorQuaternionRotation(theta, axis, axisToMove);
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

Quaternion Polyline::getRotationQuaternion(const Vec &axis, const double &theta){
    return Quaternion(cos(theta/2.0)*axis.x, cos(theta/2.0)*axis.y, cos(theta/2.0)*axis.z, sin(theta/2.0));
}

void Polyline::getCuttingAngles(std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals){
    cuttingLines.clear();
    cuttingBinormals.clear();

    for(unsigned int i=0; i<segmentNormals.size()-1; i++){
        Vec v = segmentNormals[i] + segmentNormals[i+1];
        v /= 2.0;
        v.normalize();
        cuttingLines.push_back(v);
        Vec b = segmentBinormals[i] + segmentBinormals[i+1];
        b /= 2.0;
        b.normalize();
        cuttingBinormals.push_back(b);
        double theta = angle(cuttingLines.back(), cuttingBinormals.back());
        double alpha = M_PI / 2.0 - theta + M_PI;
        Vec axis = cross(cuttingLines.back(), cuttingBinormals.back());
        cuttingLines.back() = vectorQuaternionRotation(alpha, axis, cuttingLines.back());
        //std::cout << "Dot product " << i << " : " << cuttingBinormals[i]*cuttingLines[i] << std::endl;
    }

    // get the relative normals
    relativeNorms.clear();

    for(unsigned int i=0; i<cuttingLines.size(); i++){
        Frame f = Frame();
        initialiseFrame(f);
        Quaternion q = Quaternion(cuttingLines[i], normal);
        f.rotate(q);

        /*Vec rotationAxis = f.localTransformOf(normal);
        double alpha = angle(getCuttingBinormal(i+1), binormal);
        Quaternion qbi = getRotationQuaternion(rotationAxis, alpha);
        f.rotate(qbi);*/

       /* double theta = angle(cuttingLines[i], normal);

        Frame f = Frame();
        initialiseFrame(f);
        Quaternion q = getRotationQuaternion(binormal, -theta+M_PI);
        f.rotate(q);*/

        //cuttingBinormals.push_back(getCuttingBinormal(i+1));

        relativeNorms.push_back(f.localInverseTransformOf(segmentNormals[i]));      // save for the fibula
        relativeNorms.push_back(f.localInverseTransformOf(segmentNormals[i+1]));

        planeNormals.push_back(cuttingLines[i]);            // save for the mandible
        planeBinormals.push_back(cuttingBinormals[i]);
    }

}

Vec Polyline::getCuttingBinormal(unsigned int index){
    Vec p0(points[index-1] - points[index]);
    Vec p1(points[index+1] - points[index]);
    p0.z = 0;
    p1.z = 0;
    p0.normalize();
    p1.normalize();

    Vec bi = (p0+p1)/2.;
    bi.normalize();

    return bi;
}

void Polyline::updateNormals(const std::vector<Vec> &relativeNorms){
    displayNormals.clear();

    displayNormals.push_back(segmentNormals[0]);
    for(unsigned int i=0; i<relativeNorms.size(); i++){
        displayNormals.push_back(relativeNorms[i]);
    }
    displayNormals.push_back(segmentNormals.back());
}

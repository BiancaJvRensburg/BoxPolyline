#include "polyline.h"
#include "Tools/point3.h"

Polyline::Polyline()
{
    frame = ManipulatedFrame();
}

// This is only called once. Used to set the reference frame.
void Polyline::init(const Frame *const refFrame, unsigned int nbPoints){
    frame.setReferenceFrame(refFrame);
    reinit(nbPoints);
}

/* A simple reinitialisation where we set all the normals to the polyline normal
 * and create an evenly spaced polyline with the correct number of points.
 * Note: the points are not correctly positioned here, this is simply a initialisation.
*/
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
    for(unsigned int i=1; i<points.size()-1; i++) cuttingBinormals.push_back(binormal);
    for(unsigned int i=0; i<points.size()-1; i++){
        boxes.push_back(Box());
        boxes[i].init(frame.referenceFrame());
    }
}

void Polyline::draw(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH);

    glPushMatrix();
    glMultMatrixd(frame.matrix());

    glColor3f(1.,1.,1.);

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

    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    for(unsigned int i=0; i<boxes.size(); i++){
        glColor4f(0,0,0, boxTransparency);
        boxes[i].draw(0);
    }
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

    if(!isWireframe){
        for(unsigned int i=0; i<boxes.size(); i++){
            glColor4f(0,i%2,(i+1)%2, boxTransparency);
            boxes[i].draw(-0.1);
        }
    }

    glPopMatrix();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH);
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
        glVertex3d(p0.x, p0.y, p0.z);
        glVertex3d(p1.x, p1.y, p1.z);

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

// Update the points locations without updating their orientations
void Polyline::updatePoints(const std::vector<Vec> &newPoints){
    points.clear();
    for(unsigned int i=0; i<newPoints.size(); i++) points.push_back(newPoints[i]);
    for(unsigned int i=0; i<boxes.size(); i++){
        resetBox(i);
        boxes[i].restoreRotation();
    }
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

// Match the box to the current position and orientation of a point index
void Polyline::resetBox(unsigned int index){
    boxes[index].setPosition(points[index]);
    const Vec& n = segmentNormals[index];
    const Vec& b = segmentBinormals[index];

    boxes[index].setFrameFromBasis(-cross(n,b),b,n);
    double length = euclideanDistance(points[index], points[index+1]);
    boxes[index].setLength(length);
}

void Polyline::bend(unsigned int index, Vec &newPosition, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals){
    if(index >= points.size()) return;

    bendFibula(index, newPosition);     // the fibula is bent in the same way but does not need to send info

    getCuttingAngles(planeNormals, planeBinormals);     // Get the normals and binormals of the planes which interset two boxes at a polyline point
}

void Polyline::bendFibula(unsigned int index, Vec &newPosition){
    if(index >= points.size()) return;

    points[index] = getLocalCoordinates(newPosition);

    // Recalculate the orientations of the two boxes attached to the point index (the box behind and the box infront)
    if(index!=0){
        recalculateBinormal(index-1, points[index-1], points[index]);
        resetBox(index-1);
        boxes[index-1].restoreRotation();
    }
    if(index!=points.size()-1){
        recalculateBinormal(index, points[index], points[index+1]);
        resetBox(index);
        boxes[index].restoreRotation();
    }
}

void Polyline::recalculateOrientations(){
    for(unsigned int i=0; i<points.size()-1; i++) recalculateBinormal(i, points[i], points[i+1]);
}

// Recalculate the normal as a cross product of the binormal and the tangent
void Polyline::recalculateNormal(unsigned int index, const Vec &origin, const Vec &newPosition){
    Vec pos = newPosition - origin;
    pos.normalize();

    segmentNormals[index] = -cross(pos, segmentBinormals[index]);
    segmentNormals[index].normalize();
}

void Polyline::recalculateBinormal(unsigned int index, const Vec &origin, const Vec &newPosition){
    /*Vec pos = newPosition - origin;     // Calculate an orthogonal vector on the plane
    pos.normalize();
    pos.z = 0;        // The new polyline projected in the z plane

    double theta = angle(pos, tangent);        // get the angle which the tangent rotated
    if(pos.y <0) theta = -theta;        // rotate the opposite way

    // Rotate the CONSTANT binormal around the z axis
    double x = binormal.x * cos(theta) - binormal.y * sin(theta);
    double y = binormal.x * sin(theta) + binormal.y * cos(theta);
    segmentBinormals[index] = Vec(x,y,0);
    segmentBinormals[index].normalize();

    recalculateNormal(index, newPosition, origin);*/

         point3d  Nprev = normal;
         point3d  Tprev = tangent;

         point3d  T0 = newPosition - origin;        // the polyline tangent
         point3d  N0 = T0.getOrthogonal(); //index == 0 ? T0.getOrthogonal() :
         point3d::rotateVectorSimilarly( Nprev , Tprev , T0  );

         point3d  B0 = point3d::cross( T0 , N0 );

         segmentNormals[index] = - Vec(B0);
         segmentBinormals[index] = - Vec(N0);
         segmentNormals[index].normalize();
         segmentBinormals[index].normalize();
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

// Get the normals and binormals of the planes which interset two boxes at a polyline point in order to send it to the fibula
void Polyline::getCuttingAngles(std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals){
    cuttingLines.clear();
    cuttingBinormals.clear();
    planeNormals.clear();
    planeBinormals.clear();

    planeNormals.push_back(segmentNormals[0]);      // for the first plane (there's only one box attached to the first and last point)
    planeBinormals.push_back(segmentBinormals[0]);

    // Get the angle between the two boxes
    for(unsigned int i=0; i<segmentNormals.size()-1; i++){
        Vec v = (segmentNormals[i] + segmentNormals[i+1]);
        v.normalize();
        cuttingLines.push_back(v);

        Vec b = (segmentBinormals[i] + segmentBinormals[i+1]);
        b.z /= 2.;
        b.normalize();
        cuttingBinormals.push_back(b);
    }

    // Make sure the normal and binormal have a right angle between them
    for(unsigned int i=0; i<cuttingLines.size(); i++){
        double theta = angle(cuttingLines[i], cuttingBinormals[i]);     // Get the current angle between them
        double alpha = M_PI / 2.0 - theta + M_PI;           // Get the angle it needs to rotate in order for it to be a right angle
        Vec axis = cross(cuttingLines[i], cuttingBinormals[i]);     // Get the axis on which it has to rotate (which is orthogonal to the current normal and binormal)
        cuttingLines[i] = vectorQuaternionRotation(alpha, axis, cuttingLines[i]);       // Rotate the normal so its now at a right angle to the binormal

        planeNormals.push_back(cuttingLines[i]);            // save for the mandible
        planeBinormals.push_back(cuttingBinormals[i]);
    }

    planeNormals.push_back(segmentNormals.back());      // for the last plane
    planeBinormals.push_back(segmentBinormals.back());
}

// Get the distances between the polyline points
void Polyline::getDistances(std::vector<double> &distances){
    distances.clear();

    for(unsigned int i=0; i<points.size()-1; i++){
        distances.push_back(euclideanDistance(points[i], points[i+1]));
    }
}

double Polyline::euclideanDistance(const Vec &a, const Vec &b){
    return sqrt(pow(a.x-b.x, 2.) + pow(a.y-b.y, 2.) + pow(a.z-b.z, 2.));
}

// Move the polyline point by the vector toLower
void Polyline::lowerPoint(unsigned int index, const Vec &toLower){
    points[index] += toLower;
}

// Reset the boxes to the current orientation, position and length of the polyline
void Polyline::resetBoxes(){
    recalculateOrientations();
    for(unsigned int i=0; i<boxes.size(); i++) resetBox(i);
}

// Rotate the box on its own axis
void Polyline::rotateBox(unsigned int i, double angle){
    boxes[i].rotateOnAxis(angle);
}

void Polyline::restoreBoxRotations(){
    for(unsigned int i=0; i<boxes.size(); i++) boxes[i].restoreRotation();
}


// Convert the vectors from the box frame to the world frame
void Polyline::getRelatvieNormals(std::vector<Vec> &relativeNorms){
    for(unsigned int i=0; i<relativeNorms.size(); i++){
        unsigned int boxID = i/4+1;
        relativeNorms[i] = getWorldTransform(boxes[boxID].worldTransform(relativeNorms[i]));
    }
}

// Get each plane in relation to each of its corresponding boxes
void Polyline::getRelativePlane(Plane &p, std::vector<Vec> &norms){
    // Get the plane norms in terms of the mesh
    Vec n(1,0,0);
    Vec b(0,1,0);
    n = p.getMeshVectorFromLocal(n);
    b = p.getMeshVectorFromLocal(b);

    // Now get it in terms of its boxes
    norms.clear();
    const unsigned int& id = p.getID();
    if(id!=0){
        norms.push_back(boxes[id-1].localTransform(n));
        norms.push_back(boxes[id-1].localTransform(b));
    }
    if(id<boxes.size()){
        norms.push_back(boxes[id].localTransform(n));
        norms.push_back(boxes[id].localTransform(b));
    }
}

// Get the vector direction of each segment of the polyline (in terms of the world)
void Polyline::getDirections(std::vector<Vec> &directions){
    directions.clear();
    for(unsigned int i=0; i<boxes.size(); i++) directions.push_back(getWorldTransform(boxes[i].worldTangent()));
}


// Move the entire polyline
void Polyline::lowerPolyline(Vec localDirection, double distance){
    Vec p = frame.position();
    Vec worldDirection = getWorldTransform(localDirection);     // convert to world coordinates
    frame.setPosition(p+worldDirection*distance);
}

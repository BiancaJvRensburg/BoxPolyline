#include "box.h"

Box::Box()
{
    f = Frame();
    dimensions = Vec(100., 25., 25.);
    tangent = Vec(1.,0.,0.);
    binormal = Vec(0.,1.,0.);
    normal = Vec(0.,0.,1.);
    prevRotation = 0.;
}

void Box::init(const Frame *ref){
    f.setReferenceFrame(ref);
}

void Box::setFrameFromBasis(Vec x, Vec y, Vec z){
    x.normalize();
    y.normalize();
    z.normalize();

    Quaternion q;
    q.setFromRotatedBasis(x,y,z);
    f.setOrientation(q);
}

void Box::draw(){
    glPushMatrix();
    glMultMatrixd(f.matrix());

    //glColor3f(1.,1.,1.);

    QGLViewer::drawAxis(40.);

    const double& length = getLength();
    const double& width = getWidth();
    const double& height = getHeight();

    const Vec& p0 = Vec(0,0,0);
    const Vec& p1 = p0 + length*tangent;
    Vec p2 = p0 + binormal*width;
    Vec p3 = p1 + binormal*width;
    Vec p4 = p0 + normal*height;
    Vec p5 = p1 + normal*height;
    Vec p6 = p0 + normal*height +  binormal*width;
    Vec p7 = p1 + normal*height +  binormal*width;

    glBegin(GL_QUADS);
        glVertex3d(p0.x, p0.y, p0.z);
        glVertex3d(p1.x, p1.y, p1.z);
        glVertex3d(p5.x, p5.y, p5.z);
        glVertex3d(p4.x, p4.y, p4.z);
    glEnd();

    glBegin(GL_QUADS);
        glVertex3d(p0.x, p0.y, p0.z);
        glVertex3d(p1.x, p1.y, p1.z);
        glVertex3d(p3.x, p3.y, p3.z);
        glVertex3d(p2.x, p2.y, p2.z);
    glEnd();

    glBegin(GL_QUADS);
        glVertex3d(p2.x, p2.y, p2.z);
        glVertex3d(p3.x, p3.y, p3.z);
        glVertex3d(p7.x, p7.y, p7.z);
        glVertex3d(p6.x, p6.y, p6.z);
    glEnd();

    glBegin(GL_QUADS);
        glVertex3d(p6.x, p6.y, p6.z);
        glVertex3d(p7.x, p7.y, p7.z);
        glVertex3d(p5.x, p5.y, p5.z);
        glVertex3d(p4.x, p4.y, p4.z);
    glEnd();

    glBegin(GL_QUADS);
        glVertex3d(p1.x, p1.y, p1.z);
        glVertex3d(p3.x, p3.y, p3.z);
        glVertex3d(p7.x, p7.y, p7.z);
        glVertex3d(p5.x, p5.y, p5.z);
    glEnd();

    glBegin(GL_QUADS);
        glVertex3d(p0.x, p0.y, p0.z);
        glVertex3d(p2.x, p2.y, p2.z);
        glVertex3d(p6.x, p6.y, p6.z);
        glVertex3d(p4.x, p4.y, p4.z);
    glEnd();

    /*glBegin(GL_LINES);
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

    glEnd();*/

    glPopMatrix();
}

void Box::rotateOnAxis(double angle){
    double alpha = angle - prevRotation;
    prevRotation = angle;
    Vec centre = f.localInverseCoordinatesOf(dimensions/2.);

    Vec axis(1,0,0);
    Quaternion r(axis, alpha);

    f.rotateAroundPoint(r, centre);
}

void Box::restoreRotation(){
    Vec centre = f.localInverseCoordinatesOf(dimensions/2.);

    Vec axis(1,0,0);
    Quaternion r(axis, prevRotation);

    f.rotateAroundPoint(r, centre);
}

Vec Box::getLocation(){
    return f.position();
}

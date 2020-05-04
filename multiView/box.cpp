#include "box.h"

Box::Box()
{
    f = Frame();
    length = 0;
    tangent = Vec(1,0,0);
    binormal = Vec(0,1,0);
    normal = Vec(0,0,1);
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

    QGLViewer::drawAxis(40.);

    double size = 25.;
    const Vec& p0 = Vec(0,0,0);
    const Vec& p1 = p0 + length*tangent;
    Vec p2 = p0 + binormal*size;
    Vec p3 = p1 + binormal*size;
    Vec p4 = p0 + normal*size;
    Vec p5 = p1 + normal*size;
    Vec p6 = p0 + (normal +  binormal)*size;
    Vec p7 = p1 + (normal + binormal)*size;

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

    glPopMatrix();
}

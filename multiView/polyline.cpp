#include "polyline.h"

Polyline::Polyline()
{
    frame = ManipulatedFrame();

    tangent = Vec(1,0,0);
    normal = Vec(0,0,1);

    for(unsigned int i=0; i<4; i++) points.push_back(Vec(i, 0, 0));
}

void Polyline::init(const Frame *const refFrame){
    frame.setReferenceFrame(refFrame);
}

void Polyline::draw(){
    glPushMatrix();
    glMultMatrixd(frame.matrix());

    glLineWidth(5.);
    glColor3f(0.,0.,1.);
    glBegin(GL_LINES);
    for(unsigned int i=0; i<points.size()-1; i++){
        glVertex3f(points[i].x, points[i].y, points[i].z);
        glVertex3f(points[i+1].x, points[i+1].y, points[i+1].z);
    }
    glEnd();

    glPointSize(10.);
    glBegin(GL_POINTS);
    for(unsigned int i=0; i<points.size(); i++) glVertex3f(points[i].x, points[i].y, points[i].z);
    glEnd();

    glPopMatrix();
}

#include "controlpoint.h"
#include <math.h>

#include <QGLViewer/manipulatedFrame.h>

ControlPoint::ControlPoint()
{
    this->p = Vec(0,0,0);
    initialise();
    isSwitchFrames = true;
}

ControlPoint::ControlPoint(const Vec& p)
{
    this->p = p;
    initialise();
    isSwitchFrames = true;
}

ControlPoint::ControlPoint(double x, double y, double z)
{
    this->p = Vec(x,y,z);
    initialise();
}

// Set the manipulated frame (may be activated at a later stage)
void ControlPoint::initialise(){
    mf = Frame();
    mf.setPosition(this->p.x, this->p.y, this->p.z);
}

void ControlPoint::draw(){

    if(isSwitchFrames){
        glPushMatrix();
        glMultMatrixd(mf.matrix());
    }

    glPointSize(10.0);
    glColor3f(1.0, 0.0, 0.);
    glBegin(GL_POINTS);
        glVertex3d(0, 0, 0);
    glEnd();

    glPointSize(1.0);
    glColor3f(1.0,1.0,1.0);

    if(isSwitchFrames) glPopMatrix();
}

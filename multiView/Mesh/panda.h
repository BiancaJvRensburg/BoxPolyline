#ifndef PANDA_H
#define PANDA_H

#include <QGLViewer/qglviewer.h>
#include "simplemesh.h"
#include "QGLViewer/manipulatedFrame.h"
#include "Manipulator/simplemanipulator.h"

using namespace qglviewer;

class Panda
{
public:
    Panda();

    void draw();

    void setLocation(const Vec &v){ f.setPosition(v); }
    void setOrientation(const Quaternion &q){ f.setOrientation(q); }
    void rotate(Vec axis, double angle){ f.rotate(Quaternion(axis, angle)); }

private:
    void openOFF(QString f, SimpleMesh &m);

    SimpleMesh effector, navex, marker, wrj;
    Frame f;
};

#endif // PANDA_H

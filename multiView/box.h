#ifndef BOX_H
#define BOX_H

#include <QGLViewer/qglviewer.h>
#include <QGLViewer/frame.h>

using namespace qglviewer;

class Box
{
public:
    Box();
    void draw();
    void init(const Frame *ref);
    void setPosition(const Vec& v){ f.setPosition(v); }
    void setFrameFromBasis(Vec x, Vec y, Vec z);
    void setLength(double l){ length = l; }

private:
    Frame f;
    Vec normal, binormal, tangent;
    double length;
};

#endif // BOX_H

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
    void setLength(double l){ dimensions.x = l; }
    void setWidth(double l){ dimensions.y = l; }
    void setHeight(double l){ dimensions.z = l; }
    const double& getLength(){ return dimensions.x; }
    const double& getWidth(){ return dimensions.y; }
    const double& getHeight(){ return dimensions.z; }
    void rotateOnAxis(double angle);
    Vec localTransform(Vec v){ return f.localTransformOf(v); }
    Vec worldTransform(Vec v){ return f.localInverseTransformOf(v); }

private:
    Frame f;
    Vec normal, binormal, tangent;
    Vec dimensions;
    double prevRotation;
};

#endif // BOX_H

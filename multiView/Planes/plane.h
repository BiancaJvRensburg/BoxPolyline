#ifndef PLANE_H
#define PLANE_H

#include <QGLViewer/qglviewer.h>
#include <QGLViewer/manipulatedFrame.h>

#include "curvepoint.h"

enum Movable {STATIC, DYNAMIC};

using namespace qglviewer;

class Plane
{
public:
    Plane(double s, Movable status, float alpha, unsigned int id);

    void toggleIsVisible(){ isVisible = !isVisible; }
    void setAlpha(float alpha){ this->alpha = alpha; }
    float getAlpha(){ return alpha; }
    void setID(unsigned int id){ this->id = id; cp.setID(id); }
    const unsigned int& getID(){ return this->id; }
    void setSize(double s){ size = s; }
    bool getIsPoly(){ return isPoly; }

    void setPosition(Vec pos);
    void setOrientation(Quaternion q){ cp.getFrame().setOrientation(q); }
    Quaternion fromRotatedBasis(Vec x, Vec y, Vec z);
    void setFrameFromBasis(Vec x, Vec y, Vec z);

    void rotatePlane(Vec axis, double angle);
    void rotatePlaneXY(double percentage);
    void setPlaneRotation(Vec axis, double angle);
    void constrainZRotation(){ cp.getFrame().setConstraint(&constraint); }
    void freeZRotation(){ cp.getFrame().setConstraint(&constraintFree); }
    void draw();
    const Frame* getReferenceFrame(){ return cp.getReferenceFrame(); }

    // Mesh calculations
    bool isIntersection(Vec v0, Vec v1, Vec v2);
    double getSign(Vec v);

    Vec getNormal(){ return normal; }
    Vec getProjection(Vec p);
    Vec getLocalProjection(Vec p);      // for vectors already in local coordinates
    Vec& getPosition(){ return cp.getPoint(); }
    CurvePoint& getCurvePoint(){ return cp; }

    Vec getLocalCoordinates(Vec v) { return cp.getFrame().localCoordinatesOf(v); }    // same as get polyline
    Vec getMeshCoordinatesFromLocal(Vec v){ return cp.getFrame().localInverseCoordinatesOf(v); }
    Vec getLocalVector(Vec v) { return cp.getFrame().localTransformOf(v); }    // same as get polyline
    Vec getMeshVectorFromLocal(Vec v){ return cp.getFrame().localInverseTransformOf(v); }

    void setRotation(Quaternion q) { cp.getFrame().setRotation(q); }
    void rotate(Quaternion q) { cp.getFrame().rotate(q); }

    bool isIntersectionPlane(Vec &v0, Vec &v1, Vec &v2, Vec &v3);
    void getCorners(Vec &v0, Vec &v1, Vec &v2, Vec &v3);

    const double& getSize(){ return size; }

    void toggleIsPoly();

    Movable status;

private:
    void initBasePlane();

    AxisPlaneConstraint constraint;
    AxisPlaneConstraint constraintFree;
    Vec points[4];
    double size;
    double rotationPercentage;
    Vec normal;
    CurvePoint cp;
    bool isVisible;
    float alpha;
    unsigned int id;
    bool isPoly;
};

#endif // PLANE_H
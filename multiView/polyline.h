#ifndef POLYLINE_H
#define POLYLINE_H

#include "curvepoint.h"

class Polyline
{
public:
    Polyline();

    void draw();
    void init(const Frame *const refFrame);
    void update(const std::vector<Vec> &newPoints);
    int getNbPoints(){ return points.size(); }
    void bend(unsigned int index, Vec &newPosition);

private:
    Vec projection(Vec &a, Vec &planeNormal);
    double angle(Vec &a, Vec &b);
    double getBendAngle(Vec &a, Vec &b);
    void rotateSegment(unsigned int index, double angle, const Vec &axis);
    void recalculateNormal(unsigned int index, const Vec &origin, const Vec &newPoint);

    ManipulatedFrame frame;
    const Vec tangent = Vec(1,0,0);
    const Vec normal = Vec(0,0,1);
    const Vec binormal = Vec(0,1,0);
    std::vector<Vec> points;
    std::vector<Vec> segmentNormals;
};

#endif // POLYLINE_H

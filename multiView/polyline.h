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
    Vec& getPoint(unsigned int i){return points[i];}
    Vec getWorldCoordinates(const Vec& v){ return frame.inverseCoordinatesOf(v);}
    Vec getWorldTransform(const Vec& v){ return frame.inverseTransformOf(v);}
    void bend(unsigned int index, Vec &newPosition, std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals);
    void bendNormals(unsigned int index, Vec &newPosition);
    void updateNormals(const std::vector<Vec>& relativeNorms);

private:
    Vec projection(Vec &a, Vec &planeNormal);
    double angle(const Vec &a, const Vec &b);
    double getBendAngle(Vec &a, Vec &b);
    void rotateSegment(Vec &seg, double angle, const Vec &axis, const Vec &axisToMove);
    Vec vectorQuaternionRotation(double angle, const Vec &axis, const Vec &vectorToRotate);
    void recalculateNormal(unsigned int index, const Vec &origin, const Vec &newPoint);
    void recalculateBinormal(unsigned int index, const Vec &origin, const Vec &newPoint);
    void initialiseFrame(Frame &f);
    Quaternion getRotationQuaternion(const Vec& axis, const double &theta);
    void getCuttingAngles(std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals);
    Vec getCuttingBinormal(unsigned int index);

    ManipulatedFrame frame;
    const Vec tangent = Vec(1,0,0);
    const Vec normal = Vec(0,0,1);
    const Vec binormal = Vec(0,1,0);
    std::vector<Vec> points;
    std::vector<Vec> segmentNormals;
    std::vector<Vec> segmentBinormals;
    std::vector<Vec> cuttingLines;
    std::vector<Vec> cuttingBinormals;
    std::vector<Vec> displayNormals;
    std::vector<Frame> boxes;
};

#endif // POLYLINE_H

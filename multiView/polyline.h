#ifndef POLYLINE_H
#define POLYLINE_H

#include "curvepoint.h"

class Polyline
{
public:
    Polyline();

    void draw();
    void init(const Frame *const refFrame, unsigned int nbPoints);
    void reinit(unsigned int nbPoints);
    void update(const std::vector<Vec> &newPoints);
    unsigned int getNbPoints(){ return points.size(); }
    Vec& getPoint(unsigned int i){return points[i];}
    Vec getWorldCoordinates(const Vec& v){ return frame.inverseCoordinatesOf(v);}
    Vec getWorldTransform(const Vec& v){ return frame.inverseTransformOf(v);}
    Vec getLocalTransform(const Vec& v){ return frame.localTransformOf(v); }
    void bend(unsigned int index, Vec &newPosition, std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals);
    void getDistances(std::vector<double>& distances);
    double euclideanDistance(const Vec &a, const Vec &b);

private:
    Vec projection(Vec &a, Vec &planeNormal);
    double angle(const Vec &a, const Vec &b);
    double getBendAngle(Vec &a, Vec &b);
    Vec vectorQuaternionRotation(double angle, const Vec &axis, const Vec &vectorToRotate);
    void recalculateNormal(unsigned int index, const Vec &origin, const Vec &newPoint);
    void recalculateBinormal(unsigned int index, const Vec &origin, const Vec &newPoint);
    void initialiseFrame(Frame &f);
    Vec averageVector(const std::vector<Vec>& v);
    void getCuttingAngles(std::vector<Vec>& relativeNorms, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals);

    ManipulatedFrame frame;
    const Vec tangent = Vec(1,0,0);
    const Vec normal = Vec(0,0,1);
    const Vec binormal = Vec(0,1,0);
    std::vector<Vec> points;
    std::vector<Vec> segmentNormals;
    std::vector<Vec> segmentBinormals;
    std::vector<Vec> segmentTangents;
    std::vector<Vec> cuttingLines;
    std::vector<Vec> cuttingBinormals;
};

#endif // POLYLINE_H

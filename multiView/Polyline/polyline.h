#ifndef POLYLINE_H
#define POLYLINE_H

#include "Planes/curvepoint.h"
#include "box.h"
#include "Planes/plane.h"

class Polyline
{
public:
    Polyline();

    void draw();
    void drawBox(unsigned int index);
    void init(const Frame *const refFrame, unsigned int nbPoints);
    void reinit(unsigned int nbPoints);
    void updatePoints(const std::vector<Vec> &newPoints);
     void reinitAngles(unsigned int nbPoints);
    unsigned int getNbPoints(){ return points.size(); }

    Vec& getPoint(unsigned int i){return points[i];}
    Vec getMeshPoint(unsigned int i){ return getWorldCoordinates(points[i]); }
    Vec getMeshBoxPoint(unsigned int i){ return getWorldCoordinates(boxes[i].getLocation()); }
    Vec getMeshBoxTransform(unsigned int i, Vec v){ return getWorldTransform(boxes[i].worldTransform(v)); }
    Vec getMeshBoxMiddle(unsigned int i){ return getWorldCoordinates(boxes[i].getMidPoint()); }
    Vec getMeshBoxEnd(unsigned int i){ return getWorldCoordinates(boxes[i].getEnd()); }

    void setBoxLocation(unsigned int i, const Vec& v){ boxes[i].setPosition(v); }

    const double& getBoxHeight(unsigned int i) { return boxes[i].getHeight(); }
    const double& getBoxWidth(unsigned int i){ return boxes[i].getWidth(); }

    Vec getWorldCoordinates(const Vec& v){ return frame.localInverseCoordinatesOf(v);}
    Vec getLocalCoordinates(const Vec& v){ return frame.localCoordinatesOf(v);}
    Vec getWorldTransform(const Vec& v){ return frame.localInverseTransformOf(v);}
    Vec getLocalTransform(const Vec& v){ return frame.localTransformOf(v); }

    Vec getLocalBoxCoordinates(unsigned int i, const Vec &v){ return boxes[i].localCoordinates( getLocalCoordinates(v) ); }
    Vec getLocalBoxTransform(unsigned int i, const Vec &v){ return boxes[i].localTransform( getLocalTransform(v) ); }
    Vec getWorldBoxCoordinates(unsigned int i, const Vec &v){ return getWorldCoordinates(boxes[i].worldCoordinates(v)); }
    Vec getWorldBoxTransform(unsigned int i, const Vec &v){ return getWorldTransform(boxes[i].worldTransform(v)); }

    void bend(unsigned int index, Vec &newPosition, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals);
    void bendFibula(unsigned int index, Vec &newPosition);
    void getDistances(std::vector<double>& distances);
    void lowerPoint(unsigned int index, const Vec &toLower);
    const std::vector<Vec>& getPoints(){ return points; }
    void lowerPolyline(Vec localDirection, double distance);

    const Vec& getNormal(){ return normal; }
    const Vec& getBinormal(){ return binormal; }
    const Vec& getTangent(){ return tangent; }
    void setFirstPoint(const Vec& firstPoint){ frame.setPosition(firstPoint); }

    void rotate(Quaternion q){ frame.rotate(q); }
    void rotateOnAxis(double angle, const Vec& point){ Quaternion q(tangent, angle); frame.rotateAroundPoint(q, point); }
    void getRelatvieNormals(std::vector<Vec>& relativeNorms);
    void setAlpha(float a){ boxTransparency = a; }
    void resetBoxes();
    void rotateBox(unsigned int i, double angle);
    void getRelativePlane(Plane& p, std::vector<Vec>& norms);
    void recalculateOrientations();
    void restoreBoxRotations();
    void getDirections(std::vector<Vec>& directions);

    void toggleIsWireframe(){ isWireframe = !isWireframe; }

private:
    Vec projection(Vec &a, Vec &planeNormal);
    double angle(const Vec &a, const Vec &b);
    Vec vectorQuaternionRotation(double angle, const Vec &axis, const Vec &vectorToRotate);
    void recalculateBinormal(unsigned int index, const Vec &origin, const Vec &newPoint);
    void initialiseFrame(Frame &f);
    void getCuttingAngles(std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals);
    double euclideanDistance(const Vec &a, const Vec &b);
    void resetBox(unsigned int index);

    Frame frame;
    const Vec tangent = Vec(1,0,0);
    const Vec normal = Vec(0,0,1);
    const Vec binormal = Vec(0,1,0);
    std::vector<Vec> points;
    std::vector<Vec> segmentNormals;
    std::vector<Vec> segmentBinormals;
    std::vector<Vec> cuttingLines;
    std::vector<Vec> cuttingBinormals;
    float boxTransparency = 1.f;
    std::vector<Box> boxes;
    bool isWireframe = true;
};

#endif // POLYLINE_H

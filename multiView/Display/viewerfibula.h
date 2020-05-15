#ifndef VIEWERFIBULA_H
#define VIEWERFIBULA_H

#include "viewer.h"

class ViewerFibula : public Viewer
{
    Q_OBJECT

public:
    ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffsetMax);
    void initGhostPlanes(Movable s);
    void updateFibPolyline(const Vec& firstPoint, const std::vector<double>& distances);
    void constructCurve();
    void toggleIsPolyline();
    void repositionPlanesOnPolyline();

public Q_SLOTS:
    void bendPolylineNormals(std::vector<Vec>&, const std::vector<double>&);
    void bendPolyline(unsigned int pointIndex, Vec v);
    void constructPolyline(const std::vector<double>&, const std::vector<Vec>&);
    void updateDistances(const std::vector<double>&);
    void movePlanes(double);
    void updatePlaneOrientations(std::vector<Vec>&);
    void rotatePolylineOnAxisFibula(double);
    void cut();

Q_SIGNALS:
    void okToPlacePlanes(const std::vector<Vec>&);

private:
    void rotatePolyline();
    void setPlanesInPolyline(std::vector<Vec> &normals);
    void setPlaneOrientations(std::vector<Vec> &normals);
    void setDistances(const std::vector<double> &distances);
    void constructSegmentPoints(unsigned int nbU);
    void projectToMesh(const std::vector<double>& distances);
    void matchDistances(const std::vector<double>& distances, std::vector<unsigned int> &segIndexes, std::vector<Vec> &outputPoints, double epsilon, const unsigned int& searchRadius);
    double euclideanDistance(const Vec &a, const Vec &b);
    unsigned int getClosestDistance(unsigned int index, const double &targetDistance, std::vector<unsigned int> &segIndexes, std::vector<Vec> &outputPoints, unsigned int searchRadius);
    std::vector<Vec> planeNormals;
};

#endif // VIEWERFIBULA_H

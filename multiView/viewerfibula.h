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
    void initCurve();
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

Q_SIGNALS:
    void okToPlacePlanes(const std::vector<Vec>&);

private:
    void rotatePolyline();
    void setPlanesInPolyline(std::vector<Vec> &normals);
    void setPlaneOrientations(std::vector<Vec> &normals);
    std::vector<Vec> planeNormals;
};

#endif // VIEWERFIBULA_H

#ifndef VIEWERFIBULA_H
#define VIEWERFIBULA_H

#include "viewer.h"

class ViewerFibula : public Viewer
{
    Q_OBJECT

public:
    ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffsetMax);
    void initGhostPlanes(Movable s);
    void updateFibPolyline(const std::vector<double>& distances);
    void initCurve();
    void constructCurve();

public Q_SLOTS:
    void bendPolylineNormals(const std::vector<Vec>&, const std::vector<double>&);
    void bendPolyline(unsigned int pointIndex, Vec v);
    void constructPolyline(const std::vector<double>&, const std::vector<Vec>&);

Q_SIGNALS:
    void okToPlacePlanes(const std::vector<Vec>&);
};

#endif // VIEWERFIBULA_H

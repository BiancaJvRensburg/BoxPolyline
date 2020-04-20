#ifndef VIEWERFIBULA_H
#define VIEWERFIBULA_H

#include "viewer.h"

class ViewerFibula : public Viewer
{
    Q_OBJECT

public:
    ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffsetMax);
    void initGhostPlanes();
    void updateFibPolyline(const std::vector<double>& distances);

public Q_SLOTS:
    void bendPolylineNormals(const std::vector<Vec>&, const std::vector<double>&, const std::vector<Vec>&);
    void bendPolyline(unsigned int pointIndex, Vec v);
};

#endif // VIEWERFIBULA_H

#ifndef VIEWERFIBULA_H
#define VIEWERFIBULA_H

#include "viewer.h"

class ViewerFibula : public Viewer
{
    Q_OBJECT

public:
    ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffsetMax);

public Q_SLOTS:
    void updateFibPolyline(const std::vector<Vec>&);
};

#endif // VIEWERFIBULA_H

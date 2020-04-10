#ifndef VIEWER_H
#define VIEWER_H

#include <QGLViewer/qglviewer.h>
#include "standardcamera.h"
#include "plane.h"
#include "curve.h"
#include "polyline.h"

using namespace qglviewer;

class Viewer : public QGLViewer
{
    Q_OBJECT

public :
    Viewer(QWidget *parent, StandardCamera *camera, int sliderMax);

public Q_SLOTS:
    void toUpdate();
    void extendPolyline(int);
    void bendPolyline();

Q_SIGNALS:
    void polylineUpdate(const std::vector<Vec>&);

protected:
    void draw();
    void init();
    void updateCamera(const Vec& center, float radius);
    void updatePolyline(const std::vector<Vec> &newPoints);

    double angle(Vec a, Vec b);
    double segmentLength(const Vec a, const Vec b);

    ManipulatedFrame* viewerFrame;
    class Polyline poly;


private:
    int partition(int sorted[], int start, int end);
    void quicksort(int sorted[], int start, int end);
};

#endif // VIEWER_H

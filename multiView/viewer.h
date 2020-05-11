#ifndef VIEWER_H
#define VIEWER_H

#include <QGLViewer/qglviewer.h>
#include "standardcamera.h"
#include "plane.h"
#include "curve.h"
#include "polyline.h"
#include "mesh.h"

using namespace qglviewer;

class Viewer : public QGLViewer
{
    Q_OBJECT

public :
    Viewer(QWidget *parent, StandardCamera *camera, int sliderMax);
    void openOFF(QString f);
    void readJSON(const QJsonArray &json);

public Q_SLOTS:
    void toUpdate();
    virtual void bendPolyline(unsigned int pointIndex, Vec v);
    void cutMesh();
    void uncutMesh();
    void placePlanes(const std::vector<Vec>&);
    void moveLeftPlane(int);
    void moveRightPlane(int);
    void rotatePolylineOnAxis(int);
    void toggleDrawMesh();
    void toggleDrawPlane();
    void setPlaneAlpha(int);
    void setMeshAlpha(int);
    void setBoxAlpha(int);
    void cut();

Q_SIGNALS:
    void polylineUpdate(const std::vector<Vec>&);
    void polylineBent(std::vector<Vec>&, const std::vector<double>&);
    void constructPoly(const std::vector<double>&, const std::vector<Vec>&);
    void toUpdateDistances(std::vector<double>&);
    void toUpdatePlaneOrientations(std::vector<Vec>&);
    void toRotatePolylineOnAxis(double);
    void planeMoved(double);

protected:
    void draw();
    void init();
    virtual void constructCurve();
    virtual void initGhostPlanes(Movable s);
    void initPolyPlanes(Movable s);
    void initCurvePlanes(Movable s);
    void updateCamera(const Vec& center, float radius);
    void updatePolyline(const std::vector<Vec> &newPoints);
    void deleteGhostPlanes();
    void repositionPlane(Plane* p, unsigned int index);
    virtual void repositionPlanesOnPolyline();
    void matchPlaneToFrenet(Plane *p, unsigned int index);
    virtual void toggleIsPolyline();
    unsigned int partition(std::vector<unsigned int>& sorted, unsigned int start, unsigned int end);
    void quicksort(std::vector<unsigned int>& sorted, int start, int end);
    void findGhostLocations(unsigned int nbGhostPlanes, std::vector<unsigned int>& ghostLocations);
    void deconstructPolyline();
    void getPlaneBoxOrientations(std::vector<Vec>& norms);
    void simpleBend(const unsigned int &pointIndex, Vec v, std::vector<Vec>& planeNormals, std::vector<Vec>& planeBinormals);


    double angle(Vec a, Vec b);
    double segmentLength(const Vec a, const Vec b);

    ManipulatedFrame* viewerFrame;
    class Polyline poly;
    std::vector<Plane*> ghostPlanes;
    Plane* leftPlane;
    Plane* rightPlane;
    Plane* endLeft;
    Plane* endRight;

    std::vector<Vec> control;
    Curve curve;
    Mesh mesh;

    unsigned int curveIndexL;
    unsigned int curveIndexR;
    unsigned int nbU;
    bool isCurve;
    const double constraint = 25.;
    int sliderMax;
    bool isCut;
    bool isDrawMesh;

    int polyRotation;

private:
    void constructPolyline(const std::vector<Vec>& polyPoints);
    void movePlane(Plane *p, unsigned int curveIndex);
    void setPlaneOrientation(Plane& p, std::vector<Vec>& norms, std::vector<Vec>& binorms);
    void setPlaneOrientations(std::vector<Vec>& norms, std::vector<Vec>& binorms);
};

#endif // VIEWER_H

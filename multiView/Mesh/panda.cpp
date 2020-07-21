#include "panda.h"
#include "meshreader.h"

Panda::Panda()
{
    openOFF("effector.off", effector);
    openOFF("marker.off", marker);
    openOFF("Navex.off", navex);
    openOFF("WRJ.off", wrj);

   /* m = new SimpleManipulator();
    m->deactivate();
    m->setDisplayScale(50.);
    m->setID(0);
    m->setRotationActivated(false);*/
}

void Panda::draw(){
    glPushMatrix();
    glMultMatrixd(f.matrix());

    wrj.draw();
    marker.draw();
    navex.draw();
    effector.draw();

    //m->draw();

    QGLViewer::drawAxis(50.);

    glPopMatrix();
}

void Panda::openOFF(QString filename, SimpleMesh &mesh){
    std::vector<Vec3Df> &vertices = mesh.getVertices();
    std::vector<Triangle> &triangles = mesh.getTriangles();

    filename = QDir().relativeFilePath(filename);

    FileIO::openOFF(filename.toStdString(), vertices, triangles);

    mesh.update();
}

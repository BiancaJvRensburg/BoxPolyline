TEMPLATE = app
TARGET   = multiView

HEADERS  = \
    box.h \
    controlpoint.h \
    curvepoint.h \
    curve.h \
    mainwindow.h \
    mesh.h \
    meshreader.h \
    plane.h \
    point3.h \
    polyline.h \
    standardcamera.h \
    triangle.h \
    vec3D.h \
    viewer.h \
    viewerfibula.h
SOURCES  = main.cpp \
    box.cpp \
    controlpoint.cpp \
    curvepoint.cpp \
    curve.cpp \
    mainwindow.cpp \
    mesh.cpp \
    plane.cpp \
    polyline.cpp \
    standardcamera.cpp \
    viewer.cpp \
    viewerfibula.cpp

include( ../baseInclude.pri )
INCLUDEPATH += "..\eigen-3.3.7\Eigen"
INCLUDEPATH += "..\nanoflann\include"

TEMPLATE = app
TARGET   = multiView

HEADERS  = \
    Polyline/box.h \
    Curve/controlpoint.h \
    Planes/curvepoint.h \
    Curve/curve.h \
    Display/mainwindow.h \
    Mesh/mesh.h \
    Mesh/meshreader.h \
    Planes/plane.h \
    Tools/point3.h \
    Polyline/polyline.h \
    Tools/standardcamera.h \
    Tools/triangle.h \
    Tools/vec3D.h \
    Display/viewer.h \
    Display/viewerfibula.h
SOURCES  = main.cpp \
    Polyline/box.cpp \
    Curve/controlpoint.cpp \
    Planes/curvepoint.cpp \
    Curve/curve.cpp \
    Display/mainwindow.cpp \
    Mesh/mesh.cpp \
    Planes/plane.cpp \
    Polyline/polyline.cpp \
    Tools/standardcamera.cpp \
    Display/viewer.cpp \
    Display/viewerfibula.cpp

include( ../baseInclude.pri )
INCLUDEPATH += "..\eigen-3.3.7\Eigen"
INCLUDEPATH += "..\nanoflann\include"

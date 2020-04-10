#ifndef POLYLINE_H
#define POLYLINE_H

#include "curvepoint.h"

class Polyline
{
public:
    Polyline();

    void draw();
    void init(const Frame *const refFrame);

private:
    ManipulatedFrame frame;
    Vec tangent, normal;
    std::vector<Vec> points;

};

#endif // POLYLINE_H

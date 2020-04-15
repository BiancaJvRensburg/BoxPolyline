#ifndef CURVEPOINT_H
#define CURVEPOINT_H

#include "controlpoint.h"

class CurvePoint : public ControlPoint
{
    Q_OBJECT

public:
    CurvePoint(Vec& p, const unsigned int &id);
    //CurvePoint(CurvePoint &cp);
    /*~CurvePoint(){
        disconnect((ManipulatedFrame*)mf, &ManipulatedFrame::manipulated, this, &ControlPoint::cntrlMoved);
    }*/

    void setPosition(Vec& p){ this->p = p; mf.setPosition(getX(), getY(), getZ()); }

    void matchCurvepoint(CurvePoint &c);
    Quaternion getOrientation(){ return mf.orientation(); }

    void draw();

public Q_SLOTS:
    void cntrlMoved();

Q_SIGNALS:
    void curvePointTranslated(unsigned int pointIndex, Vec offset);

private:
    unsigned int id;

};

#endif // CURVEPOINT_H

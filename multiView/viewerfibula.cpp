#include "viewerfibula.h"

ViewerFibula::ViewerFibula(QWidget *parent, StandardCamera *camera, int sliderMax, int fibulaOffset) : Viewer (parent, camera, sliderMax)
{

}

void ViewerFibula::updateFibPolyline(const std::vector<Vec>& newPoints){
    updatePolyline(newPoints);
}

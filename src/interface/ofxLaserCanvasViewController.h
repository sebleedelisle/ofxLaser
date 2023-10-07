//
//  ofxLaserCanvasViewController.h
//
//  Created by Seb Lee-Delisle on 11/02/2023.
//
//

#pragma once

#include "ofxLaserViewWithMoveables.h"
#include "ofxLaserUI.h"
#include "ofxLaserShapeTargetCanvas.h"

namespace ofxLaser {

class CanvasInputZoneQuad : public MoveablePoly  {
    
    public :
    
    CanvasInputZoneQuad() {
        setHue(0);
        setBrightness(180);
        
    }
    
};

class CanvasViewController : public ViewWithMoveables {
    
    public :
    void drawImGui();
    bool updateZonesFromUI(ShapeTargetCanvas& canvasTarget);
    void updateUIFromZones(ShapeTargetCanvas& canvasTarget);
    
};
}

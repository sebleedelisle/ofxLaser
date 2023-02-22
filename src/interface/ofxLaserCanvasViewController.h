//
//  ofxLaserCanvasViewController.h
//
//  Created by Seb Lee-Delisle on 11/02/2023.
//
//

#pragma once

#include "ofxLaserViewWithMoveables.h"
#include "ofxLaserInputZone.h"

namespace ofxLaser {

class CanvasViewController : public ViewWithMoveables {
    
    public :
    void drawImGui();
    bool updateZonesFromUI(vector<InputZone*>& zones);
    void updateUIFromZones(const vector<InputZone*>& zones);
    
    
    
};
}

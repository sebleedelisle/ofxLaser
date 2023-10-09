//
//  ofxLaserZoneUiLine.h
//
//
//  Created by Seb Lee-Delisle on 06/02/2023.
//
//

#pragma once

#include "ofxLaserZoneUiBase.h"
#include "ofxLaserZoneTransformLineData.h"

namespace ofxLaser {
class ZoneUiLine : public ZoneUiBase {
    
    public :
    ZoneUiLine();
    
    bool updateDataFromUi(OutputZone* outputZone) override;
    bool updateFromData(OutputZone* outputZone) override;
    
    bool mousePressed(ofMouseEventArgs &e) override;
    
    void draw() override;
    
    void updatePoly() override;
    bool autoSmooth = true; 
    
   //vector<DragHandle*> anchorPoints; // bezier anchor points
   // vector<DragHandle*> controlPoints; // bezier control points
   
};
}

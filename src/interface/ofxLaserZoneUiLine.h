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
    
    void draw() override;
    
    void updatePoly() override;
    
    
};
}

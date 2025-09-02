//
//  ofxLaserZoneUiQuad.h
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#pragma once

#include "ofxLaserZoneUiBase.h"
#include "ofxLaserZoneTransformQuadData.h"

namespace ofxLaser {
class ZoneUiQuad : public ZoneUiBase {
    
    public :
    ZoneUiQuad(); 
    
    bool updateDataFromUi(std::shared_ptr<OutputZone>& outputZone) override;
    bool updateFromData(std::shared_ptr<OutputZone>& outputZone) override;
    
    bool setCorners(const vector<glm::vec2*>& points);
    
};
}

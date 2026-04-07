//
//  ofxLaserZoneUIQuad.h
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#pragma once

#include "ofxLaserZoneUIBase.h"
#include "ofxLaserZoneTransformQuadData.h"

namespace ofxLaser {
class ZoneUIQuad : public ZoneUIBase {
    
    public :
    ZoneUIQuad(); 
    
    bool updateDataFromUI(std::shared_ptr<OutputZone>& outputZone) override;
    bool updateFromData(std::shared_ptr<OutputZone>& outputZone) override;

};
}

//
//  ofxLaserZoneUiLine.h
//  example_HelloLaser
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
    
    bool updateDataFromUI(ZoneTransformBase* zonetransform) override;
    bool updateFromData(ZoneTransformBase* zonetransform) override;
    
    void draw() override;
    
    
    //bool setCorners(const vector<glm::vec2*>& points);
    
    
};
}

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
class ZoneUiQuad : public ZoneUiBase {
    
    public :
    ZoneUiQuad(); 
    
    
    bool updateDataFromUI(ZoneTransformBase* zonetransform) override;
    bool updateFromData(ZoneTransformBase* zonetransform) override;
    
    bool setCorners(const vector<glm::vec2*>& points);
    //vector<DragHandle*> getCornersClockwise();
    bool isSquare();
    
  
    bool constrainedToSquare = false;

    
};
}

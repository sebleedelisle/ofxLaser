//
//  ofxLaserZoneUIBase.h
//
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserMoveablePoly.h"
#include "ofxLaserOutputZone.h"

namespace ofxLaser {

class ZoneUIBase : public MoveablePoly {
  
    public :
    
    ZoneUIBase() = default;
    virtual ~ZoneUIBase() = default; 
    
    
    virtual void drawLabel() override;
    

    virtual bool updateDataFromUI(std::shared_ptr<OutputZone>& outputZone);
    virtual bool updateFromData(std::shared_ptr<OutputZone>& outputZone);
    void updateLabel();
    bool setCorners(const vector<glm::vec2*>& points);

    ZoneId zoneId;
    bool muted = false;
    bool locked = false;

    protected :

    
   
};
}

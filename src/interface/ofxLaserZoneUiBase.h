//
//  ofxLaserZoneUiBase.h
//
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserMoveablePoly.h"
//#include "ofxLaserZoneTransformBase.h"
#include "ofxLaserOutputZone.h"
#include "ofxLaserUI.h"

class ZoneTransformBase;

namespace ofxLaser {

class ZoneUiBase : public MoveablePoly {
  
    public :
    
    ZoneUiBase() = default;
    virtual ~ZoneUiBase() = default; 
    
    
    virtual void drawLabel() override;
    

    virtual bool updateDataFromUi(std::shared_ptr<OutputZone>& outputZone);
    virtual bool updateFromData(std::shared_ptr<OutputZone>& outputZone);
    void updateLabel(); 

    ZoneId zoneId;
    bool inputZoneAlt = false;
    bool muted = false;
    bool locked = false;

    protected :

    
   
};
}

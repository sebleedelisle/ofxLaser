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

class ZoneTransformBase;

namespace ofxLaser {

class ZoneUiBase : public MoveablePoly {
  
    public :
    
    ZoneUiBase() = default;
    virtual ~ZoneUiBase() = default; 

    virtual bool updateDataFromUi(OutputZone* outputZone);
    virtual bool updateFromData(OutputZone* outputZone);
    void updateLabel(); 

   

    int inputZoneIndex = -1;
    bool inputZoneAlt = false;
    bool muted = false; 

    protected :

    
   
};
}

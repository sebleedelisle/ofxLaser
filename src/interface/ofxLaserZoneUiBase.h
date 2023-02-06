//
//  ofxLaserZoneUiBase.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserMoveablePoly.h"
#include "ofxLaserZoneTransformBase.h"
class ZoneTransformBase;

namespace ofxLaser {

class ZoneUiBase : public MoveablePoly {
  
    public :
    
    ZoneUiBase();
    virtual ~ZoneUiBase() = default; 

    string getLabel(); 
    
    virtual bool updateDataFromUI(ZoneTransformBase* zonetransform) = 0;
    virtual bool updateFromData(ZoneTransformBase* zonetransform) = 0;

    bool showContextMenu = false; // bit of a hack to show the right click menu

    int inputZoneIndex = -1;
    bool inputZoneAlt = false;

    protected :

   // bool locked;
    
   
};
}

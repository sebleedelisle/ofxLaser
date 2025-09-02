//
//  ofxLaserZoneContent.h
//
//  Created by Seb Lee-Delisle on 09/02/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserZoneId.h"

class Shape; 

namespace ofxLaser {

class ZoneContent {
    public :
   
    ZoneId zoneId;
    vector<std::shared_ptr<Shape>> shapes;
    
    ofRectangle sourceRectangle;


};

}

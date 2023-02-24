//
//  ofxLaserZoneContent.h
//
//  Created by Seb Lee-Delisle on 09/02/2023.
//
//

#pragma once

#include "ofMain.h"

class Shape; 

namespace ofxLaser {

class ZoneContent {
    public :
    
    enum ZoneType {
        BEAM,
        CANVAS
    };
    
    int zoneIndex;
    ZoneType zoneType;
    string label;
    vector<Shape*> shapes;
    
    ofRectangle sourceRectangle;


};

}

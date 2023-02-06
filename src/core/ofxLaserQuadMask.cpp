//
//  ofxLaserQuadMask.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 16/04/2021.
//


#include "ofxLaserQuadMask.h"

using namespace ofxLaser;

QuadMask::QuadMask() {
    maskLevel.set("Reduction amount", 100, 0,100);
    //setColours(ofColor::red,ofColor::red,ofColor::red);
    maskLevel.addListener(this, &QuadMask::maskLevelChanged);
    
    
};

QuadMask::~QuadMask() {
    maskLevel.removeListener(this, &QuadMask::maskLevelChanged);
}



void QuadMask::maskLevelChanged(int&e) {
    isDirty = true;
}


void QuadMask::serialize(ofJson&json) const {
    
    
    PolygonBase::serialize(json);
    json["masklevel"] = (int)maskLevel;
    
    
    
  
}

bool QuadMask::deserialize(ofJson& jsonGroup) {
    
    if(PolygonBase::deserialize(jsonGroup)) {
        
        maskLevel = jsonGroup["masklevel"];
        return true;
    } else {
        return false;
    }
    
}

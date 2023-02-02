//
//  ofxLaserQuadMask.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 16/04/2021.
//


#include "ofxLaserQuadMask.h"

using namespace ofxLaser;

QuadMask::QuadMask() : QuadGui() {
    maskLevel.set("Reduction amount", 100, 0,100);
    //setColours(ofColor::red,ofColor::red,ofColor::red);
    maskLevel.addListener(this, &QuadMask::maskLevelChanged);
    
    
};

QuadMask::~QuadMask() {
    maskLevel.removeListener(this, &QuadMask::maskLevelChanged);
}

void QuadMask::serialize(ofJson&json) const {
    ofJson& pointsjson = json["maskpoints"];
    for(int i = 0; i<points.size(); i++) {
        const glm::vec2& pos = points[i];
        pointsjson.push_back({pos.x, pos.y});
    }
    json["masklevel"] = (int)maskLevel;
    
    
    
  
}

bool QuadMask::deserialize(ofJson& jsonGroup) {
    
    ofJson& pointsjson = jsonGroup["maskpoints"];
    if(pointsjson.size()>=3) {
        
        maskLevel = jsonGroup["masklevel"];
        
        points.resize(pointsjson.size());
    
        for(int i = 0; i<points.size(); i++) {
            ofJson& point = pointsjson[i];
      
            points[i].x = point[0];
            points[i].y = point[1];
            
        }
        isDirty = true;
        return true;
    } else {
        return false;
    }

    
}


void QuadMask::maskLevelChanged(int&e) {
    isDirty = true;
}

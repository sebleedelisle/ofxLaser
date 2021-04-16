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
    
};
void QuadMask::serialize(ofJson&json) {
    QuadGui::serialize(json);
    json["masklevel"] = (int)maskLevel;
   // cout << json.dump(3) << endl;
}

bool QuadMask::deserialize(ofJson& jsonGroup) {
    
    QuadGui::deserialize(jsonGroup);

    maskLevel = jsonGroup["masklevel"];
    
    return true;
}

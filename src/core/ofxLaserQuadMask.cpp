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
    setColours(ofColor::red,ofColor::red,ofColor::red);
    maskLevel.addListener(this, &QuadMask::maskLevelChanged);
    
    
};

QuadMask::~QuadMask() {
    
    maskLevel.removeListener(this, &QuadMask::maskLevelChanged);
    
    
}
void QuadMask::serialize(ofJson&json) {
    QuadGui::serialize(json);
    json["masklevel"] = (int)maskLevel;
  
}

bool QuadMask::deserialize(ofJson& jsonGroup) {
    
    QuadGui::deserialize(jsonGroup);

    maskLevel = jsonGroup["masklevel"];
    
    return true;
}

void QuadMask:: draw() {
    
    ofPushStyle();
    
    ofPushMatrix();
    ofTranslate(offset);
    ofScale(scale, scale);
    
    ofFill();
    
    ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    
    ofSetColor(ofMap(maskLevel,0,100,255,0));
    
   
    ofBeginShape();
    ofVertex(handles[0]);
    ofVertex(handles[1]);
    ofVertex(handles[3]);
    ofVertex(handles[2]);
    ofEndShape();
    

   
    ofPopMatrix();
    ofPopStyle();
    
    QuadGui::draw();
    
}

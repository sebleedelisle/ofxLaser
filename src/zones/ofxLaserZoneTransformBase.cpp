//
//  ofxLaserZoneTransformQuad.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransformBase.h"


using namespace ofxLaser;



ZoneTransformBase:: ZoneTransformBase(){
    
    isDirty = true;
   
    transformParams.setName("ZoneTransformBaseParams");
    
    transformParams.add(locked.set("locked", false));
    srcRect.set(0,0,400,400); 
 
    
    
}
ZoneTransformBase:: ~ZoneTransformBase(){
    
    
}



void ZoneTransformBase:: updateSrc(const ofRectangle& rect) {
    
    if(srcRect != rect) {
        //ofLogNotice("ZoneTransformBase:: setSrc ") << rect;
        srcRect = rect;
        isDirty = true;
    }
};





void ZoneTransformBase :: setDirty(bool state) {
    isDirty = state;
    
};

vector<glm::vec2> ZoneTransformBase::getPerimeterPoints() {
    vector<glm::vec2> points;
    getPerimeterPoints(points);
    return points;
}

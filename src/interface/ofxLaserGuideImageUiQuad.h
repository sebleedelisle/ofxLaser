//
//  ofxLaserGuideImageUiQuad.h
//
//
//  Created by Seb Lee-Delisle on 07/02/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserQuadMask.h"

namespace ofxLaser {

class GuideImageUiQuad : public MoveablePoly {
    
    public :
    
    GuideImageUiQuad(string uid) : MoveablePoly(uid) {
        
        setHue(180);
        setBrightness(180);
        fillColourSelected = fillColour = ofColor(0,0,0,0); 
    }
    virtual void draw() override {
        ofPushStyle();
        ofSetColor(colour);
        image.draw(getBoundingBox());
        MoveablePoly::draw();
        ofPopStyle();
    }

    ofImage image;
    ofColor colour;
    
    
};
}


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
        
        fillColour  = ofColor::fromHex(0x001123, 0);
        //fillColourSelected = ofColor::fromHex(0x001123);
        strokeColour  = ofColor::fromHex(0x0E87E7,0);
        //strokeColourSelected = ofColor::fromHex(0x0E87E7);
        handleColour = ofColor::fromHex(0x0E87E7);
        handleColourOver = ofColor :: fromHex(0xffffff);
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


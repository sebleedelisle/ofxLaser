//
//  ofxLaserQuadMask.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 01/02/2018.
//

#pragma once 
#include "ofxLaserQuadGui.h"



namespace ofxLaser {
class QuadMask : public QuadGui{
    
    public :
    QuadMask() : QuadGui() {
        maskLevel.set("Reduction amount", 100, 0,100);
    };


    ofParameter<int> maskLevel;

    
};
}

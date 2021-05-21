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
    QuadMask();
    ~QuadMask(); 
    virtual void serialize(ofJson&json) override;
    virtual bool deserialize(ofJson&jsonGroup) override;
    virtual void draw() override;

    void maskLevelChanged(int&e) {
        isDirty = true; 
    }
    
    ofParameter<int> maskLevel;

    
};
}

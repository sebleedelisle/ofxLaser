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
    void serialize(ofJson&json) const;
    bool deserialize(ofJson&jsonGroup);


    void maskLevelChanged(int&e);
    ofParameter<int> maskLevel;
    
    vector<glm::vec2> points;

    
};
}

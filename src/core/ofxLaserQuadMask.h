//
//  ofxLaserQuadMask.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 01/02/2018.
//

#pragma once

#include "ofxLaserPolygonBase.h"

namespace ofxLaser {
class QuadMask : public PolygonBase {
    
    public :
    QuadMask();
    ~QuadMask();
    
    virtual void serialize(ofJson&json) const override;
    virtual bool deserialize(ofJson&jsonGroup) override;
    
    void maskLevelChanged(int&e);
    ofParameter<int> maskLevel;
    
    
};
}

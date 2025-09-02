//
//  ofxLaserObjectWithZoneId.h
//  example_Beams
//
//  Created by Seb Lee-Delisle on 03/03/2023.
//
//

#pragma once
#include "ofxLaserZoneId.h"

namespace ofxLaser {

class ObjectWithZoneId {
    public :
    virtual ~ObjectWithZoneId() = default;
    
    virtual void serialize(ofJson& json) const;
    virtual bool deserialize(ofJson& json);
    
    ZoneId zoneId;
    
    
};
}

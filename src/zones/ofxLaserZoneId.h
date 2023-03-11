//
//  ofxLaserZoneId.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 25/02/2023.
//
//

#pragma once

#include "ofMain.h"

namespace ofxLaser {

class ZoneId {
    public :
    ZoneId() ;
    
    enum ZoneType {
        BEAM,
        CANVAS
    };
    
    string getLabel() ;
    string getUid() const;
    
    virtual void serialize(ofJson& json) const;
    virtual bool deserialize(ofJson& json);
 
    bool operator==(const ZoneId & other) const;
    bool operator!=(const ZoneId & other) const;
    
    inline bool operator< (const ZoneId& rhs) const {
        return true;
    }
    
    ZoneType type;
    int zoneGroup;
    int zoneIndex;
    
    string label;
    
    
};




}


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
    
    string getLabel() const;
    string getDefaultLabel() const ;
    bool setLabel(string newlabel);
    const string& getUid() const;
    ZoneType getType(); 
    void updateUid();
    
    bool set(ZoneType zonetype, int group, int num); 
    
    virtual void serialize(ofJson& json) const;
    virtual bool deserialize(ofJson& json);
 
    bool operator==(const ZoneId & other) const;
    bool operator!=(const ZoneId & other) const;
    
    inline bool operator< (const ZoneId& rhs) const {
        if(zoneGroup!=rhs.zoneGroup) return zoneGroup<rhs.zoneGroup;
        else return zoneIndex<rhs.zoneIndex;
        // TODO add zone type checking
        //return true;
    }
    
    protected :
    
    ZoneType type;
    int zoneGroup;
    int zoneIndex;
    
    ZoneType cachedType;
    int cachedZoneGroup;
    int cachedZoneIndex;
    string cachedUid = "" ;
    
    string label;
    
    
};




}


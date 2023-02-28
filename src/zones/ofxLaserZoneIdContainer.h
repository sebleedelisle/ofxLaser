//
//  ZoneIdContainer.h
//
//  Created by Seb Lee-Delisle on 27/02/2023.
//
//

#pragma once
#include "ofxLaserZoneId.h"



namespace ofxLaser {

class ObjectWithZoneId {
    public :
    virtual ~ObjectWithZoneId() = default; 
    ZoneId zoneId;
    
    
};

class ZoneIdContainer {
    
    public :
    
    bool addZoneIdObject(ObjectWithZoneId* newZoneIdObject);
    bool clearZones(); 
    vector<ObjectWithZoneId*>& getZoneIds(); 
    map<ZoneId, ZoneId> removeZoneById();
    map<ZoneId, ZoneId> renumberZones();
    ObjectWithZoneId* getObjectForZoneId(ZoneId& zoneid);
    ObjectWithZoneId* getObjectAtIndex(int index);
    int getNumZoneIds() const; 

    protected :
    
    vector<ObjectWithZoneId*> zoneIdObjects;
    int zoneGroup;
    ZoneId :: ZoneType type;
    string groupLabel;
    
} ;
}



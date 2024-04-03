//
//  ZoneIdContainer.h
//
//  Created by Seb Lee-Delisle on 27/02/2023.
//
//

#pragma once
#include "ofxLaserObjectWithZoneId.h"


namespace ofxLaser {

class ZoneIdContainer {
    
    public :
    
    bool addZoneIdObject(ObjectWithZoneId* newZoneIdObject);
    // note that you need to override this function if your
    // array of objects is not a base ObjectWithZoneId
    virtual bool addZoneByJson(ofJson& json);
    bool clearZones(); 
    vector<ObjectWithZoneId*>& getZoneIds(); 
    map<ZoneId, ZoneId> removeZoneById(ZoneId& zoneId);
    map<ZoneId, ZoneId> moveZoneByIdToIndex(ZoneId& zoneId, int index);
    map<ZoneId, ZoneId> renumberZones();
    ObjectWithZoneId* getObjectForZoneId(ZoneId& zoneid);
    ObjectWithZoneId* getObjectForZoneIdUid(string& uid);
    ObjectWithZoneId* getObjectAtIndex(int index);
    int getNumZoneIds() const; 

    virtual void serialize(ofJson& json);
    virtual bool deserialize(ofJson& json);
    
    protected :
    
    vector<ObjectWithZoneId*> zoneIdObjects;
    int zoneGroup;
    ZoneId :: ZoneType type;
    string groupLabel;
    
} ;
}



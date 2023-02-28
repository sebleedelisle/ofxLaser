//
//  ZoneIdContainer.cpp
//  example_Beams
//
//  Created by Seb Lee-Delisle on 27/02/2023.
//
//

#include "ofxLaserZoneIdContainer.h"

using namespace ofxLaser;

bool ZoneIdContainer :: addZoneIdObject(ObjectWithZoneId* newZoneIdObject) {
    

    if(find(zoneIdObjects.begin(), zoneIdObjects.end(), newZoneIdObject)!=zoneIdObjects.end()) {
        return false;
    } else {
        zoneIdObjects.push_back(newZoneIdObject);
        renumberZones();
        return true;
    }
    
    
}


map<ZoneId, ZoneId> ZoneIdContainer :: renumberZones() {
    
    map<ZoneId, ZoneId> zonesChanged;
    
    for(int i = 0; i<zoneIdObjects.size(); i++) {
    
        ZoneId& zoneId = zoneIdObjects[i]->zoneId;
        ZoneId originalZoneId = zoneId; // makes copy
        zoneId.zoneIndex = i;
        zoneId.zoneGroup = zoneGroup;
        zoneId.type = type; // shouldn't change but hopefully OK
        
        if(zoneId!=originalZoneId) {
            //zonesChanged[originalZoneId] = zoneId;
        }
        
        
    }
    return zonesChanged;
    
    
    
}

bool ZoneIdContainer :: clearZones() {
    if(zoneIdObjects.size()==0) return false;
    else {
        for(ObjectWithZoneId* zoneidobj : zoneIdObjects) {
            delete zoneidobj;
        }
        zoneIdObjects.clear();
        return true;
    }
}

vector<ObjectWithZoneId*>& ZoneIdContainer :: getZoneIds(){
    return zoneIdObjects;
    
    
}

ObjectWithZoneId* ZoneIdContainer :: getObjectForZoneId(ZoneId& zoneid) {
    for(ObjectWithZoneId* zoneidobj : zoneIdObjects) {
        if(zoneidobj->zoneId == zoneid) return zoneidobj;
    }
    return nullptr;
    
}

ObjectWithZoneId* ZoneIdContainer :: getObjectAtIndex(int index) {
    
    if((index>=0) && (index<zoneIdObjects.size())) return zoneIdObjects[index];
    else return nullptr;
    
}

int ZoneIdContainer ::getNumZoneIds() const{
    return zoneIdObjects.size();
}

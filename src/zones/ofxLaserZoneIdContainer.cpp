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

bool ZoneIdContainer :: addZoneByJson(ofJson& json) {
    
    ObjectWithZoneId* zoneIdObject = new ObjectWithZoneId();
    if(zoneIdObject->deserialize(json)) {
        addZoneIdObject(zoneIdObject);
        return true;
    } else {
        delete zoneIdObject;
        return false;
    }
    
}

map<ZoneId, ZoneId> ZoneIdContainer :: removeZoneById(ZoneId& zoneId) {

    ObjectWithZoneId* zoneIdObject = getObjectForZoneId(zoneId);
    vector<ObjectWithZoneId*> :: iterator it = find(zoneIdObjects.begin(), zoneIdObjects.end(), zoneIdObject);
    if(it!=zoneIdObjects.end()) {
        zoneIdObjects.erase(it);
        delete zoneIdObject;
    }
    return renumberZones();
    
    
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
            zonesChanged.insert({originalZoneId,zoneId});
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

void ZoneIdContainer :: serialize(ofJson& json) const {
    
    ofJson& containerJson = json["zoneidcontainer"];
    containerJson["grouplabel"] = groupLabel;
    containerJson["zonetype"] = (int) type;
    containerJson["zonegroup"] = zoneGroup;
    
    ofJson& jsonarray = containerJson["zoneidobjects"];
    for(ObjectWithZoneId* zoneIdObject : zoneIdObjects) {
        ofJson zoneIdJson;
        zoneIdObject->serialize(zoneIdJson);
        jsonarray.push_back(zoneIdJson);
    }
    
}
bool ZoneIdContainer :: deserialize(ofJson& json) {
    
    if(json.contains("zoneidcontainer")) {

        ofJson& containerJson = json["zoneidcontainer"];

        if(containerJson.contains("grouplabel") && containerJson.contains("zonetype") && containerJson.contains("zonegroup") && containerJson.contains("zoneidobjects")) {

            clearZones();
            
            type = containerJson["zonetype"];
            zoneGroup = containerJson["zonegroup"];
            groupLabel = containerJson["grouplabel"];
            
            bool success = true;
            
            ofJson& jsonarray = containerJson["zoneidobjects"];
           // cout << jsonarray.dump(3) << endl;
            int numzones = jsonarray.size();
            for(int i = 0; i<numzones; i++) {

                ofJson& zoneIdJson = jsonarray[i];
                if(!addZoneByJson(zoneIdJson)) {
                    success = false;
                }
            }
            

            return success;

        } else {
            return false;
        }


    } else {
        return false;
    }
    
    
}

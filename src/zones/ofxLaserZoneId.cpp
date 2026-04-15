//
//  ofxLaserZoneId.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 25/02/2023.
//
//

#include "ofxLaserZoneId.h"

using namespace ofxLaser; 

ZoneId :: ZoneId(){
    type = CANVAS;
    zoneGroup = 0;
    zoneIndex = 0;
    label = "";
    cachedUid = "";
}

bool ZoneId :: set(ZoneType zonetype, int group, int index) {
    if((zonetype!=type) || (group!=zoneGroup) || (index!=zoneIndex)) {
        type = zonetype;
        zoneGroup = group;
        zoneIndex= index;
        updateUid();
        return true;
    } else {
        return false;
    }
    
}

string ZoneId :: getLabel() const {
    return getDefaultLabel();
}
string ZoneId :: getDefaultLabel() const {

    string newlabel = "ZONE ";
    newlabel = newlabel +ofToString(zoneIndex+1);
    return newlabel;

}


bool ZoneId :: setLabel(string newlabel) {
    if (!label.empty()) {
        label.clear();
        return true;
    }
    return false;
}

//const string& getUid() const;
const string& ZoneId :: getUid() const {
    // caching id for optimisation
    return cachedUid;
}

ofxLaser::ZoneId::ZoneType ZoneId :: getType() {
    return type;
}

void ZoneId :: updateUid() {

    string uid = "Z";
    uid = uid+ofToString(zoneGroup)+"_"+ofToString(zoneIndex);

    cachedUid = uid;

}
void ZoneId :: serialize(ofJson& json) const{
    ofJson& zoneIdJson = json["zoneId"];
    zoneIdJson["type"] = (int)type;
    zoneIdJson["zonegroup"] = zoneGroup;
    zoneIdJson["zoneindex"] = zoneIndex;
    zoneIdJson["label"] = "";
    //return true;
}
bool ZoneId :: deserialize(ofJson& json) {
    if(json.contains("zoneId")) {
        
        ofJson& zoneIdJson = json["zoneId"];
        
        if(zoneIdJson.contains("type") && zoneIdJson.contains("zonegroup") && zoneIdJson.contains("zoneindex") && zoneIdJson.contains("label")) {
            
            type = zoneIdJson["type"];
            zoneGroup = zoneIdJson["zonegroup"];
            zoneIndex = zoneIdJson["zoneindex"];
            label.clear();
            updateUid();
            return true;
            
        } else {
            return false;
        }
    
        
    } else {
        return false;
    }
    
    
}
bool ZoneId::operator==(const ZoneId & other) const{
    return other.getUid()==getUid();
}

//--------------------------------------------------------------
bool ZoneId::operator!=(const ZoneId & other) const{
    return !(other==*this);
}

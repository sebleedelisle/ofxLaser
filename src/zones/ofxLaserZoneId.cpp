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
    type = BEAM;
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
    if(label=="") {
        return getDefaultLabel();
    } else {
        return label;
    }
    
}
string ZoneId :: getDefaultLabel() const {
    
    string newlabel = "";
    
    if(type==BEAM) newlabel = "BEAM ";
    else newlabel = "CANVAS ";
    newlabel = newlabel +ofToString(zoneIndex+1);
    return newlabel;
            
   
    
}


bool ZoneId :: setLabel(string newlabel) {
        
    if(newlabel != getDefaultLabel()) {
        if(newlabel!=label) {
            label = newlabel;
            return true;
        } else {
            return false;
        }
    
    } else {
            
        label = "";
        return true;
    }
    
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
    
    string uid = "";
    if(type==BEAM) uid = "B";
    else uid = "C";
    uid = uid+ofToString(zoneGroup)+"_"+ofToString(zoneIndex);
    
    cachedUid = uid;
     
}
void ZoneId :: serialize(ofJson& json) const{
    ofJson& zoneIdJson = json["zoneId"];
    zoneIdJson["type"] = (int)type;
    zoneIdJson["zonegroup"] = zoneGroup;
    zoneIdJson["zoneindex"] = zoneIndex;
    zoneIdJson["label"] = label;
    //return true;
}
bool ZoneId :: deserialize(ofJson& json) {
    if(json.contains("zoneId")) {
        
        ofJson& zoneIdJson = json["zoneId"];
        
        if(zoneIdJson.contains("type") && zoneIdJson.contains("zonegroup") && zoneIdJson.contains("zoneindex") && zoneIdJson.contains("label")) {
            
            type = zoneIdJson["type"];
            zoneGroup = zoneIdJson["zonegroup"];
            zoneIndex = zoneIdJson["zoneindex"];
            label = zoneIdJson["label"];
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
    return (other.getUid()==getUid()) && (other.getLabel()==getLabel());
}

//--------------------------------------------------------------
bool ZoneId::operator!=(const ZoneId & other) const{
    return !(other==*this);
}

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
}

string ZoneId :: getLabel() {
    if(label=="") {
        
        string newlabel = "";
        
        if(type==BEAM) newlabel = "BEAM ";
        else newlabel = "CANVAS ";
        newlabel = newlabel +ofToString(zoneIndex+1);
        return newlabel;
            
    } else {
        return label;
    }
    
}

string ZoneId :: getUid() const {
    string uid = "";
    if(type==BEAM) uid = "B";
    else uid = "C";
    uid = uid+ofToString(zoneGroup)+"_"+ofToString(zoneIndex);
    return uid;
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
    return other.getUid()!=getUid();
}

//
//  ofxLaserObjectWithZoneId.cpp
//  example_Beams
//
//  Created by Seb Lee-Delisle on 03/03/2023.
//
//

#include "ofxLaserObjectWithZoneId.h"


using namespace ofxLaser;

void ObjectWithZoneId :: serialize(ofJson& json) const{
    
    ofJson& containerJson = json["zoneidobject"];
    
    return zoneId.serialize(containerJson);
    
    
//    ofJson& zoneIdJson = json["zoneId"];
//    zoneIdJson["type"] = (int)type;
//    zoneIdJson["zonegroup"] = zoneGroup;
//    zoneIdJson["zoneindex"] = zoneIndex;
//    zoneIdJson["label"] = label;
    //return true;
}
bool ObjectWithZoneId :: deserialize(ofJson& json) {
    ofJson& containerJson = json["zoneidobject"];
    return zoneId.deserialize(containerJson);
    
    
    
}

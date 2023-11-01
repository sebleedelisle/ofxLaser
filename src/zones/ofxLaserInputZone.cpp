//
//  ofxLaserZone.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserInputZone.h"

using namespace ofxLaser;

InputZone::InputZone(float x, float y, float w, float h) {
    //setIndex(0);
    zoneId.type = ZoneId::CANVAS;
    zoneId.zoneGroup = 0; 
	set(x, y, w,h);

}

void InputZone:: setZoneId(ZoneId zoneid)  {
    zoneId = zoneid;
   // setName("Z"+ofToString(index+1));
   // zoneLabel = "ZONE " + ofToString(index+1);
}

InputZone::~InputZone() {
	
}

void InputZone:: set(float x, float y, float w, float h) {
//    for(int i = 0; i<4; i++) {
//        float xpos = ((float)(i%2)/1.0f*w)+x;
//        float ypos = (floor((float)(i/2))/1.0f*h)+y;
//      //  cout << i<<" "<< xpos << " " << ypos << endl;
//        handles[i].set(xpos, ypos);
//
//
//    }
    rect.set(x,y,w,h);
    isDirty = true;

}

//
//bool InputZone::update() {
//
//    // not sure why we need this now?
//	if(isDirty) {
//		//rect.set(handles[0], handles[3]);
//		isDirty = false;
//		return true;
//		
//	}
//	return false;
//}

void InputZone::serialize(ofJson&json) const{
    
    ObjectWithZoneId::serialize(json);
    
    ofJson& containerJson = json["zoneidobject"];
    // adds json node for the handles, which is an array
    ofJson& handlesjson = containerJson["inputzone"];
    handlesjson = {rect.getLeft(), rect.getTop(), rect.getWidth(), rect.getHeight()};
    containerJson["locked"]  = locked;
    

}
bool InputZone::deserialize(ofJson&json) {
    
    ObjectWithZoneId::deserialize(json);
    
    ofJson& jsonGroup = json["zoneidobject"];
    cout << jsonGroup.dump(3) << endl;
    
    // OLD API - can delete at some point
    if(jsonGroup.contains("inputzone") && (jsonGroup["inputzone"].size()==4)) {

        ofJson& rectjson = jsonGroup["inputzone"];
        rect.set(rectjson[0],rectjson[1],rectjson[2],rectjson[3]);
        
        if(jsonGroup.contains("locked")) locked = jsonGroup["locked"].get<bool>();
        
        return true;
        
    } else {
        return false;
    }
   
    
}

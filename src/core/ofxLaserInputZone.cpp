//
//  ofxLaserZone.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserInputZone.h"

using namespace ofxLaser;

//Zone::Zone() {
//	ofLog(OF_LOG_NOTICE, "Zone() constructor") ;
////	editable = false;
//}

InputZone::InputZone(float x, float y, float w, float h) : QuadGui::QuadGui() {
    setIndex(0);
	set(x, y, w,h);
    setColours(ofColor(200,20,200), ofColor(200,20,200), ofColor(200,20,200));
	
    lockPerpendicular = true;
    for(DragHandle& handle : handles) {
        handle.setGrid(true, 5);
        
    }
    lineWidth = 2;
    
   // setConstrained(rect);
}

void InputZone:: setIndex(int _index)  {
    index = _index;
    setName("Z"+ofToString(index+1));
    zoneLabel = "ZONE " + ofToString(index+1); 
   
    
}

InputZone::~InputZone() {
	removeListeners();
	
}

void InputZone:: set(float x, float y, float w, float h) {
    QuadGui::set(x, y, w, h);
    rect.set(x,y,w,h);
    isDirty = true;

}

void InputZone::draw() {
    if(editable) {
        lineWidth =2;
    } else {
        lineWidth =1;
    } 
    QuadGui::draw();
}


bool InputZone::update() {

	if(isDirty) {
		rect.set(handles[0], handles[3]);
		isDirty = false;
		return true;
		
	}
	return false;
}



bool InputZone::deserialize(ofJson&jsonGroup) {
    bool success = QuadGui :: deserialize(jsonGroup);
    if(success) {
        rect.set(handles[0], handles[3]);
        return true;
    } else {
        return false;
    } 
    
    
}

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
    setIndex(0);
	set(x, y, w,h);

}

void InputZone:: setIndex(int _index)  {
    index = _index;
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


bool InputZone::update() {

    // not sure why we need this now?
	if(isDirty) {
		//rect.set(handles[0], handles[3]);
		isDirty = false;
		return true;
		
	}
	return false;
}

void InputZone::serialize(ofJson&json) const{
    
    // adds json node for the handles, which is an array
    ofJson& handlesjson = json["canvaszone"];
    
    handlesjson = {rect.getLeft(), rect.getTop(), rect.getWidth(), rect.getHeight()};
    

}
bool InputZone::deserialize(ofJson&jsonGroup) {
    
    // OLD API - can delete at some point
    if(jsonGroup.contains("quadguihandles") && (jsonGroup["quadguihandles"].size()>=4)) {
        ofJson& handlejson = jsonGroup["quadguihandles"];

        glm::vec2 topleft = glm::vec2(handlejson[0][0], handlejson[0][1]);
        glm::vec2 botright = glm::vec2(handlejson[3][0], handlejson[3][1]);
        rect.set(topleft, botright);
        return true;
        
    } else if(jsonGroup.contains("canvaszone") && (jsonGroup["canvaszone"].size()==4)) {

        ofJson& rectjson = jsonGroup["canvaszone"];
        rect.set(rectjson[0],rectjson[1],rectjson[2],rectjson[3]);
        return true;
        
    } else {
        return false;
    }
   
    
}

//
//  Object3D.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 10/06/2022.
//
#pragma once
#include "ofMain.h"

namespace ofxLaser {

class Object3D {
    
    public :
    
    Object3D() {
        position.set("3D Position", glm::vec3(0,0,0), glm::vec3(-100, -100,-100), glm::vec3(100,100, 100));
        orientation.set("3D Orientation", glm::vec3(0,0,0), glm::vec3(-180, -180, -180), glm::vec3(180,180,180));
     
    }
    
    ofParameter<glm::vec3> position;
    ofParameter<glm::vec3> orientation; // euler angles in degrees
     
    // add serialization / deserialization
    
};
}

//
//  Object3D.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 10/06/2022.
//
#pragma once
#include "ofMain.h"

namespace ofxLaser {

class Laser3DVisualObject {
    
    public :
    
    Laser3DVisualObject() {
        visual3DParams.setName("3D Visualisation");
        visual3DParams.add(position.set("3D Position", glm::vec3(0,0,0), glm::vec3(-1000, -1000,-1000), glm::vec3(1000,1000, 1000)));
        visual3DParams.add(orientation.set("3D Orientation", glm::vec3(0,0,0), glm::vec3(-180, -180, -180), glm::vec3(180,180,180)));
        visual3DParams.add(horizontalRangeDegrees.set("Output Range horizontal", 60,0,180));
        visual3DParams.add(verticalRangeDegrees.set("Output Range vertical", 60,0,180));
    }
    
    ofParameter<glm::vec3> position;
    ofParameter<glm::vec3> orientation; // euler angles in degrees
    ofParameter<int> horizontalRangeDegrees = 60;
    ofParameter<int> verticalRangeDegrees = 60;
    
    ofParameterGroup visual3DParams;
    
    // add serialization / deserialization
    
};
}

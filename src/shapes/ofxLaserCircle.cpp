//
//  ofxLaserCircle.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 16/11/2017.
//
//

#include "ofxLaserCircle.h"

using namespace ofxLaser;
//class Manager;
Circle::Circle(const glm::vec3& centre, const float radius, const ofColor& col, string profilelabel){
	
	
	reversable = false;
    setColours(vector<ofFloatColor>{col});

    //radius = _radius;
    //centre = _centre;
	
	glm::vec3 p;
    
    // TODO fade out overlap
	for(int angle = 0; angle<360; angle+=2) {
		p = glm::rotateZ(glm::vec3(radius, 0, 0), ofDegToRad(angle));
		//p.rotate(i, glm::vec3(0,0,1));
		p+=centre;
        // projection is now done within the laser manager
		points.push_back(p);
	}
    setClosed(true);
	
	tested = false;
	profileLabel = profilelabel;
	
}

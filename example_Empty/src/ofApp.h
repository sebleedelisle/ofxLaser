#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	void exit();
	

	ofxLaser::Manager laser;
	
	int laserWidth;
	int laserHeight; 
	
	ofxPanel laserGui;
	ofxPanel colourGui; 
	

};

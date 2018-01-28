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
	void keyPressed( int key );
	

	ofxLaser::Manager laser;
	
	int laserWidth;
	int laserHeight; 
	
	ofxPanel laserGui;
	ofxPanel colourGui; 
	

};

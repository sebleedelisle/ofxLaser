#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"


class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed  (int key);
	
	ofxLaser::Manager laser;
	
	int laserWidth;
	int laserHeight;
    		
	float elapsedTime; 

};


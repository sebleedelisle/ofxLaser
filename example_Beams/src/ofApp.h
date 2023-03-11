#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"
#include "StringGraphic.h"

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed(ofKeyEventArgs& e);
    
	ofxLaser::Manager laser;
    
    StringGraphic textGraphic; 
};


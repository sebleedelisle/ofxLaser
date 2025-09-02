#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
    ofPolyline makeStarPolyline(int numsides);
    
	void keyPressed(ofKeyEventArgs& e);
    
	ofxLaser::Manager laser;
    
    ofPolyline starPoly;

    
};


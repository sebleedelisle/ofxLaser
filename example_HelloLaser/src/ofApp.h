
#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"


class ofApp : public ofBaseApp{
	
public:
    
	void setup();
	void update();
	void draw();
	
    ofPolyline makeStarPolyline(int numsides, float size);
    
	void keyPressed(ofKeyEventArgs& e);
    
	ofxLaser::Manager laser;
    
    ofPolyline starPoly;
    ofPolyline starPolyInner;
    vector<ofColor> starPolyColours;
    //bool testscale = false; 
    
};


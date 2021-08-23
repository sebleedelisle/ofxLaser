#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
    
	void keyPressed(ofKeyEventArgs& e);
    
	ofxLaser::Manager laser;

    ofParameter<bool> boolParam;
    ofParameter<int> intParam;
    ofParameter<float> circleSize;
    ofParameter<int> resettableIntParam;
    ofParameter<ofColor> circleColour; 
    
};


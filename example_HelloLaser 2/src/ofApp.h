#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed(ofKeyEventArgs& e);
	void mouseDragged(ofMouseEventArgs& e);
	void mousePressed(ofMouseEventArgs& e);
	void mouseReleased(ofMouseEventArgs& e);
	
	void showLaserEffect(int effectnum);
	
	ofParameter<int> currentLaserEffect;
    ofParameter<float>timeSpeed;
    ofParameter<int> renderProfileIndex;
    ofParameter<string> renderProfileLabel;
    
    int numLaserEffects;
    float elapsedTime;
    
	ofxLaser::Manager laser;
	
	bool drawingShape = false;
	int laserWidth;
	int laserHeight;
    
	std::vector<ofPolyline> polyLines;
	
    ofParameter<ofColor> colour;
};


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
	
	void keyPressed  (int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	
	void showLaserEffect(int effectnum);
	
	ofParameter<int> currentLaserEffect;
	int numLaserEffects; 
	
	ofxLaser::Manager laser;
	
	bool drawingShape = false;
	int laserWidth;
	int laserHeight; 
	
	vector<ofPolyline> polyLines;
	
	ofxPanel laserGui;
	ofxPanel colourGui; 
	
	float elapsedTime; 

};

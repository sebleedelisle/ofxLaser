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
	
	ofxLaserX::Manager laser;
	
	bool drawingShape = false;
	int laserWidth;
	int laserHeight; 
	
	ofxPanel laserGui;
	ofxPanel redGui;
	ofxPanel greenGui;
	ofxPanel blueGui;
	
	
};

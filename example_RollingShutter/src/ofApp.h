#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed  (int key);
		
	void showLaserEffect(int effectnum);
	
	ofParameter<int> currentLaserEffect;
	
	ofParameter<int> startOffset;
	ofParameter<int> endOffset;
	ofParameter<int> pointsPerFrame;
	ofParameter<float> beamPos;
	ofParameter<float> timeSpeed;
	
	vector<ofxLaser::Point> points;
	int pointsToSend = 0;
	
	int numLaserEffects; 
	
	ofxLaser::Manager laser;
	
	bool drawingShape = false;
	int laserWidth;
	int laserHeight;
    
	std::vector<ofPolyline> polyLines;
		
	float elapsedTime;
	float startTime;

    ofParameter<ofColor> color;
};


#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"
#include "ofxLaserDacEtherdream.h"
#include "ofxLaserDacLaserdock.h"
#include "ofxLaserDacIDN.h"
#include "HeliosDac.h"

#define USE_ETHERDREAM
//#define USE_IDN
//#define USE_LASERDOCK

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
	
#ifdef USE_ETHERDREAM
	ofxLaser::DacEtherdream dac;
#endif
#ifdef USE_IDN
	ofxLaser::DacIDN dac;
#endif
#ifdef USE_LASERDOCK
	ofxLaser::DacLaserdock dac;
#endif
	
	bool drawingShape = false;
	int laserWidth;
	int laserHeight;
    
	std::vector<ofPolyline> polyLines;
		
	float elapsedTime;
	float startTime;

    ofParameter<ofColor> color;
};


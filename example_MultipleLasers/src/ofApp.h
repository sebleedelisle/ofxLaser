#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"
#include "ofxLaserDacLaserdock.h"
#include "ofxLaserDacHelios.h"

#define USE_LASERDOCK
//#define USE_HELIOS

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed  (int key);
	
	ofxLaser::Manager laser;
	
#ifdef USE_LASERDOCK
	ofxLaser::DacLaserdock dac1;
	ofxLaser::DacLaserdock dac2;
#endif
#ifdef USE_HELIOS
    ofxLaser::DacHelios dac1;
    ofxLaser::DacHelios dac2;
#endif
	
	int laserWidth;
	int laserHeight;
    		
	float elapsedTime; 

};


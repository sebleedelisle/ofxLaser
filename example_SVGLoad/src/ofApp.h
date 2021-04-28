#pragma once

#define USE_ETHERDREAM
//#define USE_IDN
//#define USE_LASERDOCK

#include "ofMain.h"
#include "ofxLaserManager.h"
#include "ofxLaserGraphic.h"



class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed  (int key);
		
    ofParameter<int> currentSVG;
    ofParameter<string> currentSVGFilename; 
    ofParameter<float> scale;
    ofParameter<bool> rotate3D;
    ofParameter<int> renderProfileIndex;
    ofParameter<string> renderProfileLabel;
    
	vector<ofxLaser::Graphic> laserGraphics;
	 vector<string> fileNames; 
	
	ofxLaser::Manager laser;
	
	int laserWidth;
	int laserHeight;
    
};


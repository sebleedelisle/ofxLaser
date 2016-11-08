#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"
#include "ofxGui.h"
#include "Asteroids.h"
#include "FlappyBird.h"

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void audioIn(float * input, int bufferSize, int numChannels);
	
	void showLaserEffect(int effectnum);
	
	ofParameter<int> currentLaserEffect;
	int numLaserEffects; 
	
	ofParameter<float> timeSpeed;
	
	ofxLaser::Manager laser;
	
	bool drawingShape = false;
	int laserWidth;
	int laserHeight; 
		
	ofxPanel laserGui;
	ofxPanel colourGui; 
	
	bool showGui;
	bool blankAll;
	
	float elapsedTime;
	float deltaTime;

	
	Asteroids asteroids;

	
	
	FlappyBird flappyBird;
	
	ofSoundStream soundStream;
	vector <float> left;
	vector <float> right;
	float smoothedInputVolume;

	
};


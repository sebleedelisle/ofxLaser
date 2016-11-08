//
//  Asteroids.h
//  example_Games
//
//  Created by Seb Lee-Delisle on 14/05/2016.
//
//

#pragma once

#include "ofxLaserManager.h"
#include "ofMain.h"
#include "TextWriter.h"
#include "ofxSvg.h"

struct Column {
	
	Column(float x, float gapheight, float gapsize)  {
		xpos = x;
		
		gapYPosition = gapheight;
		gapSize = gapsize;
		
	};
	
	float xpos, gapYPosition, gapSize;
	
};

class FlappyBird {


	public :
	
	FlappyBird();
	
	void update(float deltaTime, float inputlevel);
	void draw(ofxLaser::Manager &lm);
	void flap();
	
	void reset();
	void showScore(ofxLaser::Manager &laserManager);
	void renderBird(ofxLaser::Manager &laserManager);
	
	void transformPolyline(ofPolyline& source, ofPolyline& target, ofPoint pos, float rotation, ofPoint scale) {
		
		target = source;
		for(int i = 0; i<target.size(); i++) {
			ofPoint& p = target[i];
			p*=scale;
			p.rotate(rotation, ofPoint(0,0,1));
			p+=pos;
		}
	}
	
	int gameWidth, gameHeight;
	ofRectangle gameRect;
	int score = 0;
	
	float top, bottom, ypos, yvel, xpos ;
	float elapsedTime;
	int columnsMade;
	float columnsPerSecond;
	float radius;
	
	ofParameter<float> sensitivity;
	deque<Column> columns;
	
	bool alive;
	
	vector<ofxSVG> birdSVGs;
	int currentBirdSVG;
	
	TextWriter textWriter;

	
};

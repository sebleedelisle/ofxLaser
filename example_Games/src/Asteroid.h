//
//  Asteroid.h
//  example_Games
//
//  Created by Seb Lee-Delisle on 29/05/2016.
//
//

#pragma once
#include "ofMain.h"
#include "ofxLaserManager.h"

class Asteroid {
	
	public :
	
	Asteroid();
	
	void update(float deltaTime);
	void draw(ofxLaser::Manager &lm);
	void setRadius(float r);	
	
	float radius;
	ofPoint pos;
	ofPoint vel;
	ofPolyline asteroidImage;
	ofPolyline asteroidImageTransformed;
	bool enabled;
	
};


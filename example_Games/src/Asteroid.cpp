//
//  Asteroid.cpp
//  example_Games
//
//  Created by Seb Lee-Delisle on 29/05/2016.
//
//

#include "Asteroid.h"

Asteroid :: Asteroid() {
	setRadius(50);
	vel.set(1,0);
	vel.rotate(ofRandom(0,360),ofPoint(0,0,1));
	enabled = true;
};

void Asteroid :: update(float deltaTime) {
	pos+=vel*deltaTime*50;
}

void Asteroid :: draw(ofxLaser::Manager &lm) {
	lm.addLaserPolyline(asteroidImage, ofColor::white, pos, 0);
	
}

void Asteroid :: setRadius(float r) {
	
	int counter = 0;
	
	float anglechange = ofMap(r, 12, 50, 50,25);
	
	asteroidImage.clear();
	
	for(float angle=0; angle<=360; angle+=(anglechange * ofRandom(0.6,1.5))) {
		ofPoint temp;
		
		if(counter%2==0)
			temp.set(r,0);
		else
			temp.set(r * ofRandom(0.6,1), 0);
		
		temp.rotate(angle, ofPoint(0,0,1));
		
		asteroidImage.addVertex(temp);
		
		counter++;
	}
	asteroidImage.addVertex(ofPoint(r,0));
	
	radius = r;
	
}

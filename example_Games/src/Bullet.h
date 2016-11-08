//
//  Bullet.h
//  
//
//  Created by Seb Lee-Delisle on 29/05/2016.
//
//

#pragma once
#include "ofMain.h"

class Bullet {
	public :
	
	Bullet(ofPoint position, float direction) {
		pos = position;
		vel = ofPoint(10,0);
		vel.rotate(direction, ofPoint(0,0,1));
		lifeTime = 0;
		
	}
	void update(float deltaTime) {
		lifeTime+=deltaTime;
		pos+=vel*deltaTime*50;
	}
	
	ofPoint pos;
	ofPoint vel;
	float lifeTime;
	
};


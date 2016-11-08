//
//  Ship.h
//  
//
//  Created by Seb Lee-Delisle on 29/05/2016.
//
//
#pragma once
#include "ofMain.h" 
#include "ofxLaserManager.h"

class Ship {

	public :

	
	Ship() {
		
		shipImage.addVertex(-9,-7);
		shipImage.lineTo(-6,-3);
		shipImage.lineTo(-6,3);
		shipImage.lineTo(-9,7);
		shipImage.lineTo(14,0);
		shipImage.lineTo(-9,-7);
		
		flameImage.addVertex(-5,-5);
		flameImage.lineTo(-13,0);
		flameImage.lineTo(-5,5);
		
		reset();
		
		exploding = true;
		thrusting = false;
		explodeProgress =2;
	}
	
	void update(float deltaTime, ofRectangle& gameRect) {
		elapsedTime+=deltaTime;
		while(frameCount<elapsedTime*40.0f) {
			frameCount++;
			vel*=0.995;
		}
		
		pos+=vel*deltaTime;
		if(pos.x<gameRect.getLeft()) {
			pos.x = gameRect.getRight();
		} else if(pos.x>gameRect.getRight()) {
			pos.x = gameRect.getLeft();
		}
		if(pos.y<gameRect.getTop()) {
			pos.y = gameRect.getBottom();
		} else if (pos.y>gameRect.getBottom()) {
			pos.y = gameRect.getTop();
		}
		if(exploding) explodeProgress += deltaTime;
		
	}
	void thrust(float deltaTime) {
		
		if(exploding) return;
		
		ofPoint thrustvec(300*deltaTime,0);
		thrustvec.rotate(rotation, ofPoint(0,0,1));
		vel+=thrustvec;
		thrusting = true;
		

	}
	void draw(ofxLaser::Manager &lm) {
		
		if(!exploding) {
			lm.addLaserPolyline(shipImage, ofColor::white, pos, rotation);
			if((thrusting)&&(frameCount%4<2)) {
				
				lm.addLaserPolyline(flameImage, ofColor::white, pos, rotation);
			}
		} else {
			if(explodeProgress<=2) {
				float distance = ofMap(explodeProgress, 0, 2, 1, 0, true);
				distance*=distance*distance;
				distance = 1-distance;
				distance*=30;
				//ofLog(OF_LOG_NOTICE, ofToString(explodeProgress));
				for (int i = 0; i<3; i++) {
					ofPoint p1(4 + distance,-8);
					ofPoint p2(4 + distance,8);
					p1.rotate(120*i + 90, ofPoint(0,0,1));
					p2.rotate(120*i + 90, ofPoint(0,0,1));
					
					lm.addLaserLine(p1+pos, p2+pos, ofColor::white);
				}
			}
			
		}
		
		thrusting = false;
	}
	
	
	
	
	void rotateLeft(float deltaTime) {
		if(exploding) return;
		rotation-=6*deltaTime*50;
	}
	void rotateRight(float deltaTime) {
		if(exploding) return;
		rotation+=6*deltaTime*50;
	}
	
	void reset() {
		resetPosition();
		rotation = 0;
		thrusting = false;
		exploding = false;
	}
	void resetPosition() {
		pos.set(400,300);
		vel.set(0,0);

	}
	
	void explode() {
		exploding = true;
		explodeProgress = 0;
		vel.set(0,0);
		
	}
	float rotation;
	ofPoint pos;
	ofPoint vel;

	ofPolyline shipImage;
	ofPolyline flameImage;
	long frameCount;
	float elapsedTime;
	bool thrusting;
	bool exploding;
	float explodeProgress;


};
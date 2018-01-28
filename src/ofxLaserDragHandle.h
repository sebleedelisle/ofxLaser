//
//  DragHandle.h
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//
#pragma once

#include "ofMain.h"

namespace ofxLaser {
class DragHandle : public ofPoint{
	
	
	public :
	DragHandle() {
		set(0,0);
	};
	

	void set(float xpos, float ypos, float r = 8) {
		
		x = xpos;
		y = ypos;
		radius = r;
	};
	
	void set(ofPoint pos) {
		ofPoint::set(pos);
	
	};
	
	void draw(bool isOver = true) {
		//bool isOver = hitTest(ofPoint(ofGetMouseX(), ofGetMouseY()));
		ofPushStyle();
		if(isCircular) {
			ofSetColor(isOver?overCol:col);
			ofNoFill();
			ofSetLineWidth(1);
			ofDrawCircle(*this,radius);
		} else {
			
			ofFill();
			ofSetColor(isOver?overCol:col);
			//ofSetLineWidth(1);
			ofSetRectMode(OF_RECTMODE_CENTER);
			ofDrawRectangle(*this,radius, radius);
		}
		ofPopStyle();
	};
	
	void startDrag(ofPoint clickPos, bool dragXAxis = true, bool dragYAxis = true, bool dontMoveWhenAltPressed = false) {
		clickOffset = clickPos - *this;
		isDragging = true;
		xAxis = dragXAxis;
		yAxis = dragYAxis;
		altKeyDisable = dontMoveWhenAltPressed;
		startPos = *this;
		
		
	};
	
	bool updateDrag(ofPoint pos) {
		
		
		if(isDragging) {
			
			if(xAxis){
				x = startPos.x + (((pos.x - clickOffset.x) - startPos.x) * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.2 : 1));
			}
			if(yAxis) {
				y = startPos.y + (((pos.y - clickOffset.y) - startPos.y) * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.2 : 1));
			}
			
			if(altKeyDisable && ofGetKeyPressed(OF_KEY_ALT)) {
				x = startPos.x;
				y = startPos.y;
			}
			
			return true;
		} else {
			return false;
		}
	}
	
	bool stopDrag(){
		if(isDragging) {
			isDragging = false;
			return true;
		} else {
			return false;
		}
	};
	
	bool hitTest(ofPoint hitpoint) {
		return(distance(hitpoint)<radius);
	}
	
	ofPoint clickOffset, startPos;
	bool isDragging = false;
	
	float radius = 5;
	bool xAxis;
	bool yAxis;
	bool altKeyDisable;
	bool isCircular = false;
	
	ofColor col = ofColor(60);
	ofColor overCol = ofColor::white;
	
	
};
}

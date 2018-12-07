//
//  DragHandle.h
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//
#pragma once

#include "ofMain.h"

namespace ofxLaser {
	class DragHandle : public glm::vec3{
	
	
	public :
	DragHandle() {
		set(0,0);
	};
	

	void set(float xpos, float ypos, float r = 5) {
		
		x = xpos;
		y = ypos;
		radius = r;
	};
	
	void set(glm::vec3 pos) {
		x = pos.x;
		y = pos.y;
		z = pos.z;
		
		//ofLog(OF_LOG_NOTICE, "DragHandle::set " + ofToString(pos) + " " +ofToString(x) + " " + ofToString(y));
		
	};
	void set(glm::vec2 pos) {
		x = pos.x;
		y = pos.y;
		z = 0; 
		
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
			
			if(isFilled) ofFill();
            else ofNoFill();
			ofSetColor(isOver?overCol:col);
			ofSetLineWidth(1);
			ofSetRectMode(OF_RECTMODE_CENTER);
			ofDrawRectangle(*this,radius, radius);
		}
		ofPopStyle();
	};
	
	void startDrag(glm::vec3 clickPos, bool dragXAxis = true, bool dragYAxis = true, bool dontMoveWhenAltPressed = false) {
		clickOffset = clickPos - *this;
		isDragging = true;
		xAxis = dragXAxis;
		yAxis = dragYAxis;
		altKeyDisable = dontMoveWhenAltPressed;
		startPos = *this;
		
		
	};
	
	bool updateDrag(glm::vec3 pos) {
		
		
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
	
	bool hitTest(glm::vec3 hitpoint) {
		return( glm::distance( (glm::vec3) *this, hitpoint ) < radius );
	}
	
	ofPoint clickOffset, startPos;
	bool isDragging = false;
	
	float radius = 5;
	bool xAxis;
	bool yAxis;
	bool altKeyDisable;
	bool isCircular = false;
    bool isFilled = false;
	
	ofColor col = ofColor(60);
	ofColor overCol = ofColor(255);
	
	
};
}

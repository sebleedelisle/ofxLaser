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
		
	DragHandle(glm::vec3 p) {
		set(p); 
	}
	DragHandle(float x, float y) {
		set(x, y);
	}
	
	void set(float xpos, float ypos, float r =5) {
		
		x = xpos;
		y = ypos;
		radius = r;
	};
	
	void set(glm::vec3 pos) {
		x = pos.x;
		y = pos.y;
		z = pos.z;
		
	};
	void set(glm::vec2 pos) {
		x = pos.x;
		y = pos.y;
		z = 0; 
		
	};
	void draw(bool isOver = true, float scale = 1) {
		if(!active) return; 
		//bool isOver = hitTest(ofPoint(ofGetMouseX(), ofGetMouseY()));
		ofPushStyle();
		
		if(isFilled) ofFill();
		else {
			ofNoFill();
			ofSetLineWidth(1);
		}
		ofSetColor(isOver?overCol:col);
		
		if(isCircular) {
			ofDrawCircle(*this,radius*scale);
		} else {
			ofSetRectMode(OF_RECTMODE_CENTER);
			ofDrawRectangle(*this,radius*2*scale, radius*2*scale);
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
			
			if(xAxis && !xLocked){
				x = startPos.x + (((pos.x - clickOffset.x) - startPos.x) * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.2 : 1));
			}
			if(yAxis && !yLocked) {
				y = startPos.y + (((pos.y - clickOffset.y) - startPos.y) * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.2 : 1));
			}
			
			if(altKeyDisable && ofGetKeyPressed(OF_KEY_ALT)) {
				x = startPos.x;
				y = startPos.y;
			}
			if(snapToGrid) {
				x = round(x*(1/gridSize))*gridSize;
				y = round(y*(1/gridSize))*gridSize;
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
		return(active && glm::distance( (glm::vec3) *this, hitpoint ) < radius );
	}
	
	ofPoint clickOffset, startPos;
	bool isDragging = false;
	bool active = true;
	float radius = 4;
	bool xAxis;
	bool yAxis;
	
	bool xLocked = false;
	bool yLocked = false;
	
	bool altKeyDisable;
	bool isCircular = true;
    bool isFilled = false;
		
	bool snapToGrid = false;
	float gridSize = 1;
		
	vector<DragHandle*>connectedHandlesX;
	vector<DragHandle*>connectedHandlesY;
		
	
	ofColor col = ofColor(60);
	ofColor overCol = ofColor(255);
	
	
};
}

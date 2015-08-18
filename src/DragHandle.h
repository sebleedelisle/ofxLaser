//
//  DragHandle.h
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//
#pragma once

#include "ofMain.h"

class DragHandle : public ofPoint{
	
	
	public :
	DragHandle() {
		set(0,0);
	};
	
//	DragHandle(float xpos, float ypos) {
//		set(xpos, ypos);
//		
//	};
//	
	void set(float xpos, float ypos, float r = 8) {
		
		x = xpos;
		y = ypos;
		radius = r;
	};
	
	void set(ofPoint pos) {
		ofPoint::set(pos);
	
	};
	
	void draw() {
		ofPushStyle();
		ofNoFill();
		ofSetLineWidth(1);
		ofCircle(*this, radius);
		ofPopStyle();
	};
	
	void startDrag(ofPoint clickPos, bool dragXAxis = true, bool dragYAxis = true) {
		offset = clickPos - *this;
		isDragging = true;
		xAxis = dragXAxis;
		yAxis = dragYAxis;
		
		
	};
	
	bool updateDrag(ofPoint pos) {
		if(isDragging) {
			if(xAxis) x = pos.x - offset.x;
			if(yAxis) y = pos.y - offset.y;
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
	
	ofPoint offset;
	bool isDragging = false;
	
	float radius = 5;
	bool xAxis;
	bool yAxis;
	
	
};
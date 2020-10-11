//
//  DragHandle.cpp
//
//  Created by Seb Lee-Delisle
//
//

#include "ofxLaserDragHandle.h"

using namespace ofxLaser;


DragHandle::DragHandle() {
	set(0,0);
};
	
DragHandle::DragHandle(glm::vec3 p) {
	set(p);
}
DragHandle::DragHandle(float x, float y) {
	set(x, y);
}

void DragHandle::set(float xpos, float ypos, float r) {
	
	x = xpos;
	y = ypos;
	radius = r;
};
void DragHandle::setSize(float r) {
	radius = r;
};

void DragHandle::set(glm::vec3 pos) {
	x = pos.x;
	y = pos.y;
	z = pos.z;
	
};
void DragHandle::set(glm::vec2 pos) {
	x = pos.x;
	y = pos.y;
	z = 0;
	
};
void DragHandle::draw(bool isOver, float scale) {
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

void DragHandle::startDrag(glm::vec3 clickPos, bool dragXAxis, bool dragYAxis, bool dontMoveWhenAltPressed ) {
	clickOffset = clickPos - *this;
	isDragging = true;
	xAxis = dragXAxis;
	yAxis = dragYAxis;
	altKeyDisable = dontMoveWhenAltPressed;
	startPos = *this;
	
};

bool DragHandle::updateDrag(glm::vec3 pos) {
	
	
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

bool DragHandle::stopDrag(){
	if(isDragging) {
		isDragging = false;
		return true;
	} else {
		return false;
	}
};

bool DragHandle::hitTest(glm::vec3 hitpoint) {
	return(active && glm::distance( (glm::vec3) *this, hitpoint ) < radius );
}


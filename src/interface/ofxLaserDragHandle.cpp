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

void DragHandle::set(float xpos, float ypos, float _size) {
	
	x = xpos;
	y = ypos;
	size = _size;
};
void DragHandle::setSize(float _size) {
	size = _size;
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


void DragHandle::draw(const glm::vec3& mousepos, float scale) {
	
	draw(hitTest(mousepos, scale), scale);
	
};

void DragHandle::draw(bool isOver , float scale ) {
    if(!active) return;
    ofPushStyle();
    
    if(isFilled) ofFill();
    else {
        ofNoFill();
        ofSetLineWidth(1);
    }
    
    //ofDrawLine(mousepos.x, mousepos.y-4, mousepos.x, mousepos.y+4);
    //ofDrawLine(mousepos.x-4, mousepos.y, mousepos.x+4, mousepos.y);
    
    ofSetColor(isOver?overCol:col);
    
    if(isCircular) {
        ofDrawCircle(*this,size/2/scale);
    } else {
        ofSetRectMode(OF_RECTMODE_CENTER);
        ofDrawRectangle(*this,size/scale, size/scale);
        //ofDrawRectRounded(*this,size/scale, size/scale, 2/scale);
    
       
    }
    ofPopStyle();
}



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

bool DragHandle::hitTest(glm::vec3 hitpoint, float scale) {
    if(active) {
        if(isCircular) {
            return (glm::distance( (glm::vec3) *this, hitpoint ) < size/2/scale );
        } else {
            ofRectangle rect;
            rect.setFromCenter(*this, size/scale, size/scale );
            return rect.inside(hitpoint);
        }
    } else {
        return(false);
    }
}


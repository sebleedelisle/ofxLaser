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
DragHandle::DragHandle(glm::vec2 p) {
    set(p);
}
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
	//z = pos.z;
	
};
void DragHandle::set(glm::vec2 pos) {
	x = pos.x;
	y = pos.y;
	//z = 0;
	
};

void DragHandle::draw(const glm::vec3& mousepos, float scale) {
    draw(glm::vec2(mousepos), scale);
};
void DragHandle::draw(const glm::vec2& mousepos, float scale) {
	draw(hitTest(mousepos,scale), scale);
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


void DragHandle::startDrag(glm::vec2 clickPos, DragHandle* relativeToHandle){
    dragProportional = false;
    dragSymmetrical = false;
    
    if((relativeToHandle!=nullptr) && (relativeToHandle!=this)) {
        dragRelativeToHandle = relativeToHandle;
        dragRelativeOffset = *this-*relativeToHandle;
    } else {
        dragRelativeToHandle = nullptr;
    }
    clickOffset = clickPos - *this;
    altKeyDisable = false;
    startPos = *this;
    isDragging = true;

}

void DragHandle::startDragProportional(glm::vec2 clickPos, glm::vec2 _anchorPos, glm::vec2 refPos,  bool dontMoveWhenAltPressed){

        
    dragProportional = true;
    dragSymmetrical = false;
    anchorPos = _anchorPos;
    referencePos = refPos;
    clickOffset = clickPos - refPos;
    altKeyDisable = dontMoveWhenAltPressed;
    startPos = *this;
    isDragging = true;

}

void DragHandle::startDragSymmetrical(glm::vec2 clickPos, glm::vec2 _anchorPos, glm::vec2 refPos,  bool dontMoveWhenAltPressed){

        
    dragSymmetrical = true;
    dragProportional = false;
    anchorPos = _anchorPos;
    referencePos = refPos;
    clickOffset = clickPos - refPos;
    altKeyDisable = dontMoveWhenAltPressed;
    startPos = *this;
    isDragging = true;

}
bool DragHandle::updateDrag(glm::vec2 mousePos) {
	
	
	if(isDragging) {
        
        if(altKeyDisable && ofGetKeyPressed(OF_KEY_ALT)) {
            x = startPos.x;
            y = startPos.y;
        } else if(dragProportional) {
			
            
            // DRAGGING PROPORTIONALLY
            // TODO drag a small around when shift is pressed!
            
            glm::vec2 currentReferencePos;//  = mousePos - clickOffset;
            currentReferencePos.x = startPos.x + (((mousePos.x - clickOffset.x) - startPos.x) * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.2 : 1));
            currentReferencePos.y = startPos.y + (((mousePos.y - clickOffset.y) - startPos.y) * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.2 : 1));
            
            if(snapToGrid) {
                currentReferencePos.x = round(currentReferencePos.x*(1/gridSize))*gridSize;
                currentReferencePos.y = round(currentReferencePos.y*(1/gridSize))*gridSize;
            }

            // conditional avoids division by zero
            if(startPos.x != anchorPos.x) {
                x = ofMap(startPos.x, anchorPos.x, referencePos.x, anchorPos.x, currentReferencePos.x);
            }
            // conditional avoids division by zero
            if(startPos.y != anchorPos.y) {
                y = ofMap(startPos.y, anchorPos.y, referencePos.y, anchorPos.y, currentReferencePos.y);
            }

            
            if((x==0) && (y ==0)) {
                ofLogError("drag point corrupted ;(");
            }
        } else if(dragSymmetrical) {
            
            // DRAGGING SYMETRICALLY
            glm::vec2 currentReferencePos = mousePos - clickOffset;
            if(snapToGrid) {
                currentReferencePos.x = round(currentReferencePos.x*(1/gridSize))*gridSize;
                currentReferencePos.y = round(currentReferencePos.y*(1/gridSize))*gridSize;
            }

            glm::vec2 v = currentReferencePos - anchorPos;
            this->set(anchorPos-v);
            
        } else {
            if(dragRelativeToHandle!=nullptr) {
                ofPoint pos = *dragRelativeToHandle+dragRelativeOffset;
                x = pos.x;
                y = pos.y;
                
            } else {
                x = startPos.x + (((mousePos.x - clickOffset.x) - startPos.x) * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.2 : 1));
                y = startPos.y + (((mousePos.y - clickOffset.y) - startPos.y) * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.2 : 1));

                if(snapToGrid) {
                    x = round(x*(1/gridSize))*gridSize;
                    y = round(y*(1/gridSize))*gridSize;
                }
            }
            
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

bool DragHandle::hitTest(glm::vec2 hitpoint, float scale) {
    if(active) {
        if(isCircular) {
            return (glm::distance( (glm::vec2) *this, hitpoint ) < size/2/scale );
        } else {
            ofRectangle rect;
            rect.setFromCenter(*this, size/scale+1, size/scale+1 );
            return rect.inside(hitpoint);
        }
    } else {
        return(false);
    }
}


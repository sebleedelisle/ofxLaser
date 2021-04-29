//
//  QuadGui.cpp
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//

#include "ofxLaserQuadGui.h"
using namespace ofxLaser;

QuadGui::QuadGui() {
    initListeners();
    
    visible = true;
    isDirty=true;
    selected = false;
    
    for(int i = 0; i<=numHandles; i++) {
        quadPoly.addVertex(0,0,0);
    }
    
    set(0,0,60,60);
    initialised = true;
    lineColour.set(255);
    handleColour.set(255);
    labelColour.set(255);
   
    updatePoly();
    
}
QuadGui::~QuadGui() {
    if(initialised) removeListeners();
    
}

void QuadGui::setName (string displaylabel) {
    displayLabel = displaylabel;
}


void QuadGui::set (const ofRectangle& rect) {
    set(rect.x, rect.y, rect.getWidth(), rect.getHeight());
}

void QuadGui::set(float x, float y, float w, float h) {
    
    allHandles.clear();
    
    for(int i = 0; i<4; i++) {
        float xpos = ((float)(i%2)/1.0f*w)+x;
        float ypos = (floor((float)(i/2))/1.0f*h)+y;
        
        handles[i].set(xpos, ypos);
         
        allHandles.push_back(&handles[i]);
        
    }
    
    centreHandle.set(x + (w/2.0f), y+(h/2.0f));
    allHandles.push_back(&centreHandle);
    
    updatePoly();
    
}

void QuadGui::setConstrained(const ofRectangle &rect) {
    constrainRect = rect;
    constrained = true;
}

void QuadGui :: initListeners() {
    
    ofAddListener(ofEvents().mousePressed, this, &QuadGui::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseMoved, this, &QuadGui::mouseMoved, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseReleased, this, &QuadGui::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseDragged, this, &QuadGui::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    
}

void QuadGui :: removeListeners() {
    
    ofRemoveListener(ofEvents().mousePressed, this, &QuadGui::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseMoved, this, &QuadGui::mouseMoved, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseReleased, this, &QuadGui::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseDragged, this, &QuadGui::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    
}
void QuadGui :: setColours(ofColor _lineColour, ofColor _handleColour, ofColor _labelColour){
    
    handleColour = _handleColour;
    lineColour = _lineColour;
    labelColour = _labelColour;
    
    for(DragHandle* handle : allHandles) {
        handle->overCol = handleColour;
        handle->col = handleColour*0.5;
    }
    
}

void QuadGui :: draw() {
	
    if(!visible) {
        isDirty = false;
        return;
    }
    if(!editable) {
        selected = false;
    }
    
    ofPushMatrix();
    ofTranslate(offset);
    ofScale(scale, scale);
    
	ofPushStyle();
	ofNoFill();
	ofSetLineWidth(1);

    
    ofSetColor(lineColour);
    
	//glm::vec3 p1 = glm::mix((glm::vec3)handles[0],(glm::vec3)handles[1], 0.1);
	//glm::vec3 p2 = glm::mix((glm::vec3)handles[0],(glm::vec3)handles[2], 0.1);
    glm::vec3 shift(0.5,0.5,0);
    if(editable) {
		
        UI::drawDashedLine(handles[1]+shift, handles[3]+shift);
        UI::drawDashedLine(handles[3]+shift, handles[2]+shift);
        UI::drawDashedLine(handles[0]+shift, handles[1]+shift);
        UI::drawDashedLine(handles[2]+shift, handles[0]+shift);

    } else {
        ofSetColor(lineColour*0.5);
        ofDrawLine(handles[1]+shift, handles[3]+shift);
        ofDrawLine(handles[3]+shift, handles[2]+shift);
        ofDrawLine(handles[0]+shift, handles[1]+shift);
        ofDrawLine(handles[2]+shift, handles[0]+shift);
       // ofRectangle rect(handles[0], )
        
    }
    
    
    if(!displayLabel.empty()) {
        float textwidth = displayLabel.size()*8;

        ofFill();
        ofPushMatrix();
        ofTranslate(handles[1]);
        ofScale(1/scale, 1/scale, 1); //-ofPoint(textwidth+12,1.5));
        ofTranslate(-textwidth-10, 1);
        ofSetColor(0,150);
        
        ofDrawRectangle(0,0,textwidth+10,18);
        ofSetColor(labelColour * (editable?1:0.5));

        ofDrawBitmapString(displayLabel, 7, 13);

        ofPopMatrix();
       
    }
    if(selected) {
        for(int i = 0; i<numHandles; i++) {
            handles[i].draw(mousePos, scale);
        }
        centreHandle.draw(mousePos, scale);
    }
    
  
    ofPopStyle();
	isDirty = false;
    ofPopMatrix();
}


bool QuadGui::checkDirty() {
	if(isDirty) {
        isDirty = false; 
		return true;
	} else {
		return false;
	}
}


void QuadGui :: startDragging(int handleIndex, glm::vec3 clickPos) {
	
	handles[handleIndex].startDrag(clickPos);
    
    int x = ((handleIndex%2)+1)%2;
    int y = handleIndex/2;
    
    int xhandleindex = x+(y*2);
    
    x = handleIndex%2;
    y = ((handleIndex/2)+1)%2;
    int yhandleindex = x+(y*2);
    
    handles[xhandleindex].startDrag(clickPos, false, true, !lockPerpendicular);
    handles[yhandleindex].startDrag(clickPos, true, false, !lockPerpendicular);
    
	
}
bool QuadGui :: hitTestScreen(ofPoint mousePoint) {
    
    mousePoint-=offset;
    mousePoint/=scale;
    
    if(!boundingBox.inside(mousePoint)) return false;
    return(quadPoly.inside(mousePoint));

}
bool QuadGui :: hitTest(const ofPoint& p) {
    
    if(!boundingBox.inside(p)) return false;
    else return(quadPoly.inside(p));

}


bool QuadGui :: mousePressed(ofMouseEventArgs &e){
	
        
	if(!visible) return false;

    bool hit = hitTestScreen(e);
    if((hit) &&(!selected)) {
        selected = true;
        return true;
    }
    
        
    if(!selected) {
        return false;
    }
    ofPoint mousePoint = e;
    mousePoint-=offset;
    mousePoint/=scale;
    
   
    
	bool handleHit = false;
	
    
	if(centreHandle.hitTest(mousePoint)) {
		
		centreHandle.startDrag(mousePoint);
		handleHit = true;
		for(int i = 0; i<numHandles; i++) {
			handles[i].startDrag(mousePoint);
		}
			
    
    } else {

		for(int i = 0; i<numHandles; i++) {
			if(handles[i].hitTest(mousePoint)) {
				startDragging(i, mousePoint);
				handleHit = true;
			}
		
		}
	}
	
    if(!handleHit && !hit) {
        selected = false;
    }
	return handleHit;
	
}
bool QuadGui :: mouseMoved(ofMouseEventArgs &e){
    mousePos = e;
    mousePos-=offset;
    mousePos/=scale;
    
}
bool QuadGui :: mouseDragged(ofMouseEventArgs &e){

	if(!visible) return false;
	if(!selected) return false;

    mousePos = e;
    mousePos-=offset;
    mousePos/=scale;
    
	ofRectangle bounds(centreHandle, 0, 0);
	bool dragging = false;
	for(int i = 0; i<numHandles; i++) {
        DragHandle& handle = handles[i];
        if(handle.updateDrag(mousePos)){
            dragging = true;
            if(constrained && (!constrainRect.inside(handle))) {
                handle.x = ofClamp(handle.x, constrainRect.getLeft(), constrainRect.getRight());
                handle.y = ofClamp(handle.y, constrainRect.getTop(), constrainRect.getBottom());
            }

            bounds.growToInclude(handle);
        }
	}
 	if(!dragging) {
		dragging = centreHandle.updateDrag(mousePos);
	} else {
		updateCentreHandle();
        updatePoly();
	}
	
	//isDirty |= dragging;
	
	
	return dragging;
	
	
}
void QuadGui :: updatePoly() {
    vector<glm::vec3>& vertices = quadPoly.getVertices(); 
   
    vertices[0] = (handles[0]);
    vertices[1] = (handles[1]);
    vertices[2] = (handles[3]);
    vertices[3] = (handles[2]);
    vertices[4] = (handles[0]);
   
    boundingBox = quadPoly.getBoundingBox();
    
}

bool QuadGui :: mouseReleased(ofMouseEventArgs &e){
	
	if(!visible) return false;
	if(!selected) return false;

	
	bool wasDragging = false;
	
	for(int i = 0; i<allHandles.size(); i++) {
		if(allHandles[i]->stopDrag()) wasDragging = true;
	}
    if(wasDragging) {
//        if(!reversable) {
//            // TODO FIGURE OUT HOW TO UNWARP RECT!
//
//        }
        //saveSettings();
    }
    
    isDirty |= wasDragging;
	return wasDragging;
	
}



void QuadGui::updateCentreHandle() {
	centreHandle.set(0,0);
	for(int i = 0; i<4; i++) {
		centreHandle+=(glm::vec3)handles[i];
	}
	centreHandle/=4;
	//	centreHandle.set(bounds.getCenter());
}



void QuadGui::serialize(ofJson&json) {
	
    // adds json node for the handles, which is an array
    ofJson& handlesjson = json["quadguihandles"];
	for(int i = 0; i<4; i++) {
		DragHandle& pos = handles[i];
		handlesjson.push_back({pos.x, pos.y});
	}

}

bool QuadGui::deserialize(ofJson& jsonGroup) {
	
    ofJson& handlejson = jsonGroup["quadguihandles"];
	
    if(handlejson.size()>=4) {
        for(int i = 0; i<4; i++) {
            ofJson& point = handlejson[i];
      
            handles[i].x = point[0];
            handles[i].y = point[1];
            handles[i].z = 0;
            
        }
        updateCentreHandle();
        updatePoly();
        return true;
    } else {
        return false;
    }
    
}

void QuadGui::setVisible(bool warpvisible) {
	visible = warpvisible;
}



//
//  ofxLaserMoveablePoly.cpp
//
//
//  Created by Seb Lee-Delisle on 03/02/2023.
//
//

#include "ofxLaserMoveablePoly.h"

using namespace ofxLaser;

MoveablePoly :: MoveablePoly() {
    selected = false;
    snapToGrid = false;
    gridSize = 10;
    mousePos = {0,0};
    
    resetColours();
    
    label = "";
    
}


bool MoveablePoly :: update() {
    if(isDirty) {
        updatePoly();
         
    }
    
    
    
    bool wasDirty = isDirty;
    isDirty = false;
    return wasDirty;
}


void MoveablePoly :: draw() {
    
    ofPushStyle();
    
    ofColor fColour = selected ? fillColourSelected : fillColour;
    if(dimmed) fColour*=0.5;
    ofSetColor(fColour);
    
    ofFill();
    if(!isDisabled) drawShape();
    
    ofNoFill();
    if(selected) {
        ofSetLineWidth(2);
    }
    
    ofColor sColour = selected ? strokeColourSelected : strokeColour;
    if(dimmed) sColour*=0.5;
    else if(isDisabled) sColour*=0.7;
    ofSetColor(sColour);
    drawShape(); 
    drawHandlesIfSelectedAndNotDisabled();
    ofPopStyle();
}


void MoveablePoly :: setScale(float _scale) {
    if(scale!=_scale) {
        scale = _scale;
        for(DragHandle& handle : handles) {
            handle.setSize(10/scale);
        }
    }
}

void MoveablePoly :: drawHandlesIfSelectedAndNotDisabled() {
    if(getSelected() && !getDisabled()) {
        for(DragHandle& handle : handles) {
            if(&handle == getMainDragHandle()) {
                handle.draw(true);
            } else {
                handle.draw(mousePos);
            }
        }
    }
}

void MoveablePoly :: drawLabel() {
    ofPushStyle();
   
    if(dimmed) {
        ofSetColor(strokeColour*0.5);
    } else {
        ofSetColor(strokeColour);
    }
    
    ofDrawBitmapString(getLabel(), centre - glm::vec3(4.0f*label.size()/scale,-4.0f/scale, 0));
    
    ofPopStyle();
    
}

DragHandle* MoveablePoly :: getMainDragHandle() {
    if(mainDragHandleIndex<0) {
        return nullptr;
    } else {
        return &handles[mainDragHandleIndex];
    }
}

int MoveablePoly :: getMainDragHandleIndex() {
    return mainDragHandleIndex;
}

bool MoveablePoly::getSelected() {
    return selected;
    
};

bool MoveablePoly::setSelected(bool v) {
    if(selected!=v)  {
        selected = v;
        for(DragHandle& handle : handles) {
            handle.stopDrag();
        }
        return true;
    } else {
        return false;
    }
};


bool MoveablePoly::getDisabled() {
    return isDisabled;
    
};

bool MoveablePoly::setDisabled(bool v) {
    if(isDisabled!=v)  {
        isDisabled = v;
        for(DragHandle& handle : handles) {
            handle.stopDrag();
        }
        if(isDisabled) setSelected(false);
        return true;
    } else {
        return false;
    }
};

glm::vec2 MoveablePoly :: getCentre() {
    return centre;
}

bool MoveablePoly :: setGrid(bool snapstate, int gridsize) {
    if((snapstate!=snapToGrid) || (gridSize!=gridsize)) {
        snapToGrid = snapstate;
        gridSize = gridsize;
        
        for(DragHandle& handle : handles) {
            handle.setGrid(snapstate, gridsize);
        }
        
        return true;
    } else {
        return false;
    }
}

void MoveablePoly::setHue(int hue) {
    
    resetColours();
    fillColour.setHue(hue);
    fillColourSelected.setHue(hue);
    strokeColour.setHue(hue);
    strokeColourSelected.setHue(hue);
    handleColour.setHue(hue);
    handleColourOver.setHue(hue);
    
    updateHandleColours();
  
}

void MoveablePoly::resetColours() {
    
    fillColour  = ofColor::fromHex(0x001123, 128);
    fillColourSelected = ofColor::fromHex(0x001123);
    strokeColour  = ofColor::fromHex(0x0E87E7);
    strokeColourSelected = ofColor::fromHex(0x0E87E7);
    handleColour = ofColor::fromHex(0x0E87E7);
    handleColourOver = ofColor :: fromHex(0xffffff);
    
}
void MoveablePoly::setBrightness(int brightint) {
  
    float brightness = (float)brightint/255.0f;
    fillColour *=brightness;
    fillColourSelected*=brightness;
    strokeColour*=brightness;
    strokeColourSelected*=brightness;
    handleColour*=brightness;
    handleColourOver*=brightness;
    
}

void MoveablePoly::setSaturationFloat(float saturationmultiplier) {
    fillColour.setSaturation(fillColour.getSaturation()*saturationmultiplier);
    fillColourSelected.setSaturation(fillColourSelected.getSaturation()*saturationmultiplier);
    strokeColour.setSaturation(strokeColour.getSaturation()*saturationmultiplier);
    strokeColourSelected.setSaturation(strokeColourSelected.getSaturation()*saturationmultiplier);
    handleColour.setSaturation(handleColour.getSaturation()*saturationmultiplier);
    handleColourOver.setSaturation(handleColourOver.getSaturation()*saturationmultiplier);

}


void MoveablePoly ::updateHandleColours() {
    for(DragHandle& handle : handles) {
        handle.setColour(handleColour, handleColourOver);
    }
}


// TODO maybe we can handle selection at a higher level?
bool MoveablePoly :: mousePressed(ofMouseEventArgs &e) {
    mousePos = e;
    
    if(isDisabled) return true;
        
    bool handlehit = false;

    // click order so we have a sensible priority when handles overlap
    int clickorder[] = {2,1,3,0};

    if(getSelected()) {
        
        // then check all the drag points
        for (int index = 0; index<handles.size() && !handlehit; index++) {
            
            int i = index;
            
            // if it's a quad then we need to get the points in the right order
            if(isQuad()) i = clickorder[index];
            
            DragHandle& currentHandle = handles[i];
            
            if(currentHandle.hitTest(mousePos)) {
                handlehit = true;
                startDraggingHandleByIndex(i);
                
                // if we're not distorted then also start dragging the relavent corners
                if(isQuad() && poly.isAxisAligned()) {
                    DragHandle& anchorHandle = handles[(i+2)%4];
                    DragHandle& dragHandle1 = handles[(i+1)%4];
                    DragHandle& dragHandle2 = handles[(i+3)%4];

                    dragHandle1.startDragProportional(mousePos, anchorHandle, currentHandle, true);
                    dragHandle2.startDragProportional(mousePos, anchorHandle, currentHandle, true);

                    constrainedToSquare =true;
                } else {
                    constrainedToSquare =false;
                }
            }
        }
        
        if(!handlehit) {
            // if we haven't hit a handle but we have
            // hit the zone, start dragging the zone
            if(hitTest(mousePos)) {
                //ofLogNotice("start drag moveable poly");
                DragHandle* gridHandle = &handles[0];
                
                for(size_t i= 0; i<handles.size(); i++) {
                    if(&handles[i] == gridHandle) {
                        handles[i].startDrag(mousePos);
                    } else {
                        handles[i].startDrag(mousePos, gridHandle);
                    }
                }
                isDragging = true;
               
            } else {
               
                setSelected(false);
            }
            mainDragHandleIndex = -1;
        }
    }
                
        
    
    bool hit = hitTest(mousePos);
    if(hit) {
        if((!getSelected()) &&(!getDisabled())) {
            setSelected(true);
            mainDragHandleIndex = -1;
            return false; // this way it doesn't scroll the page even if it's locked
        } else {
            return isDisabled; //  don't propagate unless it's disabled
        }
    } else if(handlehit){
        return false; // don't propagate;
    } else {
        selected = false;
        mainDragHandleIndex = -1;
        return true; // propagate
    }
}
       


void MoveablePoly ::startDraggingHandleByIndex(int index) {
    
    if((index<0) || (index>=handles.size())) {
        ofLogError("MoveablePoly ::startDraggingHandleByIndex - drag handle index out of range!");
        return ;
    }
       
    handles[index].startDrag(mousePos);
    mainDragHandleIndex = index;
}

void MoveablePoly :: mouseDragged(ofMouseEventArgs &e){
    
    if(isDisabled) return;
    mousePos = e;

    int dragCount = 0;
    for(size_t i= 0; i<handles.size(); i++) {
        if(handles[i].updateDrag(mousePos)) dragCount++;
    }
    
    //if(dragCount>0) ofLogNotice("dragging! count : ") << dragCount;
    isDirty |= (dragCount>0);

}


void MoveablePoly :: mouseMoved(ofMouseEventArgs &e){
    mousePos = e;
}
void MoveablePoly :: mouseReleased(ofMouseEventArgs &e){
    
    bool wasDragging = false;
    
    for(size_t i= 0; i<handles.size(); i++) {
        if(handles[i].stopDrag()) wasDragging = true;
    }
    // for global drag
    isDragging = false;
    //if(wasDragging) ofLogNotice("Stop dragging moveable poly");
    isDirty|=wasDragging;
}


bool MoveablePoly :: hitTest(glm::vec2& p)  {
    return hitTest(p.x, p.y);
    
    
}
bool MoveablePoly :: hitTest(float x, float y)  {
    return poly.hitTestEdges(x, y, 4/scale);
}

void MoveablePoly :: updatePoly() {
    
    centre = glm::vec2(0,0);
    for(DragHandle& handle : handles) {
        centre+=glm::vec2(handle);
    }
    centre/=4;
    poly.setFromPoints(getPoints());
    poly.update();
}

bool MoveablePoly :: setFromPoints(vector<glm::vec2>* points) {
    if(points->size()<2) return false;
    bool pointschanged = false;
    
    handles.resize(points->size());
    for(size_t i = 0; i<points->size(); i++) {
        glm::vec2 originalpoint = handles[i];

        if(originalpoint!=points->at(i)) {
            handles[i].set(points->at(i));
            pointschanged = true;
        }
    }
    
    if(pointschanged) {
        updateHandleColours();
        updatePoly();
        return true;
    } else {
        return false;
    }
    
}
bool MoveablePoly :: setFromPoints(vector<glm::vec2>& pointsref) {
    vector<glm::vec2>* points = &pointsref;
    return setFromPoints(points);
    
}

bool MoveablePoly :: setFromRect(ofRectangle rect) {
    
    bool changed = false;
    if(handles.size()!=4) {
        handles.resize(4);
        changed = true;
    }
    vector<glm::vec2> points = {rect.getTopLeft(), rect.getTopRight(), rect.getBottomRight(), rect.getBottomLeft()};
    
    return setFromPoints(points) | changed;
    
}
vector<glm::vec2*> MoveablePoly :: getPoints() {
    
    vector<glm::vec2*> returnpoints;
    for(DragHandle& handle : handles) {
        returnpoints.push_back(&handle);
        
    }
    return returnpoints; 
}
ofRectangle MoveablePoly :: getBoundingBox() {
    
    return poly.getBoundingBox();
    
}
void MoveablePoly :: drawShape() {
    
    ofBeginShape();
    for(DragHandle& handle : handles) {
        ofVertex(handle);
    }
    
    ofEndShape(true);
    
    
    
}
bool MoveablePoly :: isQuad() {
    return handles.size()==4;
    
}

bool MoveablePoly :: getRightClickPressed(bool reset) {
    bool returnvalue = rightClickPressed;
    if(reset) rightClickPressed = false;
    return returnvalue;
}
 

void MoveablePoly :: setRightClickPressed(bool value) {
    rightClickPressed = value;
}
void MoveablePoly :: setLabel(string newlabel) {
    label = newlabel;
} 

//
//  ofxLaserMoveablePoly.cpp
//  example_HelloLaser
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
    
    fillColour  = ofColor::fromHex(0x001123, 128);
    fillColourSelected = ofColor::fromHex(0x001123);
    strokeColour  = ofColor::fromHex(0x0E87E7);
    strokeColourSelected = ofColor::fromHex(0x0E87E7);
    handleColour = ofColor::fromHex(0x0E87E7);
    handleColourOver = ofColor :: fromHex(0xffffff);
    
}


bool MoveablePoly :: update() {
    if(isDirty) {
        updateMeshAndPoly();
    }
    bool wasDirty = isDirty;
    isDirty = false;
    return wasDirty;
}


void MoveablePoly :: draw() {
    
    ofPushStyle();
    
    ofSetColor(selected?fillColourSelected : fillColour);
    ofFill();
    
    zoneMesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
    zoneMesh.draw();

    ofNoFill();
    if(selected) {
        ofSetLineWidth(2);
    }
    ofSetColor(selected ? strokeColourSelected : strokeColour);
    
    zoneMesh.setMode(OF_PRIMITIVE_LINE_LOOP);
    zoneMesh.draw();
    
    drawHandlesIfSelectedAndNotDisabled();
    //drawLabel();
    
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
   
    ofSetColor(strokeColour);
    string label; //  = getLabel();
    //if(getLocked()) label = label + " (locked)";
    ofDrawBitmapString(label, centre - glm::vec3(4.0f*label.size()/scale,-4.0f/scale, 0));
    
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

    fillColour.setHue(hue);
    fillColourSelected.setHue(hue);
    strokeColour.setHue(hue);
    strokeColourSelected.setHue(hue);
    handleColour.setHue(hue);
    handleColourOver.setHue(hue);
    
    updateHandleColours();
  
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
            
            int i = clickorder[index];
            
            DragHandle& currentHandle = handles[i];
            
            if(currentHandle.hitTest(mousePos)) {
                handlehit = true;
                startDraggingHandleByIndex(i);
                
                // if we're not distorted then also start dragging the relavent corners
//                if(isSquare()) {
//                    DragHandle& anchorHandle = corners[(i+2)%4];
//                    DragHandle& dragHandle1 = corners[(i+1)%4];
//                    DragHandle& dragHandle2 = corners[(i+3)%4];
//
//                    dragHandle1.startDragProportional(mousePos, anchorHandle, currentHandle, true);
//                    dragHandle2.startDragProportional(mousePos, anchorHandle, currentHandle, true);
//
//                    constrainedToSquare =true;
//                } else {
//                    constrainedToSquare =false;
//                }
            }
        }
        
        if(!handlehit) {
            // if we haven't hit a handle but we have
            // hit the zone, start dragging the zone
            if(hitTest(mousePos)) {
                
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
            return isDisabled; //  don't propogate unless it's disabled
        }
    } else if(handlehit){
        return false; // don't propogate;
    } else {
        selected = false;
        mainDragHandleIndex = -1;
        return true; // propogate
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
    
    isDirty|=wasDragging;
}


bool MoveablePoly :: hitTest(ofPoint mousePoint)  {
   
    return zonePoly.inside(mousePoint);
    
}

void MoveablePoly :: updateMeshAndPoly() {
    
    centre = glm::vec2(0,0);
    for(DragHandle& handle : handles) {
        centre+=glm::vec2(handle);
    }
    centre/=4;
    
    zoneMesh.clear();
    zoneMesh.addVertex(glm::vec3(handles[0],0));
    zoneMesh.addVertex(glm::vec3(handles[1],0));
    zoneMesh.addVertex(glm::vec3(handles[2],0));
    zoneMesh.addVertex(glm::vec3(handles[3],0));
    
    zonePoly.clear();
    zonePoly.addVertex(glm::vec3(handles[0],0));
    zonePoly.addVertex(glm::vec3(handles[1],0));
    zonePoly.addVertex(glm::vec3(handles[2],0));
    zonePoly.addVertex(glm::vec3(handles[3],0));
    zonePoly.setClosed(true);

}

bool MoveablePoly :: setFromPoints(vector<glm::vec2>& points) {
    if(points.size()<2) return false;
    
    return true; 
    
}


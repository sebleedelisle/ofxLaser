//
//  ofxLaserZoneUIQuad.cpp
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUIQuad.h"

using namespace ofxLaser;


ZoneUiQuad :: ZoneUiQuad() {
    handles.resize(4);
}

bool ZoneUiQuad :: update() {
    
    bool wasDirty = ZoneUiBase::update();
    
    return wasDirty;
    
}

void ZoneUiQuad :: draw() {
    
    ofPushStyle();
    
    ofSetColor(selected?uiZoneFillColourSelected : uiZoneFillColour);
    ofFill();
    zoneMesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
    zoneMesh.draw();

    ofNoFill();
    if(selected&&!locked) {
        ofSetLineWidth(2);
    }
    ofSetColor((selected&&!locked) ? uiZoneStrokeColourSelected : uiZoneStrokeColour);
    zoneMesh.setMode(OF_PRIMITIVE_LINE_LOOP);
    zoneMesh.draw();
    
    drawHandlesIfSelected();
    drawLabel();
    
    ofPopStyle();

}



void ZoneUiQuad :: updateMeshAndPoly() {
    
    centre = glm::vec2(0,0);
    for(DragHandle& handle : handles) {
        centre+=glm::vec2(handle);
    }
    centre/=4;
    
    zoneMesh.clear();
    zoneMesh.addVertex(glm::vec3(handles[0],0));
    zoneMesh.addVertex(glm::vec3(handles[1],0));
    zoneMesh.addVertex(glm::vec3(handles[3],0));
    zoneMesh.addVertex(glm::vec3(handles[2],0));
    
    zonePoly.clear();
    zonePoly.addVertex(glm::vec3(handles[0],0));
    zonePoly.addVertex(glm::vec3(handles[1],0));
    zonePoly.addVertex(glm::vec3(handles[3],0));
    zonePoly.addVertex(glm::vec3(handles[2],0));
    zonePoly.setClosed(true);

}

vector<DragHandle*> ZoneUiQuad :: getCornersClockwise() {

    vector<DragHandle*> corners;
    corners.push_back(&handles[0]);
    corners.push_back(&handles[1]);
    corners.push_back(&handles[3]);
    corners.push_back(&handles[2]);

    return corners;
    
    
}


// TODO maybe we can handle selection at a higher level?
bool ZoneUiQuad :: mousePressed(ofMouseEventArgs &e) {
    mousePos = e;
    
     // zoneUiQuad needs to receieve a mousePressed event to handle
     // logic.
     // If the quad is selected, it'll need to check a hit with all it's dragpoints.
     // if one is hit then it starts the dragging process.
     // if none of the points are hit, a check for the shape is
     // executed. If true, then it starts dragging the whole shape.
     // If none of that is true and it's selected, deselect.
    
    vector<DragHandle*> corners = getCornersClockwise();
    bool handlehit = false;
    
    if(getSelected() && !locked) {
        
        // then check all the drag points
        
        for (int i = 0; i<corners.size() && !handlehit; i++) {
            
            DragHandle& currentHandle = *corners[i];
            
            if(currentHandle.hitTest(mousePos)) {
                handlehit = true;
                currentHandle.startDrag(mousePos);
                // if we're not distorted then also start dragging the relavent corners
                if(true) { // isSquare()) {
                    DragHandle& anchorHandle = *corners[(i+2)%4];
                    DragHandle& dragHandle1 = *corners[(i+1)%4];
                    DragHandle& dragHandle2 = *corners[(i+3)%4];
                    
                    dragHandle1.startDragProportional(mousePos, anchorHandle, currentHandle, true);
                    dragHandle2.startDragProportional(mousePos, anchorHandle, currentHandle, true);
                    
                    
                }
            }
        }
        
        if(!handlehit) {
            // if we haven't hit a handle but we have
            // hit the zone, start dragging the zone
            if(hitTest(mousePos)) {
                DragHandle* gridHandle = corners[0];
                for(size_t i= 0; i<corners.size(); i++) {
                    if(corners[i] == gridHandle) {
                        corners[i]->startDrag(mousePos);
                    } else {
                        corners[i]->startDrag(mousePos, gridHandle);
                    }
                }
            } else {
                setSelected(false);
            }
             
        }
    }
                
        
    
    bool hit = hitTest(mousePos);
    if(hit) {
        if(!selected) {
            selected = true;
            return false; // this way it doesn't scroll the page even if it's locked
        } else {
            return locked; //  don't propogate unless it's locked
        }
    } else if(handlehit){
        return false; // don't propogate;
    } else {
        selected = false;
        return true; // propogate
    }
}
       
                
void ZoneUiQuad :: mouseDragged(ofMouseEventArgs &e){
    
    if(locked) return; 
    mousePos = e;
//    mousePoint.x = e.x;
//    mousePoint.y = e.y;
//    mousePoint-=offset;
//    mousePoint/=scale;
    
    //ofRectangle bounds(centreHandle, 0, 0);
    int dragCount = 0;
    for(size_t i= 0; i<handles.size(); i++) {
        if(handles[i].updateDrag(mousePos)) dragCount++;
    }

    // do we need to track dirtyness? Probably.
    isDirty |= (dragCount>0);
    
}
void ZoneUiQuad :: mouseReleased(ofMouseEventArgs &e){
    
    bool wasDragging = false;
    
    for(size_t i= 0; i<handles.size(); i++) {
        if(handles[i].stopDrag()) wasDragging = true;
    }

    isDirty|=wasDragging;
}


bool ZoneUiQuad :: hitTest(ofPoint mousePoint)  {
   
    return zonePoly.inside(mousePoint);
    
}


bool ZoneUiQuad :: isSquare() {
    
    vector<DragHandle>& corners = handles;
    return (corners[0].x == corners[2].x) && (corners[0].y == corners[1].y) && (corners[1].x == corners[3].x) && (corners[2].y == corners[3].y);
    
}


bool ZoneUiQuad :: setCorners(const vector<glm::vec2*>& points) {
    
    if(points.size()<4) return false;
    
    for(int i = 0; i<4; i++) {
        handles[i].set(*points[i]);
       
    }
    
    updateHandleColours();
    updateMeshAndPoly();
    return true;
    
}



bool ZoneUiQuad :: setSelected(bool v) {
    if(ZoneUiBase::setSelected(v)) {
        // if state has changed
        for(DragHandle& handle : handles) {
            handle.stopDrag();
            // TODO - maybe make handle active / inactive?
        }
        
        return true;
    } else {
        return false;
    }
};

bool ZoneUiQuad :: updateFromOutputZone(OutputZone* outputZone) {
    if(outputZone->transformType!=0) {
        ofLogError("ZoneUIQuad :: updateFromOutputZone - WRONG TRANSFORM TYPE!");
        return false;
    } else {
        inputZoneIndex = outputZone->getZoneIndex();
        inputZoneAlt = outputZone->getIsAlternate(); 
        setCorners(outputZone->zoneTransformQuad.getCornerPoints());
        isDirty = false;
        return true;
    }
    
    
}

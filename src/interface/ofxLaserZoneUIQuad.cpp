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
   // if(isSquare()) ofSetColor(ofColor::green);
    zoneMesh.setMode(OF_PRIMITIVE_LINE_LOOP);
    zoneMesh.draw();
    
    drawHandlesIfSelected();
    //drawLabel();
    
    ofPopStyle();

}
bool ZoneUiQuad ::  updateDataFromUI(ZoneTransformBase* zonetransform) {

    ZoneTransformQuadData* zoneQuad = dynamic_cast<ZoneTransformQuadData*>(zonetransform);
    if(zoneQuad==nullptr) {
        // major error occurred
        ofLogError("ZoneUiQuad passed wrong kind of zone transform base!");
        return false;
    } else {
        if(isDragging) {
            zoneQuad->setDstCorners(handles[0], handles[1], handles[2], handles[3]);
        } else if(mainDragHandleIndexClockwise>=0) {
            zoneQuad->moveHandle(mainDragHandleIndexClockwise, *getMainDragHandle(), constrainedToSquare && !ofGetKeyPressed(OF_KEY_ALT));
        }
        
        return true;
        
    }


}

void ZoneUiQuad :: drawHandlesIfSelected() {
    if(getSelected() && !locked) {
        for(DragHandle& handle : handles) {
            if(&handle == getMainDragHandle()) {
                handle.draw(true);
            } else {
                handle.draw(mousePos);
            }
        }
    }
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

    // click order so we have a sensible priority when handles overlap
    int clickorder[] = {2,1,3,0};

    
    if(getSelected() && !locked) {
        
        // then check all the drag points
        for (int index = 0; index<corners.size() && !handlehit; index++) {
            
            int i = clickorder[index];
            
            DragHandle& currentHandle = *corners[i];
            
            if(currentHandle.hitTest(mousePos)) {
                handlehit = true;
                currentHandle.startDrag(mousePos);
                
                mainDragHandleIndexClockwise = i;
                
                // if we're not distorted then also start dragging the relavent corners
                if(isSquare()) {
                    DragHandle& anchorHandle = *corners[(i+2)%4];
                    DragHandle& dragHandle1 = *corners[(i+1)%4];
                    DragHandle& dragHandle2 = *corners[(i+3)%4];
                    
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
                DragHandle* gridHandle = corners[0];
                for(size_t i= 0; i<corners.size(); i++) {
                    if(corners[i] == gridHandle) {
                        corners[i]->startDrag(mousePos);
                    } else {
                        corners[i]->startDrag(mousePos, gridHandle);
                    }
                }
                isDragging = true;
               
            } else {
               
                setSelected(false);
            }
            mainDragHandleIndexClockwise = -1;
        }
    }
                
        
    
    bool hit = hitTest(mousePos);
    if(hit) {
        if(!selected) {
            selected = true;
            mainDragHandleIndexClockwise = -1;
            return false; // this way it doesn't scroll the page even if it's locked
        } else {
            return locked; //  don't propogate unless it's locked
        }
    } else if(handlehit){
        return false; // don't propogate;
    } else {
        selected = false;
        mainDragHandleIndexClockwise = -1;
        return true; // propogate
    }
}
       
DragHandle* ZoneUiQuad :: getMainDragHandle() {
    if(mainDragHandleIndexClockwise<0) {
        return nullptr;
    } else {
        return getCornersClockwise()[mainDragHandleIndexClockwise];
    }
}

int ZoneUiQuad :: getMainDragHandleIndexClockwise() {
    return mainDragHandleIndexClockwise; 
//    if(mainDragHandleIndexClockwise<2) return mainDragHandleIndexClockwise;
//    else return 3-(mainDragHandleIndexClockwise%2);
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
    // for global drag
    isDragging = false;
    
    isDirty|=wasDragging;
}


bool ZoneUiQuad :: hitTest(ofPoint mousePoint)  {
   
    return zonePoly.inside(mousePoint);
    
}


bool ZoneUiQuad :: isSquare() {
    
    vector<DragHandle>& corners = handles;
    
    return (fabs(handles[0].x - handles[2].x)<0.01f) &&
        (fabs(handles[0].y - handles[1].y)<0.01f) &&
        (fabs(handles[1].x - handles[3].x)<0.01f) &&
        (fabs(handles[2].y - handles[3].y)<0.01f);
    

   // return (corners[0].x == corners[2].x) && (corners[0].y == corners[1].y) && (corners[1].x == corners[3].x) && (corners[2].y == corners[3].y);
    
}


bool ZoneUiQuad :: setCorners(const vector<glm::vec2*>& points) {
    
    if(points.size()<4) return false;
    bool pointschanged = false;
    
    for(int i = 0; i<4; i++) {
        
        glm::vec2 originalpoint = handles[i];
        
        if(originalpoint!=*points[i]) {
            handles[i].set(*points[i]);
            pointschanged = true;
        }
    }
    
    if(pointschanged) {
        updateHandleColours();
        updateMeshAndPoly();
        return true;
    } else {
        return false;
    }
    
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

bool ZoneUiQuad :: updateFromData(ZoneTransformBase* zonetransform) {
    
    ZoneTransformQuadData* data = dynamic_cast<ZoneTransformQuadData*>(zonetransform);
    
    if(data!=nullptr) {
        
        
        return setCorners(data->getCornerPoints());
        
        
        
        
    } else {
        return false;
    }
    
    //
//
//
//
//    if(outputZone->transformType!=0) {
//        ofLogError("ZoneUIQuad :: updateFromOutputZone - WRONG TRANSFORM TYPE!");
//        return false;
//    } else {
//        inputZoneIndex = outputZone->getZoneIndex();
//        inputZoneAlt = outputZone->getIsAlternate();
//        setCorners(outputZone->zoneTransformQuad.getCornerPoints());
//        isDirty = false;
//        return true;
//    }
    
    
}

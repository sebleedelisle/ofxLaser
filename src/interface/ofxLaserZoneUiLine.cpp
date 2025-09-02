//
//  ofxLaserZoneUiLine.cpp
//
//
//  Created by Seb Lee-Delisle on 06/02/2023.
//
//

#include "ofxLaserZoneUiLine.h"

using namespace ofxLaser;


ZoneUiLine :: ZoneUiLine() {
    
}


bool ZoneUiLine ::  updateDataFromUi(std::shared_ptr<OutputZone>& outputZone) {
    //ofLogNotice("ZoneUiLine :: updateDataFromUi");
    bool changed = ZoneUiBase :: updateDataFromUi(outputZone);
    

    ZoneTransformLineData* zoneLine = dynamic_cast<ZoneTransformLineData*>(&outputZone->getZoneTransform());
    if(zoneLine==nullptr) {
        // major error occurred
        ofLogError("ZoneUiLine passed wrong kind of zone transform base!");
        return changed;
    } else {
        
       // if(isDragging)
        if(mainDragHandleIndex>=0) {
            //ofLogNotice("Dragging!");
            zoneLine->moveHandle(mainDragHandleIndex, *getMainDragHandle());
            changed = true;
        } else {
            
            if(vertexToDelete>-1) {
                //ofLogNotice("ZoneUiLine :: updateDataFromUi - Deleting node at ") << vertexToDelete;
                //handles.erase(handles.begin()+(vertexToDelete*3), handles.begin()+((vertexToDelete*3)+3));
                zoneLine->deleteNode(vertexToDelete);
                vertexToDelete = -1;
            } else if(pointsToAdd.size()>0) {
                
                while(pointsToAdd.size()>0) {

                    glm::vec2 point = pointsToAdd.back();
                    //ofLogNotice("ZoneUiLine :: updateDataFromUi - Adding node at ") << point.x << ", " << point.y;
                    zoneLine->addNode(point);
                    pointsToAdd.pop_back();
                    
                }
                
            } else {
                //
                zoneLine->setFromPoints(getPoints());
            }
//            zoneLine->update();
//            vector<glm::vec2> points;
//            zoneLine->getPerimeterPoints(points);
//            outlinePoly.setFromPoints(points);
//            
//            if(outlinePoly.update()) {
//                linePoly = zoneLine->getLinePoly();
//            }
//            
            changed = true;
        }
        
        return changed;
        
    }

}


void ZoneUiLine :: draw() {
    
    ofPushStyle();
    
   
    
    ofColor fColour = selected ? fillColourSelected : fillColour;
    if(dimmed) fColour*=0.5;
    ofSetColor(fColour);
    
    ofFill();
    if(!isDisabled) {
        ofBeginShape();
        for(glm::vec2& p : outlinePoly) {
            ofVertex(p);
        }
        ofEndShape(true);
    }
    
    
    // Line width not working on macos right now
    ofNoFill();
    if(selected) {
        ofSetLineWidth(2);
    }
    //ofSetLineWidth(10*GlobalScale::getScale());

    ofColor sColour = selected ? strokeColourSelected : strokeColour;
    if(dimmed) sColour*=0.5;
    else if(isDisabled) sColour*=0.7;
    ofSetColor(sColour);
    
    // draw centre line
    ofBeginShape();
    ofVertex(handles[0]);
    for(int i = 3; i<handles.size(); i+=3) {
        ofBezierVertex(handles[i-1], handles[i+1], handles[i]);
    }
    ofEndShape();
   
    
    ofNoFill();
    // draw bezier handles
    if(selected && !isDisabled) {
        
        if(!autoSmooth) {
            
            for(int i = 0; i<handles.size(); i+=3) {
                if(i>0) ofDrawLine(handles[i+1], handles[i]);
                if(i<handles.size()-3) ofDrawLine(handles[i+2], handles[i]);
                
            }
        }
    }
    

    
    
    
    ofSetColor(sColour); // selected ? strokeColourSelected : strokeColour);
    // outline of shape
    ofSetLineWidth(1);
    ofBeginShape();
    for(glm::vec2& p : outlinePoly) {
        ofVertex(p);
    }
    ofEndShape(true);
    
    ofPopStyle();
    
    drawHandlesIfSelectedAndNotDisabled();
    
    
}
glm::vec2 ZoneUiLine :: getClosestPointOnLine(glm::vec2 pos) {
    
    return linePoly.getClosestPoint(glm::vec3(pos.x, pos.y, 0));
}

int ZoneUiLine ::getClosestPointIndexToPosition(glm::vec2 pos) {
   
    //auto& points = linePoly.getVertices();
    
    float closestDistance = std::numeric_limits<float>::max();
    int index = 0;
    for(int i = 0; i<handles.size(); i+=3) {
        auto& point = handles.at(i);
        float dist = glm::distance(point, pos);
        if(dist < closestDistance) {
            index = i;
            closestDistance = dist;
        }
        
    }
    return index/3;
}



bool ZoneUiLine :: updateFromData(std::shared_ptr<OutputZone>& outputZone) {
    
    
    
    bool changed = ZoneUiBase::updateFromData(outputZone);

    ZoneTransformLineData* zonelinedata = dynamic_cast<ZoneTransformLineData*>(&outputZone->getZoneTransform());
    autoSmooth = zonelinedata->autoSmooth;
    
    if(zonelinedata!=nullptr) {
        
        vector<BezierNode>& nodes = zonelinedata->getNodes();
        
        if(handles.size()!=nodes.size()*3) setNumHandles(nodes.size()*3);
        //if(controlPoints.size()!=nodes.size()*2) controlPoints.resize(nodes.size()*2);
        
        for(int i = 0; i<nodes.size(); i++) {
            BezierNode& node = nodes[i];
            
            int handleindex = i*3;
            handles[handleindex].set(node.getPosition());
            handles[handleindex].isCircular = false;
            handles[handleindex].setGrid(snapToGrid, gridSize);
            
            // control points
            handles[handleindex+1].set(node.getControlPoint1());
            handles[handleindex+1].isCircular = true;
            handles[handleindex+1].setGrid(false, gridSize); // don't snap control points!
            handles[handleindex+2].set(node.getControlPoint2());
            handles[handleindex+2].isCircular = true;
            handles[handleindex+2].setGrid(false, gridSize); // don't snap control points!
            if(zonelinedata->autoSmooth) {
                handles[handleindex+1].active = false;
                handles[handleindex+2].active = false;
            } else {
                handles[handleindex+1].active = true;
                handles[handleindex+2].active = true;
            }
            // hide unused control points at the ends
            if(i==0) {
                handles[handleindex+1].active = false;
            } else if(i == nodes.size()-1) {
                handles[handleindex+2].active = false;
            }
            
        }
        
        
        
        vector<glm::vec2> points;
        zonelinedata->getPerimeterPoints(points);
        outlinePoly.setFromPoints(points);
        if(outlinePoly.update()) {
            linePoly = zonelinedata->getLinePoly();
        }

        updateHandleColours();
        
        return true;
        
    } else {
        return changed;
    }
    
 
    
    
}
void ZoneUiLine :: updatePoly() {
    
    centre = glm::vec2(0,0);
    for(int i = 0; i<handles.size(); i++) {
//    for(DragHandle& handle : handles) {
        DragHandle& handle = handles[i];
        centre+=glm::vec2(handle);
    }
    centre/=handles.size();

}

bool ZoneUiLine :: mousePressed(ofMouseEventArgs &e) {
    
    bool propagate = MoveablePoly::mousePressed(e);
//    if(mainDragHandleIndex>=0) {
//
//        int pointtype = mainDragHandleIndex%3;
//        if(pointtype==0) {
//            // main anchor, so also drag control points
//            handles[mainDragHandleIndex+1].startDrag(mousePos,handles[mainDragHandleIndex]);
//            handles[mainDragHandleIndex+2].startDrag(mousePos,handles[mainDragHandleIndex]);
//
//        } else if(pointtype ==1) {
//            if(mode==2) handles[2].startDragSymmetrical(mousePos, handles[0], handles[1], false);
//        } else if(pointtype ==2) {
//
//        }
//    }
    return propagate;
}

int ZoneUiLine :: getNumPoints() {
    return handles.size()/3;
    
}


glm::vec2 ZoneUiLine :: getPointAtIndex(int index) {
    
    if((index>=0) && (index<handles.size()/3)) {
        return handles.at(index*3);
    } else {
        return {0,0};
    }
    
}


bool ZoneUiLine :: deleteVertex(int vertexindex){

    vertexToDelete = vertexindex;

    // make sure no nodes are being dragged! Fixes a bug where adding / deleting nodes wouldn't
    // work if a node was selected.
    mainDragHandleIndex = -1;
    setDirty();
    return true;
}
void ZoneUiLine :: addVertex(glm::vec2 point) {
    
    pointsToAdd.push_back(point);
    // make sure no nodes are being dragged! Fixes a bug where adding / deleting nodes wouldn't
    // work if a node was selected.
    mainDragHandleIndex = -1;
    setDirty();
    
}


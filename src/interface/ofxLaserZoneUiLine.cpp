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


bool ZoneUiLine ::  updateDataFromUi(OutputZone* outputZone) {
    
    bool changed = ZoneUiBase :: updateDataFromUi(outputZone);
    

    ZoneTransformLineData* zoneLine = dynamic_cast<ZoneTransformLineData*>(&outputZone->getZoneTransform());
    if(zoneLine==nullptr) {
        // major error occurred
        ofLogError("ZoneUiLine passed wrong kind of zone transform base!");
        return changed;
    } else {
        
        if(isDragging) {
            zoneLine->setFromPoints(getPoints());
            changed = true;
        } else if(mainDragHandleIndex>=0) {
            zoneLine->moveHandle(mainDragHandleIndex, *getMainDragHandle());
            changed = true;
        }
        
        return changed;
        
    }

}


void ZoneUiLine :: draw() {
    
    ofPushStyle();
    
    ofSetColor(selected?fillColourSelected : fillColour);
    ofFill();
    ofBeginShape();
    for(glm::vec2& p : poly) {
        ofVertex(p);
    }
    ofEndShape(true); 
    
    
    ofNoFill();
    if(selected) {
        ofSetLineWidth(2);
    }
    ofSetColor(selected ? strokeColourSelected : strokeColour);
    
    if(selected) {
        ofBeginShape();
        ofVertex(handles[0]);
        for(int i = 3; i<handles.size(); i+=3) {
            ofBezierVertex(handles[i-1], handles[i+1], handles[i]);
            //ofVertex(handles[i]);
        }
        ofEndShape();
        
        if(!autoSmooth) {
                
            for(int i = 0; i<handles.size(); i+=3) {
                if(i>0) ofDrawLine(handles[i+1], handles[i]);
                if(i<handles.size()-3) ofDrawLine(handles[i+2], handles[i]);

            }
        }
        
        
    }
    // outline of shape
    ofSetLineWidth(1);
    ofBeginShape();
    for(glm::vec2& p : poly) {
        ofVertex(p);
    }
    ofEndShape(true);
    
    drawHandlesIfSelectedAndNotDisabled();
    //drawLabel();
    
    ofPopStyle();
    
}



bool ZoneUiLine :: updateFromData(OutputZone* outputZone) {
    
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
            
            
            // control points
            handles[handleindex+1].set(node.getControlPoint1());
            handles[handleindex+1].isCircular = true;
            handles[handleindex+2].set(node.getControlPoint2());
            handles[handleindex+2].isCircular = true;
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
        poly.setFromPoints(points);
        poly.update();

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

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
        for(int i = 0; i<handles.size(); i++) {
            //ofBezierVertex(handles[i-1], handles[i+1], handles[i]);
            ofVertex(handles[i]);
        }
        ofEndShape();
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
    
    if(zonelinedata!=nullptr) {
        
        vector<BezierNode>& nodes = zonelinedata->getNodes();
        
        if(handles.size()!=nodes.size()) handles.resize(nodes.size());
        
        for(int i = 0; i<nodes.size(); i++) {
            BezierNode& node = nodes[i];
            //int handleindex = i*3;
            handles[i].set(node.getPosition());
            handles[i].isCircular = false;
//            handles[handleindex+1].set(node.getControlPoint1());
//            handles[handleindex+1].isCircular = true;
//            handles[handleindex+2].set(node.getControlPoint2());
//            handles[handleindex+2].isCircular = true;
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

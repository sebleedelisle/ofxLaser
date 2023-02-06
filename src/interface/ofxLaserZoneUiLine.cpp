//
//  ofxLaserZoneUiLine.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 06/02/2023.
//
//

#include "ofxLaserZoneUiLine.h"

using namespace ofxLaser;


ZoneUiLine :: ZoneUiLine() {
    //handles.resize(4);
}


bool ZoneUiLine ::  updateDataFromUI(ZoneTransformBase* zonetransform) {

    ZoneTransformLineData* zoneLine = dynamic_cast<ZoneTransformLineData*>(zonetransform);
    if(zoneLine==nullptr) {
        // major error occurred
        ofLogError("ZoneUiLine passed wrong kind of zone transform base!");
        return false;
    } else {
        
        
        
        
        // TODO UPDATE DATA!!!
        if(isDragging) {
            zoneLine->setFromPoints(getPoints());
        } else if(mainDragHandleIndex>=0) {
            zoneLine->moveHandle(mainDragHandleIndex, *getMainDragHandle());
        }
        
        return true;
        
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
    } else {
        ofBeginShape();
        for(glm::vec2& p : poly) {
            ofVertex(p);
        }
        ofEndShape(true); 
        
    }
    
    drawHandlesIfSelectedAndNotDisabled();
    
    ofPopStyle();
    
}



bool ZoneUiLine :: updateFromData(ZoneTransformBase* zonetransform) {
    
    ZoneTransformLineData* zonelinedata = dynamic_cast<ZoneTransformLineData*>(zonetransform);
    
    if(zonelinedata!=nullptr) {
        
        setDisabled(zonelinedata->locked);
        // TODO update handles from data! 
        
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

        return true;
        
    } else {
        return false;
    }
    
 
    
    
}
void ZoneUiLine :: updatePoly() {
    
    centre = glm::vec2(0,0);
    for(DragHandle& handle : handles) {
        centre+=glm::vec2(handle);
    }
    centre/=4;
//    poly.setFromPoints(getPoints());
//    poly.update();
}

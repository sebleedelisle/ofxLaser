//
//  ofxLaserZoneUiQuadComplex.cpp
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUiQuadComplex.h"

using namespace ofxLaser;


ZoneUiQuadComplex :: ZoneUiQuadComplex() {
   
    handleSize = 10;
    setNumHandles(4);
   
}


bool ZoneUiQuadComplex ::  updateDataFromUi(OutputZone* outputZone) {
    bool changed = ZoneUiBase::updateDataFromUi(outputZone);
    
    ZoneTransformQuadComplexData* zoneQuad = dynamic_cast<ZoneTransformQuadComplexData*>(&outputZone->getZoneTransform());
    if(zoneQuad==nullptr) {
        // major error occurred
        ofLogError("ZoneUiQuadComplex passed wrong kind of zone transform base!");
        return changed;
    } else {
        if(isDragging) {
            zoneQuad->drag(dragOffset);
            //zoneQuad->setDstCorners(handles[0], handles[1], handles[3], handles[2]);
        } else if(mainDragHandleIndex>=0) {
            zoneQuad->moveHandle(mainDragHandleIndex, *getMainDragHandle(), constrainedToSquare && !ofGetKeyPressed(OF_KEY_ALT));
        }
        return true;
    }
}


bool ZoneUiQuadComplex :: setCorners(const vector<glm::vec2*>& newpoints) {
    
    
    if(newpoints.size()<4) return false;
    vector<glm::vec2> temppoints;
    
    for(int i = 0; i<4; i++) {

        int handleindex = i;
        if(i>1) handleindex = 3 - (i%2); // convert to clockwise points
        temppoints.push_back(*newpoints[handleindex]);
        
    }
    return setFromPoints(temppoints);
}


bool ZoneUiQuadComplex :: updateFromData(OutputZone* outputZone) {
    
    bool changed = ZoneUiBase::updateFromData(outputZone);
    
    ZoneTransformQuadComplexData* zonetransform = dynamic_cast<ZoneTransformQuadComplexData*>(&outputZone->getZoneTransform());
    
    if(zonetransform!=nullptr) {
        
//        perimeterPolyline.clear();
//        vector<glm::vec2> perimeterpoints;
//        zonetransform->getPerimeterPoints(perimeterpoints);
//        for(glm::vec2 p : perimeterpoints) {
//            perimeterPolyline.addVertex(p.x, p.y, 0);
//        }
        poly.setFromPoints(zonetransform->getPerimeterPoints());
        
        setNumHandles(zonetransform->getNumPoints());
        for(int i = 0; i<handles.size(); i++) {
            glm::vec2 dstpoint =zonetransform->getDestPointAt(i);
            if(dstpoint!=handles[i]) {
                handles[i].set(dstpoint);
               
                
                changed = true;
            }
        }
        if(changed) updateHandleColours(); 
        
        subdivisionsX = subdivisionsY = zonetransform->getNumSubdivisions();
        
//        changed |= setCorners(zonetransform->getCornerPoints());
//        if(GeomUtils::isConvex(getPoints())) setHue(0);
//        else (setHue(130));
        
        return changed;
        
    } else {
        return false;
    }

}


void ZoneUiQuadComplex :: draw() {
    
    
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
    
    
    
    // draw subdividers
    ofNoFill();
    ofSetColor(strokeColour*0.5);
    for(int y = 1; y<subdivisionsY; y++) {
        ofBeginShape();
        for(int x = 0; x<=subdivisionsX; x++) {
            ofVertex(getPointAtPosition(x,y));
        }
        ofEndShape();
        
    }
    
    for(int x = 1; x<subdivisionsX; x++) {
        ofBeginShape();
        for(int y = 0; y<=subdivisionsY; y++) {
            ofVertex(getPointAtPosition(x,y));
        }
        ofEndShape();
        
    }
    
    ofPopStyle();
    
    drawHandlesIfSelectedAndNotDisabled();
   // drawLabel();
    
    
   
}


glm::vec2 ZoneUiQuadComplex :: getPointAtPosition(int x, int y)  {
    
    int index =   (y*(subdivisionsX+1)) +x;
    return handles[index];
    
    
}

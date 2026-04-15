//
//  ofxLaserZoneUIQuadComplex.cpp
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUIQuadComplex.h"

using namespace ofxLaser;


ZoneUIQuadComplex :: ZoneUIQuadComplex() {
   
    handleSize = 10;
    setNumHandles(4);
   
}


bool ZoneUIQuadComplex ::  updateDataFromUI(std::shared_ptr<OutputZone>& outputZone) {
    bool changed = ZoneUIBase::updateDataFromUI(outputZone);
    
    ZoneTransformQuadComplexData* zoneQuad = dynamic_cast<ZoneTransformQuadComplexData*>(&outputZone->getZoneTransform());
    if(zoneQuad==nullptr) {
        // major error occurred
        ofLogError("ZoneUIQuadComplex passed wrong kind of zone transform base!");
        return changed;
    } else {
        if(mainDragHandleIndex>=0) {
            zoneQuad->moveHandle(mainDragHandleIndex, *getMainDragHandle(), constrainedToSquare && !ofGetKeyPressed(OF_KEY_ALT));
            changed = true;
        } else {
            zoneQuad->setFromPoints(getPoints());
            changed = true;
        }
        
        return changed;
    }
}


bool ZoneUIQuadComplex :: updateFromData(std::shared_ptr<OutputZone>& outputZone) {
    
    bool changed = ZoneUIBase::updateFromData(outputZone);
    
    ZoneTransformQuadComplexData* zonetransform = dynamic_cast<ZoneTransformQuadComplexData*>(&outputZone->getZoneTransform());
    
    if(zonetransform!=nullptr) {
        
        outlinePoly.setFromPoints(zonetransform->getPerimeterPoints());
        
        setNumHandles(zonetransform->getNumPoints());
        for(int i = 0; i<handles.size(); i++) {
            glm::vec2 dstpoint =zonetransform->getDestPointAt(i);
            if(dstpoint!=handles[i]) {
                handles[i].set(dstpoint);
                handles[i].setGrid(snapToGrid, gridSize);
                
                changed = true;
            }
        }
        if(changed) updateHandleColours(); 
        
        subdivisionsX = subdivisionsY = zonetransform->getNumSubdivisions();
        
        return changed;
        
    } else {
        return false;
    }

}


void ZoneUIQuadComplex :: draw() {
    
    
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
    
    
   
}


glm::vec2 ZoneUIQuadComplex :: getPointAtPosition(int x, int y)  {
    
    int index =   (y*(subdivisionsX+1)) +x;
    return handles[index];
    
    
}

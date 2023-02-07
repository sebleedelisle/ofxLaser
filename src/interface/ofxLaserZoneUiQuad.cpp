//
//  ofxLaserZoneUiQuad.cpp
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUiQuad.h"

using namespace ofxLaser;


ZoneUiQuad :: ZoneUiQuad() {
    handles.resize(4);
}


bool ZoneUiQuad ::  updateDataFromUI(OutputZone* outputZone) {
    bool changed = ZoneUiBase::updateDataFromUI(outputZone);
    
    ZoneTransformQuadData* zoneQuad = dynamic_cast<ZoneTransformQuadData*>(&outputZone->getZoneTransform());
    if(zoneQuad==nullptr) {
        // major error occurred
        ofLogError("ZoneUiQuad passed wrong kind of zone transform base!");
        return changed;
    } else {
        if(isDragging) {
            zoneQuad->setDstCorners(handles[0], handles[1], handles[3], handles[2]);
        } else if(mainDragHandleIndex>=0) {
            zoneQuad->moveHandle(mainDragHandleIndex, *getMainDragHandle(), constrainedToSquare && !ofGetKeyPressed(OF_KEY_ALT));
        }
        return true;
    }
}


bool ZoneUiQuad :: setCorners(const vector<glm::vec2*>& newpoints) {
    
    
    if(newpoints.size()<4) return false;
    vector<glm::vec2> temppoints;
    
    for(int i = 0; i<4; i++) {

        int handleindex = i;
        if(i>1) handleindex = 3 - (i%2); // convert to clockwise points
        temppoints.push_back(*newpoints[handleindex]);
        
    }
    return setFromPoints(temppoints); 
    
}


bool ZoneUiQuad :: updateFromData(OutputZone* outputZone) {
    
    bool changed = ZoneUiBase::updateFromData(outputZone);
    
    ZoneTransformQuadData* zonetransform = dynamic_cast<ZoneTransformQuadData*>(&outputZone->getZoneTransform());
    
    if(zonetransform!=nullptr) {
        
        changed |= setCorners(zonetransform->getCornerPoints());
        
        return changed;
        
    } else {
        return false;
    }

}

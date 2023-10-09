//
//  ofxLaserZoneUiQuadComplex.cpp
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUiQuadComplex.h"

using namespace ofxLaser;


ZoneUiQuadComplex :: ZoneUiQuadComplex() {
    handles.resize(4);
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
            zoneQuad->setDstCorners(handles[0], handles[1], handles[3], handles[2]);
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
    
    ZoneTransformQuadData* zonetransform = dynamic_cast<ZoneTransformQuadData*>(&outputZone->getZoneTransform());
    
    if(zonetransform!=nullptr) {
        
        changed |= setCorners(zonetransform->getCornerPoints());
//        if(GeomUtils::isConvex(getPoints())) setHue(0);
//        else (setHue(130));
        
        return changed;
        
    } else {
        return false;
    }

}

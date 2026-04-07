//
//  ofxLaserZoneUIQuad.cpp
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUIQuad.h"

using namespace ofxLaser;


ZoneUIQuad :: ZoneUIQuad() {
    setNumHandles(4);
}


bool ZoneUIQuad ::  updateDataFromUI(std::shared_ptr<OutputZone>& outputZone) {
    bool changed = ZoneUIBase::updateDataFromUI(outputZone);
    
    ZoneTransformQuadData* zoneQuad = dynamic_cast<ZoneTransformQuadData*>(&outputZone->getZoneTransform());
    if(zoneQuad==nullptr) {
        // major error occurred
        ofLogError("ZoneUIQuad passed wrong kind of zone transform base!");
        return changed;
    } else {
        if(mainDragHandleIndex>=0) {
            zoneQuad->moveHandle(mainDragHandleIndex, *getMainDragHandle(), constrainedToSquare && !ofGetKeyPressed(OF_KEY_ALT));
            changed = true;
        } else {
            zoneQuad->setDstCorners(handles[0], handles[1], handles[3], handles[2]);
            changed = true;
        }
        return changed;
    }
}


bool ZoneUIQuad :: updateFromData(std::shared_ptr<OutputZone>& outputZone) {
    
    bool changed = ZoneUIBase::updateFromData(outputZone);
    
    ZoneTransformQuadData* zonetransform = dynamic_cast<ZoneTransformQuadData*>(&outputZone->getZoneTransform());
    
    if(zonetransform!=nullptr) {
        
        changed |= setCorners(zonetransform->getCornerPoints());
        return changed;
        
    } else {
        return false;
    }

}

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


bool ZoneUiQuad ::  updateDataFromUI(ZoneTransformBase* zonetransform) {

    ZoneTransformQuadData* zoneQuad = dynamic_cast<ZoneTransformQuadData*>(zonetransform);
    if(zoneQuad==nullptr) {
        // major error occurred
        ofLogError("ZoneUiQuad passed wrong kind of zone transform base!");
        return false;
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




bool ZoneUiQuad :: updateFromData(ZoneTransformBase* zonetransform) {
    
    ZoneTransformQuadData* data = dynamic_cast<ZoneTransformQuadData*>(zonetransform);
    
    if(data!=nullptr) {
        setDisabled(data->locked);
        bool changed = setCorners(data->getCornerPoints());
        //ofLogNotice("cross section") << GeomUtils::getMinimumCrossSectionWidth(poly);
        return changed;
        
    } else {
        return false;
    }

}

//
//  ofxLaserZoneUIQuad.cpp
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUIQuad.h"

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


//
//bool ZoneUiQuad :: isSquare() {
//    
//    vector<DragHandle>& corners = handles;
//    
//    return (fabs(handles[0].x - handles[3].x)<0.01f) &&
//        (fabs(handles[0].y - handles[1].y)<0.01f) &&
//        (fabs(handles[1].x - handles[2].x)<0.01f) &&
//        (fabs(handles[3].y - handles[2].y)<0.01f);
//    
//
//   // return (corners[0].x == corners[2].x) && (corners[0].y == corners[1].y) && (corners[1].x == corners[3].x) && (corners[2].y == corners[3].y);
//    
//}


bool ZoneUiQuad :: setCorners(const vector<glm::vec2*>& newpoints) {
    
    
    if(newpoints.size()<4) return false;
    vector<glm::vec2> temppoints;
    
    for(int i = 0; i<4; i++) {

        int handleindex = i;
        if(i>1) handleindex = 3 - (i%2); // convert to clockwise points
        temppoints.push_back(*newpoints[handleindex]);
        
    }
    return setFromPoints(temppoints); 
    
//
//    if(points.size()<4) return false;
//    bool pointschanged = false;
//
//    for(int i = 0; i<4; i++) {
//
//        int handleindex = i;
//        if(i>1) handleindex = 3 - (i%2); // convert to clockwise points
//        glm::vec2 originalpoint = handles[handleindex];
//
//        if(originalpoint!=*points[i]) {
//            handles[handleindex].set(*points[i]);
//            pointschanged = true;
//        }
//    }
//
//    if(pointschanged) {
//        updateHandleColours();
//        updateMeshAndPoly();
//        return true;
//    } else {
//        return false;
//    }
    
}




bool ZoneUiQuad :: updateFromData(ZoneTransformBase* zonetransform) {
    
    ZoneTransformQuadData* data = dynamic_cast<ZoneTransformQuadData*>(zonetransform);
    
    if(data!=nullptr) {
        
        setDisabled(data->locked);
        return setCorners(data->getCornerPoints());
        
        
        
        
    } else {
        return false;
    }
    
    //
//
//
//
//    if(outputZone->transformType!=0) {
//        ofLogError("ZoneUIQuad :: updateFromOutputZone - WRONG TRANSFORM TYPE!");
//        return false;
//    } else {
//        inputZoneIndex = outputZone->getZoneIndex();
//        inputZoneAlt = outputZone->getIsAlternate();
//        setCorners(outputZone->zoneTransformQuad.getCornerPoints());
//        isDirty = false;
//        return true;
//    }
    
    
}

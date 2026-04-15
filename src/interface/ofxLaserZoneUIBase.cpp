//
//  ofxLaserZoneUIBase.cpp
//
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUIBase.h"
using namespace ofxLaser;


bool ZoneUIBase::updateDataFromUI(std::shared_ptr<OutputZone>& outputZone) {
    bool changed = false;
    if(muted!=outputZone->muted) {
        outputZone->muted = muted ;
        changed = true;
    }
    if(locked!=outputZone->locked) {
        outputZone->locked = locked ;
        changed = true;
    }
    if(outputZone->getZoneId()!=zoneId) {
        outputZone->setZoneId(zoneId);
    }
    updateLabel();
    dimmed = muted;
    return changed;
}

bool ZoneUIBase::updateFromData(std::shared_ptr<OutputZone>& outputZone){
    bool changed = false;
    
    if(zoneId!= outputZone->getZoneId()) {
        zoneId= outputZone->getZoneId();
        changed = true;
    }
    
    if(locked!=outputZone->locked) {
        locked = outputZone->locked;
        changed = true;
    }
    if(getDisabled()!=locked) {
        setDisabled(locked);
        changed = true;
    }
    
    if(muted!=outputZone->muted.get()) {
        muted = outputZone->muted;
        changed = true;
        
    }
    
    
    
    if(changed) {
        updateLabel();
        dimmed = muted;
    }
    
    return changed;
}

void ZoneUIBase :: updateLabel() {
    
    label = ofToString(zoneId.getLabel());

}



bool ZoneUIBase :: setCorners(const vector<glm::vec2*>& newpoints) {
    if(newpoints.size()<4) return false;
    vector<glm::vec2> temppoints;
    for(int i = 0; i<4; i++) {
        int handleindex = i;
        if(i>1) handleindex = 3 - (i%2); // convert to clockwise points
        temppoints.push_back(*newpoints[handleindex]);
    }
    return setFromPoints(temppoints);
}

void ZoneUIBase :: drawLabel() {
    
    
    if(!showLabel) return;
    
    ofPushStyle();
   
    if(dimmed) {
        ofSetColor(strokeColour*0.5);
    } else {
        ofSetColor(strokeColour);
    }
    
    ofPushMatrix();
   
    
    string labeltemp = getLabel();
    if(muted) labeltemp = labeltemp + " (DISABLED)";
    if(locked) labeltemp = labeltemp + " (LOCKED)";
    ofDrawBitmapString(labeltemp, getCentre() - glm::vec3(4.0f*labeltemp.size()/scale,-4.0f/scale, 0));
    
    ofPopMatrix();
    ofPopStyle();
    
}

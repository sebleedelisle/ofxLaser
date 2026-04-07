//
//  ofxLaserZoneUIBase.cpp
//
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUIBase.h"
using namespace ofxLaser;


bool ZoneUIBase::updateDataFromUi(std::shared_ptr<OutputZone>& outputZone) {
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
    
    if(inputZoneAlt) {
        inputZoneAlt = false;
        setHue(140);
        updateHandleColours();
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
    if(inputZoneAlt) {
        label = label + " ALT";
    }
//    if(getDisabled()) {
//        label = label + " (locked)";
//    }
//    if(muted) {
//        label = label + " (muted)";
//    }
    
    
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
    if(muted) labeltemp = labeltemp + (" (DISABLED)");
    if(locked) labeltemp = labeltemp + (" (LOCKED)");
//    ofDrawCircle(getCentre()- glm::vec3(4.0f*label.size()/scale,-4.0f/scale, 0), 5);
//    ofTranslate(-ofGetMouseX()/scale, -ofGetMouseY()*scale);
//    ofLogNotice() << ofGetMouseX() << " " << ofGetMouseY();
    ofDrawBitmapString(labeltemp, getCentre() - glm::vec3(4.0f*labeltemp.size()/scale,-4.0f/scale, 0));
    
    ofPopMatrix();
    ofPopStyle();
    
}

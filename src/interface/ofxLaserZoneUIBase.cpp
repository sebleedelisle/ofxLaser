//
//  ofxLaserZoneUiBase.cpp
//
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUiBase.h"
using namespace ofxLaser;


bool ZoneUiBase::updateDataFromUi(OutputZone* outputZone) {
    bool changed = false;
    if(muted!=outputZone->muted) {
        outputZone->muted = muted ;
        changed = true;
    }
    updateLabel();
    dimmed = muted;
    return changed;
}

bool ZoneUiBase::updateFromData(OutputZone* outputZone){
    bool changed = false;
    
    if(zoneId!= outputZone->getZoneId()) {
        zoneId= outputZone->getZoneId();
        changed = true;
    }
    
    if(inputZoneAlt != outputZone->getIsAlternate()) {
        inputZoneAlt = outputZone->getIsAlternate();
        if(inputZoneAlt) {
            setHue(85);
            setSaturationFloat(0.8);
            setBrightness(180);
        } else {
            setHue(140);
        }
        updateHandleColours();
        changed = true;
    }
    
    if(getDisabled()!=outputZone->getZoneTransform().locked) {
        setDisabled(outputZone->getZoneTransform().locked);
        changed = true;
    }
    
    if(muted!=outputZone->muted) {
        muted = outputZone->muted;
        changed = true;
        
    }
    
    
    
    if(changed) {
        updateLabel();
        dimmed = muted;
    }
    
    return changed;
}

void ZoneUiBase :: updateLabel() {
    
    label = ofToString(zoneId.getLabel());
    if(inputZoneAlt) {
        label = label + " ALT";
    }
    if(getDisabled()) {
        label = label + " (locked)";
    }
    if(muted) {
        label = label + " (muted)";
    }
    
    
}

//
//  ofxLaserZoneUiBase.cpp
//
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUiBase.h"
using namespace ofxLaser;

//
//string ZoneUiBase::getLabel(){
//    string label = ofToString(inputZoneIndex+1);
//    if(inputZoneAlt) {
//        label = label + " ALT";
//    }
//    return label;
//
//}


bool ZoneUiBase::updateDataFromUI(OutputZone* outputZone) {
    bool changed = false;
    if(muted!=outputZone->muted) {
        outputZone->muted = muted ;
        changed = true;
        
    }
    
    return changed;
}

bool ZoneUiBase::updateFromData(OutputZone* outputZone){
    bool changed = false;
    
    if(inputZoneIndex!= outputZone->getZoneIndex()) {
        inputZoneIndex= outputZone->getZoneIndex();
        changed = true;
    }
    
    if(inputZoneAlt != outputZone->getIsAlternate()) {
        inputZoneAlt = outputZone->getIsAlternate();
        if(inputZoneAlt) setHue(85);
        else setHue(140);
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
        label = ofToString(inputZoneIndex+1);
        if(inputZoneAlt) {
            label = label + " ALT";
        }
        if(outputZone->getZoneTransform().locked) {
            label = label + " (locked)";
            
        }
    }
    
    return changed;
}

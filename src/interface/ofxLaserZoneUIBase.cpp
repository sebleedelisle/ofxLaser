//
//  ofxLaserZoneUiBase.cpp
//
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUiBase.h"
using namespace ofxLaser;


bool ZoneUiBase::updateDataFromUi(std::shared_ptr<OutputZone>& outputZone) {
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

bool ZoneUiBase::updateFromData(std::shared_ptr<OutputZone>& outputZone){
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

void ZoneUiBase :: updateLabel() {
    
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



void ZoneUiBase :: drawLabel() {
    
    
    if(!showLabel) return;
    
    ofPushStyle();
   
    if(dimmed) {
        ofSetColor(strokeColour*0.5);
    } else {
        ofSetColor(strokeColour);
    }
    
    ofPushMatrix();
   
    
#ifdef OFXLASER_USE_FONT_MANAGER
    
    ofTranslate(round(getCentre().x), round(getCentre().y));
    ofScale(1/scale, 1/scale);
    
    glm::vec3 pos;
    string iconlabel = "";
    if(muted) iconlabel = ofToString(ICON_FK_BAN);
    if(muted && getDisabled()) iconlabel += " ";
    if(getDisabled()) iconlabel += ofToString(ICON_FK_LOCK);
    if(iconlabel.size()>0) {
        pos.y-=ofxFontManager::getFontHeight("default")/2;
        ofxFontManager :: drawStringAsShapes(iconlabel, pos, ofxFontManager::CENTRE, ofxFontManager::MIDDLE, "symbol-large");
        pos.y+=ofxFontManager::getFontHeight("default");
    }
    ofFill();
    ofxFontManager :: drawStringAsShapes(getLabel(), pos, ofxFontManager::CENTRE, ofxFontManager::MIDDLE, "default");
    
   
            
#else
    string labeltemp = getLabel();
    if(muted) labeltemp = labeltemp + (" (DISABLED)");
    if(locked) labeltemp = labeltemp + (" (LOCKED)");
//    ofDrawCircle(getCentre()- glm::vec3(4.0f*label.size()/scale,-4.0f/scale, 0), 5);
//    ofTranslate(-ofGetMouseX()/scale, -ofGetMouseY()*scale);
//    ofLogNotice() << ofGetMouseX() << " " << ofGetMouseY();
    ofDrawBitmapString(labeltemp, getCentre() - glm::vec3(4.0f*labeltemp.size()/scale,-4.0f/scale, 0));
#endif
    
    ofPopMatrix();
    ofPopStyle();
    
}

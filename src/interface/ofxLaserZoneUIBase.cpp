//
//  ofxLaserZoneUIBase.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUIBase.h"
using namespace ofxLaser;


ZoneUiBase :: ZoneUiBase() {
   // locked = false;
}


string ZoneUiBase::getLabel(){
    string label = ofToString(inputZoneIndex+1);
    if(inputZoneAlt) {
        label = label + " ALT";
    }
    return label;
    
}

//
//void ZoneUiBase::setLocked(bool _locked){
//    locked = _locked;
//    if(locked) setSelected(false);
//}
//bool ZoneUiBase::getLocked(){
//    return locked;
//}
//

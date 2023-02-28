//
//  ofxLaserZoneId.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 25/02/2023.
//
//

#pragma once

#include "ofMain.h"

namespace ofxLaser {

class ZoneId {
    public :
    ZoneId() {
        type = BEAM;
        zoneGroup = 0;
        zoneIndex = 0;
        label = "";
    }
    
    enum ZoneType {
        BEAM,
        CANVAS
    };
    
    string getLabel() {
        if(label=="") {
            
            string newlabel = "";
            
            if(type==BEAM) newlabel = "BEAM";
            else newlabel = "CANVAS";
            newlabel = newlabel +ofToString(zoneIndex);
            return newlabel;
                
        } else {
            return label;
        }
        
    }
    
    string getUid() const {
        string uid = "";
        if(type==BEAM) uid = "B";
        else uid = "C";
        uid = uid+ofToString(zoneGroup)+"_"+ofToString(zoneIndex);
        return uid;
    }
    
    bool operator==(const ZoneId & other) const;
    bool operator!=(const ZoneId & other) const;
    
    ZoneType type;
    int zoneGroup;
    int zoneIndex;
    
    string label;
    
    
};




}


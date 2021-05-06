//
//  ofxLaserProjectorZone.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 13/04/2021.
//
#pragma once
#include "ofMain.h"
#include "ofxLaserZoneTransform.h"
#include "ofxLaserZone.h"

namespace ofxLaser {

class LaserZone {
    
    public :
    
    LaserZone(Zone& _zone) ;
    ~LaserZone();
    
    bool update() ;
    
    bool getEnabled() {
        return enabled;
    }
    void setEnabled(bool value) {
        enabled = value;
        zoneTransform.setEditable(enabled); 
    }
    void setVisible(bool value) {
        visible = value;
        zoneTransform.setVisible(visible);
    }
    bool getVisible() {
        return visible;
    }
    
    void draw();
    
    string getLabel();
    const int getZoneIndex() const {
        return zone.getIndex();
    };
    void setScale(float _scale) ;
    void setOffset(ofPoint _offset);
    void zoneMaskChanged(ofAbstractParameter& e) ;
    void updateZoneMask() ;
    
    bool serialize(ofJson& json);
    bool deserialize(ofJson& json);
    
    Zone& zone;
    ZoneTransform zoneTransform;
    ofRectangle zoneMask;
    ofParameter<float>leftEdge;
    ofParameter<float>rightEdge;
    ofParameter<float>bottomEdge;
    ofParameter<float>topEdge;
    ofParameter<bool>muted;
    ofParameter<bool>soloed;
    
    ofParameterGroup params; 
    ofParameterGroup zoneMaskGroup;
    bool isDirty; 
    
    float scale;
    ofPoint offset;
    
    protected :
    bool enabled;
    bool visible;
    
};

}

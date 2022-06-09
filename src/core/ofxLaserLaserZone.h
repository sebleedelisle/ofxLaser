//
//  ofxLaserLaserZone.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 13/04/2021.
//
#pragma once
#include "ofMain.h"
#include "ofxLaserZoneTransform.h"
#include "ofxLaserZone.h"

namespace ofxLaser {

class LaserZone : public ZoneTransform {
    
    public :
    
    LaserZone(Zone& _zone) ;
    ~LaserZone();
    
    virtual bool update() override ;
    
    bool getEnabled() {
        return enabled;
    }
    void setEnabled(bool value) {
        enabled = value;
        setEditable(enabled);
    }
    void setVisible(bool value) {
        visible = value;
        ZoneTransform :: setVisible(visible);
    }
    bool getVisible() {
        return visible;
    }
    
    void draw();
    
    string getLabel();
    const int getZoneIndex() const {
        return zone.getIndex();
    };

    // scale and offset are only for the visual interface
    void setScale(float _scale) ;
    void setOffset(ofPoint _offset);
    void zoneMaskChanged(ofAbstractParameter& e) ;
    void paramChanged(ofAbstractParameter& e) ;
    void updateZoneMask() ;
    
    virtual bool serialize(ofJson& json) override;
    virtual bool deserialize(ofJson& json) override;
    
    Zone& zone;
    //ZoneTransform zoneTransform;
    ofRectangle zoneMask;
    ofParameter<float>leftEdge;
    ofParameter<float>rightEdge;
    ofParameter<float>bottomEdge;
    ofParameter<float>topEdge;
    ofParameter<bool>muted;
    ofParameter<bool>soloed;
    
    //ofParameterGroup params;
    ofParameterGroup zoneMaskGroup;
    ofParameterGroup zoneParams;
    bool isDirty; 
    
    //float scale;
    //ofPoint offset;
    
    protected :
    bool enabled;
    bool visible;
    
};

}

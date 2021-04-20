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

class ProjectorZone {
    
    public :
    
    ProjectorZone(Zone& _zone) ;
    ~ProjectorZone();
    
    bool update() ;
    void setVisible(bool visible);
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
    ofParameter<bool>enabled;
    
    ofParameterGroup params; 
    ofParameterGroup zoneMaskGroup;
    bool isDirty; 
    
    float scale;
    ofPoint offset;
    
};

}

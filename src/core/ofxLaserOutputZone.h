//
//  ofxLaserLaserZone.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 13/04/2021.
//
#pragma once
#include "ofMain.h"
#include "ofxLaserZoneTransformQuadData.h"
#include "ofxLaserZoneTransformLineData.h"
#include "ofxLaserInputZone.h"

namespace ofxLaser {

class OutputZone {
    
    public :
    
    OutputZone(InputZone& _zone) ;
    ~OutputZone();
    void init(ofRectangle sourceRectangle); 
    
    virtual bool update();
    
    void setSourceRect(ofRectangle & rect); 
//    
//    bool getEnabled();
//    void setEnabled(bool value);
//    
    void draw();
    
    string getLabel();
    const int getZoneIndex() const;
    
    bool getIsAlternate();
    void setIsAlternate(bool v);
    
    ofxLaser::Point getWarpedPoint(const ofxLaser::Point& p);
    ofxLaser::Point getUnWarpedPoint(const ofxLaser::Point& p);
    ofPoint getWarpedPoint(const ofPoint& p);
    ofPoint getUnWarpedPoint(const ofPoint& p);
    
    void paramChanged(ofAbstractParameter& e) ;
    
    bool setGrid(bool snapstate, int gridsize);
    
    ofRectangle getBounds();
    void drawPerimeterAsShape(); 
    
    virtual bool serialize(ofJson& json);
    virtual bool deserialize(ofJson& json);
    
    InputZone& zone;
    ZoneTransformBase& getZoneTransform();
    ZoneTransformQuadData zoneTransformQuad;
    ZoneTransformLineData zoneTransformLine;

    ofParameterGroup zoneParams;
    
    ofParameter<bool>muted;
    ofParameter<bool>soloed;
    ofParameter<int>transformType;
    
    
    bool isDirty;
    
    protected :
    bool enabled;
    
    bool isAlternate;
    
};

}

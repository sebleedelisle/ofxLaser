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
#include "ofxLaserZoneTransformQuadComplexData.h"
#include "ofxLaserZoneId.h"

namespace ofxLaser {

class OutputZone {
    
    public :
    
    OutputZone(ZoneId zoneid, ofRectangle sourcerect = ofRectangle(0,0,400,400));
    ~OutputZone();
    
    virtual bool update();
    
    bool setSourceRect(const ofRectangle & rect);

    void draw();
    
    string getLabel();
    ZoneId getZoneId() const;
    bool setZoneId(ZoneId& zoneid); 
    
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
    
    virtual void serialize(ofJson& json) const;
    virtual bool deserialize(ofJson& json);
    
    //InputZone& zone;
    ZoneTransformBase& getZoneTransform();
    void resetAllTransforms();
    ZoneTransformQuadData zoneTransformQuad;
    ZoneTransformLineData zoneTransformLine;
    ZoneTransformQuadComplexData zoneTransformQuadComplex;

    ofParameterGroup zoneParams;
    
    ofParameter<bool>muted;
    ofParameter<bool>locked;
    ofParameter<bool>soloed;
    ofParameter<int>transformType;
    
    bool isDirty;
    
    protected :
    bool enabled;
    
    bool isAlternate;
    
    ZoneId zoneId; 
    
};

}

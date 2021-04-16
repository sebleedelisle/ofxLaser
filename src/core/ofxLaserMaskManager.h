//
//  ofxLaserMaskManager .h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 02/02/2018.
//


#pragma once
#include "ofMain.h"
#include "ofxLaserQuadMask.h"
#include "ofxLaserFactory.h"


namespace ofxLaser {
    
class MaskManager  {
    
public:
    
    MaskManager ();
    ~MaskManager ();
     
    virtual void init(int width, int height);
    virtual bool update();
    virtual bool draw();
    
    virtual bool deleteQuadMask(QuadMask* mask); 
    
    bool loadSettings();
    bool saveSettings();
	void setOffsetAndScale(ofPoint offset, float scale); 
	
	vector<ofPolyline*> getLaserMaskShapes();
    QuadMask& addQuadMask(int level=100);
    
    vector<QuadMask*> quads;
    
	int width, height; 
	
    protected :
    bool dirty;
	ofPoint offset;
	float scale = 1;
    bool firstUpdate = true; 
    
};
}

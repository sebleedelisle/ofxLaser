//
//  ofxLaserShapeTargetCanvas.h
//
//  Created by Seb Lee-Delisle on 23/02/2023.
//
//

#pragma once

#include "ofxLaserShapeTarget.h"
#include "ofxLaserInputZone.h"
#include "ofxLaserZoneIdContainer.h"

namespace ofxLaser {

// This needs :
// load file, store file name
// serialize, deserialize
//
class GuideImage {
    
    public :
    
    GuideImage();
    
    bool load(string _filename) {
        
        if(image.load(_filename)) {
            filename = _filename;
            return true;
        } else {
            return false;
        }
        
    }
    
    void stretchToFit(ofRectangle targetRect) {
      
        float scale =  targetRect.getWidth() / image.getWidth();
        if((image.getHeight()*scale) > targetRect.getHeight()) {
            scale =  targetRect.getHeight() / image.getHeight();
        }
        rect.set(0,0,image.getWidth()*scale, image.getHeight()*scale) ;
        
    }
    // do i need to destroy it? I think ofImage should destroy itself...
    ofImage image;
    string filename;
    // For now, fuck it let's just use a rectangle
    ofRectangle rect;
    float alpha;
    ofColor colour;
    
    
    
};



class ShapeTargetCanvas : public ShapeTarget, public ZoneIdContainer {
    public :
    
    ShapeTargetCanvas(); 
    
    //void clearZones();
    ZoneId addInputZone(float x, float y, float w, float h);
    
    virtual bool addZoneByJson(ofJson& json) override;
   

    virtual void serialize(ofJson& json) override;
    virtual bool deserialize(ofJson& json) override;
  
    
    vector<Shape*>getShapesForZoneId(ZoneId& zoneid); 
    InputZone* getInputZoneForZoneId(ZoneId& zoneid);
    InputZone* getInputZoneForZoneIndex(int index);
    InputZone* getInputZoneForZoneIdUid(string& uid);

    
    vector<InputZone*> getInputZones();
    
    
};
}


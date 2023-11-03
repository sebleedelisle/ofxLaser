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
    
    GuideImage(){};
    
    bool load(string _filename) {
        
        if(image.load(_filename)) {
            filename = _filename;
            return true;
        } else {
            return false;
        }
        
    }
    
    void setRectFromImage() {
        if(image.isAllocated()) {
            rect.set(0,0,image.getWidth(), image.getHeight());
        } else {
            rect.set(0,0,100,100);
        }
    }
    void stretchToFit(ofRectangle targetRect) {
      
        float scale =  targetRect.getWidth() / image.getWidth();
        if((image.getHeight()*scale) > targetRect.getHeight()) {
            scale =  targetRect.getHeight() / image.getHeight();
        }
        rect.set(0,0,image.getWidth()*scale, image.getHeight()*scale) ;
        
    }
    
    virtual void serialize(ofJson& json) {
        json["filename"] = filename;
        json["rect"] = {rect.x, rect.y, rect.width, rect.height};
        json["colour"] = {colour.r, colour.g, colour.b};
        json["locked"]  = locked;
        
    }
    virtual bool deserialize(ofJson& json){
        if(json.contains("filename")) {
            filename = json["filename"].get<string>();
            load(filename);
        }
        if(json.contains("rect")) {
            if(json["rect"].size()>=4) {
                rect.x = json["rect"][0].get<float>();
                rect.y = json["rect"][1].get<float>();
                rect.width = json["rect"][2].get<float>();
                rect.height = json["rect"][3].get<float>();
            }
        }
            
        if(json.contains("colour")) {
            if(json["colour"].size()>=3) {
                colour.r =json["colour"][0].get<int>();
                colour.g =json["colour"][1].get<int>();
                colour.b =json["colour"][2].get<int>();
            }
        }
            
        if(json.contains("locked")) {
            locked = json["locked"].get<bool>();
        }
        
        return true;
    }
  
 
    
    // do i need to destroy it? I think ofImage should destroy itself...
    ofImage image;
    string filename;
    // For now, fuck it let's just use a rectangle
    ofRectangle rect;
    float alpha;
    ofColor colour;
    bool locked = false; 
    
    
};



class ShapeTargetCanvas : public ShapeTarget, public ZoneIdContainer {
    public :
    
    ShapeTargetCanvas(); 
    
    //void clearZones();
    ZoneId addInputZone(float x, float y, float w, float h);
    bool addGuideImage(string filename);
    
    virtual bool addZoneByJson(ofJson& json) override;
    
    virtual void serialize(ofJson& json) override;
    virtual bool deserialize(ofJson& json) override;
  
    vector<Shape*>getShapesForZoneId(ZoneId& zoneid); 
    InputZone* getInputZoneForZoneId(ZoneId& zoneid);
    InputZone* getInputZoneForZoneIndex(int index);
    InputZone* getInputZoneForZoneIdUid(string& uid);
    
    GuideImage*  getGuideImageForUid(string& uid);
    bool deleteGuideImage(GuideImage* guideImage); 


    vector<InputZone*> getInputZones();
    vector<GuideImage*> getGuideImages();
    
    vector<GuideImage> guideImages;
    
    bool lockZones = false;
    
    int zoneBrightness =255;
    
    
};
}


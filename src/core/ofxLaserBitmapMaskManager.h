//
//  ofxLaserBitmapMaskManager.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 02/02/2018.
//


#pragma once
#include "ofxLaserMaskManager.h"


namespace ofxLaser {
    
class BitmapMaskManager : public MaskManager {
    
public:
    
    BitmapMaskManager();
    ~BitmapMaskManager();
     
    virtual void init(int width, int height) override;
    virtual bool update() override;
   //b virtual bool draw(bool showBitmap = false);
    
    ofPixels* getPixels();
    float getBrightness(int x, int y);

    ofFbo fbo;
    ofPixels pixels;
    bool firstUpdate = true;
    
};
}

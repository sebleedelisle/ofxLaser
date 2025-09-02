//
//  ofxLaserColourSettings.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 20/04/2021.
//


#pragma once
#include "ofxLaserPoint.h"
#include "ofMain.h"
#include "ofxLaserPresetBase.h"

namespace ofxLaser {
class ColourSettings : public PresetBase {
    
    public :
    
    ColourSettings();
    
    
    static string getFolderPath(){
        return "ofxLaser/colourpresets";
    };
    static string getTypeName() {
        return "Colour Settings";
    }
    static string getFileExtension() {
        return "lclr";
    }
    ColourSettings& operator=( ColourSettings& that);
    bool operator == (ColourSettings& that);
    bool operator != (ColourSettings& that);
  
    
    
    float calculateCalibratedBrightness(float value, float intensity, float level100, float level75, float level50, float level25, float level0);
    void processColour(ofxLaser::Point& p, float brightness);
    
    // would probably be sensible to move these settings out into a colour
    // calibration object.
    //ofParameterGroup params;
    
    ofParameter<float>red100;
    ofParameter<float>red75;
    ofParameter<float>red50;
    ofParameter<float>red25;
    ofParameter<float>red0;
    
    ofParameter<float>green100;
    ofParameter<float>green75;
    ofParameter<float>green50;
    ofParameter<float>green25;
    ofParameter<float>green0;
    
    ofParameter<float>blue100;
    ofParameter<float>blue75;
    ofParameter<float>blue50;
    ofParameter<float>blue25;
    ofParameter<float>blue0;
    
};
}

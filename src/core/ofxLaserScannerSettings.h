//
//  ofxLaserScannerSettings.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 20/04/2021.
//

#pragma once
//
#include "ofMain.h"
#include "constants.h"
#include "ofxLaserRenderProfile.h"
#include "ofxLaserPresetBase.h"

namespace ofxLaser {
class ScannerSettings: public PresetBase {
    
    public :
    
    ScannerSettings();
    
    static string getFolderPath(){
        return "ofxLaser/scannerpresets";
    };
    static string getTypeName() {
        return "Scanner";
    }
    ScannerSettings& operator=( ScannerSettings& that);
    bool operator == (ScannerSettings& that);
    bool operator != (ScannerSettings& that);
  
    
    // scanner settings
    ofParameterGroup renderParams;
    ofParameter<float> moveSpeed = 5;
    ofParameter<int> shapePreBlank = 0;
    ofParameter<int> shapePostBlank = 0;
    ofParameter<int> shapePreOn = 0;
    ofParameter<int> shapePostOn = 0;
    
    RenderProfile profileFast;
    RenderProfile profileDefault;
    RenderProfile profileDetail;
    
    map<string, RenderProfile&> renderProfiles;

    
};

}

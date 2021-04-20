//
//  ofxLaserScannerSettings.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 20/04/2021.
//

#pragma once
//
#include "ofMain.h"
#include "constants.h"
#include "ofxLaserRenderProfile.h"

namespace ofxLaser {
class ScannerSettings {
    
    public :
    
    ScannerSettings();
    
    // scanner settings
    ofParameterGroup params;
    ofParameterGroup renderParams;
    ofParameter<float> moveSpeed = 5;
    ofParameter<int> shapePreBlank = 0;
    ofParameter<int> shapePostBlank = 0;
    ofParameter<int> shapePreOn = 0;
    ofParameter<int> shapePostOn = 0;
    map<string, RenderProfile> renderProfiles;

    
};

}

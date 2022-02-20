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

namespace ofxLaser {
class ScannerSettings {
    
    public :
    
    ScannerSettings();
    ScannerSettings& operator=( ScannerSettings& that);
    bool operator == (ScannerSettings& that);
    bool operator != (ScannerSettings& that);
  
    void serialize(ofJson&json);
    bool deserialize(ofJson&jsonGroup);
    const string& getLabel(); 
    void setLabel(string _label) {
        label = _label;
    };
    
    // scanner settings
    ofParameterGroup params;
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
    private :
    ofParameter<string> label;
    ofParameter<string> description;
    
    
};

}

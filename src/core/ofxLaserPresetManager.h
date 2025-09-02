//
//  PresetManager.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 23/04/2021.
//

#pragma once
#include "ofxLaserScannerSettings.h"

namespace ofxLaser {
class PresetManager {
    
    
    public :
    PresetManager();
    //~PresetManager();
   
    static void loadPresets();
    static void addPreset(string, ScannerSettings& settings);
    static void addPreset(ScannerSettings& settings);
    static void savePreset(string, ScannerSettings& settings);
    static ScannerSettings* getPreset(string name);
    static  const vector<string>& getPresetNames(); 
      
        
    // it's a Singleton so shouldn't ever have more than one.
    static PresetManager * instance();
    static PresetManager * presetManager;
 
    static map<string, ScannerSettings> presetMap;
    static vector<string> presetNames; 
    
    
    
};
}

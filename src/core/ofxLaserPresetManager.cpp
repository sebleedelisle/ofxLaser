//
//  PresetManager.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 23/04/2021.
//

#include "ofxLaserPresetManager.h"

using namespace ofxLaser;

map<string, ScannerSettings> PresetManager::presetMap;
vector<string> PresetManager::presetNames;

PresetManager * PresetManager :: presetManager = NULL;

PresetManager * PresetManager::instance() {
    if(presetManager == NULL) {
        presetManager = new PresetManager();
    }
    return presetManager;
}


PresetManager :: PresetManager() {

    if(presetManager == NULL) {
        presetManager = this;
    } else {
        ofLog(OF_LOG_ERROR, "Multiple ofxLaser::PresetManager instances created");
        throw;
    }
    loadPresets();
   
}

void PresetManager :: loadPresets() {
    // open directory
    ofDirectory dir = ofDirectory("ofxLaser/scannerpresets");
    if(dir.exists()) {
        dir.listDir();
      
        //only show svg files
        dir.allowExt("json");
        //populate the directory object
        dir.listDir();
        const vector<ofFile>& allFiles = dir.getFiles();
        dir.close();
        
        // iterate through files
        
        for(auto file : allFiles) {
            ScannerSettings settings;
            ofJson json = ofLoadJson("ofxLaser/scannerpresets/"+file.getFileName());
            settings.deserialize(json);
            addPreset(settings);
            
        }
    }
    // addPreset to List
    // close directory
    // if no presets loaded make a default preset
    if(presetMap.size()==0) {
        ScannerSettings defaultSettings;
        addPreset("Default", defaultSettings);
        //addPreset("Fast Scanners", defaultSettings);
    }
}

void PresetManager::addPreset(string name, ScannerSettings& settings) {
    settings.setLabel(name);
    addPreset(settings);
    
}

void PresetManager :: addPreset(ScannerSettings& settings) {
    // check if we have one already?
    //settings.label = name; // needs rethink
    string name = settings.getLabel();
    
    if(presetMap.count(name)>0) {
        // we already have one
        // output warning?
    }
    // add to map, this should make a copy, right?
    presetMap[name] = settings;
    
    // save
    savePreset(name, settings);
    
}


void PresetManager :: savePreset(string label, ScannerSettings& settings){
    // serialize settings
    ofJson presetJson;
    settings.serialize(presetJson);
    // save json
    ofSavePrettyJson("ofxLaser/scannerpresets/"+label+".json", presetJson);
    
}

ScannerSettings* PresetManager :: getPreset(string name) {
    if(presetMap.count(name)>0) {
        return &(presetMap.find(name)->second);
    } else {
        throw;
        return nullptr;
    }
}

const vector<string>& PresetManager ::getPresetNames() {
    // update vector of strings :
    if(true) { // TODO add dirty flag
        presetNames.resize(presetMap.size());
        
        for (auto it = presetMap.begin(); it != presetMap.end(); it++) {
            int i = std::distance(std::begin(presetMap), it);
            presetNames[i] = it->first;
        
        }
    }
    return presetNames;
    
}

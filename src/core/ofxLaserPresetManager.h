//
//  PresetManager.hpp
//  GenericTest
//
//  Created by Seb Lee-Delisle on 17/06/2022.
//

#pragma once
#include "ofxLaserPresetBase.h"
#include <type_traits>
#include "ofMain.h"

namespace ofxLaser {
template <typename T>
class PresetManager {
    static_assert(std::is_base_of<PresetBase, T>::value, "T must derive from PresetBase");
    
    public :
    PresetManager();
    
    bool loadPresets();
    void addPreset(string, T& preset) ;
    void addPreset(T& preset) ;
    void savePreset(string, T& preset);
    T* getPreset(string name);
    const vector<string>& getPresetNames();
    
    map<string, T> presetMap;
    vector<string> presetNames;
    
    string filepath;
    
    
};
}
 
using namespace ofxLaser; 

template <typename T>
PresetManager<T> :: PresetManager() {
    filepath = T::getFolderPath();
    loadPresets();
}


template <typename T>
bool PresetManager<T> :: loadPresets() {
    // open directory
    ofDirectory dir = ofDirectory(filepath);
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
            T settings;
            ofJson json = ofLoadJson(filepath+"/"+file.getFileName());
            settings.deserialize(json);
            addPreset(settings);
            
        }
    }
    // addPreset to List
    // close directory
    // if no presets loaded make a default preset
    if(presetMap.size()==0) {
        T defaultSettings;
        addPreset("Default", defaultSettings);
        //addPreset("Fast Scanners", defaultSettings);
    }
}

template <typename T>
void PresetManager<T>::addPreset(string name, T& settings) {
    settings.setLabel(name);
    addPreset(settings);
    
}

template <typename T>
void PresetManager<T> :: addPreset(T& settings) {
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

template <typename T>
void PresetManager<T> :: savePreset(string label, T& settings){
    // serialize settings
    ofJson presetJson;
    settings.serialize(presetJson);
    // save json
    ofSavePrettyJson(filepath+"/"+label+".json", presetJson);
    
}

template <typename T>
T* PresetManager<T> :: getPreset(string name) {
    if(presetMap.count(name)>0) {
        return &(presetMap.find(name)->second);
    } else {
        ofLogError("Preset not found : ") << name;
        if(presetMap.size()>0) return &(presetMap.begin()->second);
        else return nullptr;
    }
}

template <typename T>
const vector<string>& PresetManager<T> ::getPresetNames() {
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

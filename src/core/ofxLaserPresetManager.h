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
#include "ofxLaserUI.h"

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
    
    // UI stuff
    void drawComboBox(T& currentPreset);
    void drawSaveButtons(T& currentPreset);
    
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
    return true; 

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

template <typename T>
void PresetManager<T> :: drawComboBox(T& settings) {
    
    const vector<string>& presets = getPresetNames();
    string label =settings.getLabel();
    T& currentPreset = *getPreset(label);
    
    bool presetEdited = (settings!=currentPreset);
    if (presetEdited){
        label+="(edited)";
    }
  
    string comboname = T::getTypeName() + " presets";
    if (ImGui::BeginCombo(comboname.c_str(), label.c_str())) { // The second parameter is the label previewed before opening the combo.
        
        for(const string presetName : presets) {
            
            if (ImGui::Selectable(presetName.c_str(), presetName == settings.getLabel())) {
                //get the preset and make a copy of it
                // uses operator overloading to create a clone
                settings = *getPreset(presetName);
            }
        }
        
        ImGui::EndCombo();
    }
    
}

template <typename T>
void PresetManager<T> :: drawSaveButtons(T& settings) {
    
    const vector<string>& presets = getPresetNames();
    string label =settings.getLabel();
    T& currentPreset = *getPreset(label);
    
    bool presetEdited = (settings!=currentPreset);
    
    if(!presetEdited) UI::startGhosted();
    label ="SAVE##"+T::getTypeName();
    
    if(ImGui::Button(label.c_str())) {
        label ="Save "+T::getTypeName()+" Preset";
        if(presetEdited)ImGui::OpenPopup(label.c_str());
        
    }
    UI::stopGhosted();
    label = "Save "+T::getTypeName()+" Preset";
    if (ImGui::BeginPopupModal(label.c_str(), 0)){
        string presetlabel = settings.getLabel();
        
        ImGui::Text("Are you sure you want to overwrite the preset \"%s\"?", presetlabel.c_str());
        ImGui::Separator();
        
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            addPreset(presetlabel, settings);
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::EndPopup();
        
        
    }
    static char newPresetLabel[255]; // = presetlabel.c_str();
    
    
    ImGui::SameLine();
    label = "SAVE AS##%s"+T::getTypeName();
    
    if(ImGui::Button(label.c_str() )) {
        strcpy(newPresetLabel, settings.getLabel().c_str());
        label = "Save "+T::getTypeName()+" Preset As";
        ImGui::OpenPopup(label.c_str());
        
    };
    
    label = "Save "+T::getTypeName()+" Preset As" ;
    if (ImGui::BeginPopupModal(label.c_str(), 0)){
        
        if(ImGui::InputText("1", newPresetLabel, IM_ARRAYSIZE(newPresetLabel))){
            
        }
        
        ImGui::Separator();
        label = "OK## "+T::getTypeName();
        if (ImGui::Button(label.c_str(),  ImVec2(120, 0))) {
            string presetlabel = newPresetLabel;
            // TODO CHECK PRESET EXISTS AND ADD POP UP
            addPreset(presetlabel, settings);
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        label = "Cancel## "+T::getTypeName();
        if (ImGui::Button(label.c_str(), ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::EndPopup();
        
        
    }
    
}
    

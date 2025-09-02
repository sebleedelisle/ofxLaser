//
//  PresetManager.h
//
//  Created by Seb Lee-Delisle on 17/06/2022.
//

#pragma once
#include "ofxLaserPresetBase.h"
#include <type_traits>
#include "ofMain.h"
#include "ofxLaserUI.h"
//#define OFXLASER_USE_OFXNATIVE

#ifdef OFXLASER_USE_OFXNATIVE
#include "ofxNative.h"
#endif


namespace ofxLaser {
template <typename T>
class PresetManager {
    static_assert(std::is_base_of<PresetBase, T>::value, "T must derive from PresetBase");
    
    public :
    PresetManager();
    
    bool loadPresets();
    void addPreset(string, T& preset, bool save = true) ;
    void addPreset(T& preset, bool save = true);
    void deletePreset(T& preset, bool save = true);
    void deleteAllPresets();
    void savePreset(string, T& preset);
    T* getPreset(string name);
    const vector<string>& getPresetNames();
    
    
    
    // UI stuff
    bool drawSettingsButton();
    bool drawComboBox(T& currentPreset, int idnum = 0, bool showLabel = true);
    bool drawSaveButtons(T& currentPreset);
    
    virtual void serialize(ofJson& json) const;
    virtual bool deserialize(ofJson& json);
    
    static map<string, T> presetMap;
    vector<string> presetNames;
    
    string filepath;
    
    //static map<string, T>  test;
    
};


}
 
namespace ofxLaser {

template <typename T>
map<string, T>  PresetManager<T> :: presetMap;


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
            //ofLogNotice("found file") << filepath << "/" << file.getFileName();
            ofJson json = ofLoadJson(filepath+"/"+file.getFileName());
            settings.deserialize(json);
            addPreset(settings, false);
            
        }
    }
    // addPreset to List
    // close directory
    // if no presets loaded make a default preset
    if(presetMap.size()==0) {
        T defaultSettings;
        addPreset("Default", defaultSettings, true); // save
        //addPreset("Fast Scanners", defaultSettings);
        
    }
    return true;
    
}

template <typename T>
void PresetManager<T>::addPreset(string name, T& settings, bool save) {
    settings.setLabel(name);
    ofLogNotice("add preset name : ") << name << " " << settings.getLabel();
    addPreset(settings, save);
    
}

template <typename T>
void PresetManager<T> :: addPreset(T& preset, bool save) {
    // check if we have one already?
    //settings.label = name; // needs rethink
    string name = preset.getLabel();
    
    
    if(presetMap.count(name)>0) {
        // we already have one
        // output warning?
    }
    // add to map, this should make a copy, right?
    presetMap[name] = preset;
    
    // save
    if(save) {
        savePreset(name, preset);
    }
}
template <typename T>
void PresetManager<T> :: deletePreset(T& preset, bool save) {
    
    string name =preset.getLabel();
    presetMap.erase(name);
    ofFile presetfile(filepath+"/"+name+".json");
    presetfile.remove();
    
    
}
template <typename T>
void PresetManager<T> :: deleteAllPresets() {
    
    vector<string> names = getPresetNames();
    for(string name : names) {
        ofFile presetfile(filepath+"/"+name+".json");
    }
    presetMap.clear();
    
}

template <typename T>
void PresetManager<T> :: savePreset(string label, T& settings){
    // serialize settings
    ofJson presetJson;
    settings.serialize(presetJson);
    // save json
    ofLogNotice("Saving preset : " ) << label;
    ofLogNotice("filename : " ) << filepath+"/"+label+".json";
    ofSavePrettyJson(filepath+"/"+label+".json", presetJson);
    
}

template <typename T>
T* PresetManager<T> :: getPreset(string name) {
    if(presetMap.count(name)>0) {
        return &(presetMap.find(name)->second);
    } else {
        ofLogError("Preset not found : ") << name;
        return nullptr;
//        if(presetMap.size()>0) return &(presetMap.begin()->second);
//        else return nullptr;
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
bool PresetManager<T> :: drawSettingsButton() {
#ifdef TARGET_OSX
    bool commandPressed = ofGetKeyPressed(OF_KEY_COMMAND);
#else
    bool commandPressed = ofGetKeyPressed(OF_KEY_CONTROL);
#endif
    string comboname = T::getTypeName() + " presets";
    
    string windowname = "Edit "+comboname;

    ImGui::PushID(comboname.c_str());
    
    if(ImGui::Button(ICON_FK_FLOPPY_O)) {
        ImGui::OpenPopup(windowname.c_str());
    }
    UI::addDelayedHover("Save / load / delete presets");
    
    if(ImGui::BeginPopup(windowname.c_str())) {
        
        ImGui::Text("%s", windowname.c_str());
        
        ImGui::Separator();
        
        if(ImGui::Button("IMPORT PRESETS FROM FILE")) {
#ifdef OFXLASER_USE_OFXNATIVE
            ofFileDialogResult result = ofxNative::systemLoadDialog("Choose preset file to import", false, "", {T::getFileExtension()});
#else
            ofFileDialogResult result = ofSystemLoadDialog("Choose preset file to import");
#endif
            
            if(result.bSuccess) {
                ofJson importjson = ofLoadJson(result.filePath);
                deserialize(importjson);
                ImGui::CloseCurrentPopup();
            }
            
        }
        
        if(ImGui::Button("EXPORT PRESETS TO FILE")) {
            string defaultfilename = T::getTypeName()+"Presets."+T::getFileExtension();
            ofLogNotice("EXPORT PRESETS TO FILE") << defaultfilename;
            
#ifdef OFXLASER_USE_OFXNATIVE
            ofFileDialogResult result = ofxNative::systemSaveDialog(defaultfilename, "Export "+ T :: getTypeName() +" presets", {T::getFileExtension()});
#else
            ofFileDialogResult result = ofSystemSaveDialog(defaultfilename, "Export "+ T :: getTypeName() +" presets");
#endif
            if(result.bSuccess) {
                ofJson json;
                serialize(json);
                ofSavePrettyJson(result.filePath, json);
                ImGui::CloseCurrentPopup();
            }
        }
        
        string presetToDeleteName = "";
        
        if(presetMap.size()>1) {
            
            const vector<string>& presets = getPresetNames();
            
            for(const string& presetName : presets) {
                
                ImGui::Separator();
                ImGui::Text("%s", presetName.c_str());
                
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetWindowWidth()-34);
                string buttonlabel = ofToString(ICON_FK_MINUS_CIRCLE) + "##" + presetName;
                
                string confirmwindowlabel = "Delete " + presetName;
                
                if (UI::DangerButton(buttonlabel, false)) {
                    // delete preset
                 
                    if(commandPressed) {
                        presetToDeleteName = presetName;
                    } else {
                        ImGui::OpenPopup(confirmwindowlabel.c_str());
                    }
                }
                
                
                
                ImVec2 mousePos = ImGui::GetMousePos(); // Get mouse position
                ImGui::SetNextWindowPos(mousePos, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                
                if(ImGui::BeginPopup(confirmwindowlabel.c_str())) {
                    
                    ImGui::Text("Delete %s - are you sure?", presetName.c_str());
                    ImGui::Separator();
                    if(UI::DangerButton("DELETE")) {
                        presetToDeleteName = presetName; 
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    ImGui::SetItemDefaultFocus();
                    if(UI::Button("Cancel")) {

                        ImGui::CloseCurrentPopup();
                    }
                    
                    ImGui::EndPopup();
                }
                
                
                
                
                
            }
        }
        
        if(presetToDeleteName!="") {
            T& preset = *getPreset(presetToDeleteName);
            deletePreset(preset);
        }
//        ImGui::Text("GROUP %d SETTINGS", i+1);
//        
//        if(groupData->flashMode) UI::secondaryColourStart();
//        if(ImGui::Button("FLASH MODE")) {
//            controllerToggleGroupFlashMode(i);
//        }
//        UI::secondaryColourEnd();
//
//        GroupData* groupdata = groupManager->getGroupData(i);
//        if(groupdata!=nullptr) {
//            if(ofxLaser::UI :: addFloatSlider(groupdata->transitionInTime, "%.2f", ImGuiSliderFlags_Logarithmic)) {
//                controllerSetGroupTransitionInTime(i, groupData->transitionInTime);
//                
//            }
//            if(ofxLaser::UI :: addFloatSlider(groupdata->transitionOutTime, "%.2f", ImGuiSliderFlags_Logarithmic)) {
//                controllerSetGroupTransitionOutTime(i, groupData->transitionOutTime);
//            }
//        }
        
        ImGui::EndPopup();
    }
    ImGui::PopID();
    
    return true;
}

template <typename T>
bool PresetManager<T> :: drawComboBox(T& settings, int idnum, bool showLabel) {
    
    bool changed = false;
    const vector<string>& presets = getPresetNames();
    string label =settings.getLabel();
    
    T* currentPreset = getPreset(label);
    
    bool presetEdited = false;
    if (currentPreset == nullptr){
        label+="(deleted)";
        presetEdited = true;
    } else if (settings!=*currentPreset){
        label+="(edited)";
        presetEdited = true;
    }
    
    string confirmopenwindowname = "";
    
    string comboname = "##"+T::getTypeName() + " presets##"+ofToString(idnum);
    //if(!showLabel) comboname = "##"+comboname;
    if (ImGui::BeginCombo(comboname.c_str(), label.c_str())) { // The second parameter is the label previewed before opening the combo.
        
        for(const string& presetName : presets) {
            string presetlabel =presetName+"##"+ofToString(idnum);
            
            string confirmwindowlabel = "Confirm preset change ##" + T::getTypeName()+"|"+presetName;
            
            if (ImGui::Selectable(presetlabel.c_str(), presetName == settings.getLabel())) {
                //get the preset and make a copy of it
                // uses operator overloading to create a clone
                if(presetEdited) {
                    confirmopenwindowname = confirmwindowlabel;
                    
                } else {
                    settings = *getPreset(presetName);
                    changed = true;
                }
            }
        }
        ImGui::EndCombo();
    }
        
    if(confirmopenwindowname!="") ImGui::OpenPopup(confirmopenwindowname.c_str());
        
    for(const string& presetName : presets) {
        string confirmwindowlabel = "Confirm preset change ##" + T::getTypeName()+"|"+presetName;
        
        ImVec2 mousePos = ImGui::GetMousePos(); // Get mouse position
        ImGui::SetNextWindowPos(mousePos, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        
        if(ImGui::BeginPopup(confirmwindowlabel.c_str())) {
            ImGui::Text("Change to %s?", presetName.c_str());
            ImGui::Text("You will lose your current settings");
            ImGui::Separator();
            if(UI::DangerButton("CHANGE")) {
                settings = *getPreset(presetName);
                changed = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if(UI::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
    }
    ImGui::SameLine();
    
    drawSettingsButton();
    
    if(showLabel) {
        ImGui::SameLine();
        ImGui::Text("%s presets", T::getTypeName().c_str());
        
    }
    
    
    return changed;
    
}

template <typename T>
bool PresetManager<T> :: drawSaveButtons(T& settings) {
    bool changed = false;
    const vector<string>& presets = getPresetNames();
    string label =settings.getLabel();
    T* currentPreset = getPreset(label);
    
    bool presetEdited = (currentPreset!=nullptr) && (settings!=*currentPreset);
    bool presetDeleted = false;
    if(currentPreset == nullptr) {
        presetDeleted = true;
        currentPreset = &settings;
    }
    
    if((!presetEdited) && (!presetDeleted)) UI::startDisabled();
    label ="STORE##"+T::getTypeName();
    
    if(ImGui::Button(label.c_str())) {
        label ="Store "+T::getTypeName()+" Preset";
        if(presetEdited) ImGui::OpenPopup(label.c_str());
        else if(presetDeleted) {
            addPreset(settings.getLabel(), settings);
            changed = true;
        }
    }
    
    UI::stopDisabled();
    label = "Store "+T::getTypeName()+" Preset";
    
    ImVec2 mousePos = ImGui::GetMousePos(); // Get mouse position
    ImGui::SetNextWindowPos(mousePos, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopup(label.c_str(), 0)){
        
        string presetlabel = settings.getLabel();
        
        ImGui::Text("Are you sure you want to overwrite");
        ImGui::Text("the preset \"%s\"?", presetlabel.c_str());
        ImGui::Separator();
        
        if (UI::DangerButton("OK")) {
            addPreset(presetlabel, settings);
            changed = true;
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (UI::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::EndPopup();
    }
    static char newPresetLabel[255]; // = presetlabel.c_str();
    
    
    ImGui::SameLine();
    label = "STORE AS##%s"+T::getTypeName();
    
    if(ImGui::Button(label.c_str() )) {
        strcpy(newPresetLabel, settings.getLabel().c_str());
        label = "Store "+T::getTypeName()+" Preset As";
        ImGui::OpenPopup(label.c_str());
        
    };
    
    label = "Store "+T::getTypeName()+" Preset As" ;
    if (ImGui::BeginPopup(label.c_str(), 0)){
        
        ImGui::Text("%s", label.c_str());
        ImGui::Separator();
        if(ImGui::InputText("##1", newPresetLabel, IM_ARRAYSIZE(newPresetLabel))){
            
        }
        
        ImGui::Separator();
        label = "OK## "+T::getTypeName();
        if (ImGui::Button(label.c_str(),  ImVec2(100, 0))) {
            string presetlabel = newPresetLabel;
            // TODO CHECK PRESET EXISTS AND ADD POP UP
            addPreset(presetlabel, settings);
            ImGui::CloseCurrentPopup();
            changed = true;
            
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        label = "Cancel## "+T::getTypeName();
        if (ImGui::Button(label.c_str(), ImVec2(100, 0))) {
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::EndPopup();
        
        
    }
    return changed;
    
}
template <typename T>
void PresetManager<T> ::serialize(ofJson& json) const {
    
    for (auto it = presetMap.begin(); it != presetMap.end(); it++) {
        int i = std::distance(std::begin(presetMap), it);
        ofJson presetJson;
        it->second.serialize(presetJson);
        json.push_back(presetJson);
        
    }
    //ofLogNotice(json.dump(3));
}

template <typename T>
bool PresetManager<T> ::deserialize(ofJson& json) {
    
    deleteAllPresets();
    
    for (ofJson::iterator it = json.begin(); it != json.end(); ++it) {
        T preset;
        try {
            preset.deserialize(*it);
            presetMap[preset.getLabel()] = preset;
            savePreset(preset.getLabel(), preset);
        } catch(...) {
            
            ofLogError("PresetManager<T> ::deserialize(ofJson& json) - failed to load preset");
        }
    }
    
    
    return true;
}

    
    
    
}

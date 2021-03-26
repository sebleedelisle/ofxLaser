//
//  ofxLaserUI.hpp
//  UIExperiment
//
//  Created by Seb Lee-Delisle on 23/03/2021.
//

#pragma once
#include "ofxImGui.h"
#include "ofMain.h"
#include "Poco/PriorityDelegate.h"

namespace ofxLaser {

class UI {
    
    public :
    
    static ofxImGui::Gui imGui;
    static ImFont* font;
    static bool initialised; 
    
    static void setupGui();
    static void updateGui();
    static void startGui();
    
    static bool addIntSlider(ofParameter<int>& param);
    static bool addFloatSlider(ofParameter<float>& param, const char* format="%.2f", float power = 1.0f) ;
    static bool addFloatAsIntSlider(ofParameter<float>& param, float multiplier);
    static bool addCheckbox(ofParameter<bool>&param);
    
    static bool addParameter(shared_ptr<ofAbstractParameter>& param);
    
    static void addParameterGroup(ofParameterGroup& parameterGroup);
    
    static void updateMouse(ofMouseEventArgs &e) {
        ImGui::GetIO().MousePos = ImVec2((float)e.x, (float)e.y);
        
    }
    
    static void render();
    
    static void toolTip(const char* desc)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
    
    
    
    
    
    
};




}

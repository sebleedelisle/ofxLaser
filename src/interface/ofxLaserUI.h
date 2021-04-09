//
//  ofxLaserUI.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 23/03/2021.
//

#pragma once
#include "ofxImGui.h"
#include "ofMain.h"
#include "Poco/PriorityDelegate.h"
#include "RobotoMedium.cpp"

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
    static bool addFloat2Slider(ofParameter<glm::vec2>& param, const char* format="%.2f", float power = 1.0f) ;
    static bool addFloat3Slider(ofParameter<glm::vec3>& parameter, const char* format="%.2f", float power = 1.0f);

   
    static bool addFloatAsIntSlider(ofParameter<float>& param, float multiplier);
    static bool addFloatAsIntPercentage(ofParameter<float>& param); 

    static bool addCheckbox(ofParameter<bool>&param);
    
    static bool addColour(ofParameter<ofFloatColor>& parameter, bool alpha = false);
    static bool addColour(ofParameter<ofColor>& parameter, bool alpha = false);

    static bool addParameter(shared_ptr<ofAbstractParameter>& param);
    
    static void addParameterGroup(ofParameterGroup& parameterGroup);
    
    static bool updateMouse(ofMouseEventArgs &e) {
		ImGui::GetIO().MousePos = ImVec2((float)e.x, (float)e.y);
		//ofLogNotice("Mouse updated " + ofToString(ImGui::GetIO().MousePos.x) +" " +ofToString(ImGui::GetIO().MousePos.y));
		return false; // propogate events 
    }
    static bool mousePressed(ofMouseEventArgs &e) {
        
        ImGui::GetIO().MouseDown[e.button] = true;
        //cout << (ImGui::GetIO().WantCaptureMouse)<< endl;
        if(ImGui::GetIO().WantCaptureMouse) {
            ofLogNotice("ImGui captured mouse press");
            return true;
        }
        else {
            ofLogNotice("ImGui no capture mouse press"); 
            return false;
        }
    }
    static bool mouseReleased(ofMouseEventArgs &e) {
        ImGui::GetIO().MouseDown[e.button] = false;
        if(ImGui::GetIO().WantCaptureMouse) return true;
        else return false;
    }
    static bool keyPressed(ofKeyEventArgs &e) {
        ImGui::GetIO().KeysDown[e.key] = true;
        if(ImGui::GetIO().WantCaptureKeyboard) {
            
            ofLogNotice("ImGui captured key press");
            return true;
        }
        else {
            ofLogNotice("ImGui no capture key press");
            return false;
        }
    }
    static bool keyReleased(ofKeyEventArgs &e) {
        ImGui::GetIO().KeysDown[e.key] = false;
        if(ImGui::GetIO().WantCaptureKeyboard) {
          
            return true;
        }
        else return false;
    }
    static void render();
    
    static void toolTip(const char* desc)
    {
        ImGui::SameLine(0,3);
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 15.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
    
    
    
    
    
    
};




}

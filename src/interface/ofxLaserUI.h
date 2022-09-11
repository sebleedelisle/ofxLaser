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
#include "RobotoBold.cpp"
#include <imgui_internal.h>
#include "IconsForkAwesome.h"
#include "ForkAwesome.cpp"

namespace ofxLaser {


class UI {
    
    public :
    
    
    
    static ofxImGui::Gui imGui;
    static ImFont* font;
    static ImFont* mediumFont; 
    static ImFont* largeFont;
    static ImFont* symbolFont;
    static bool initialised;
    
    static bool ghosted;
    static bool secondaryColourActive;
    static bool dangerColourActive;
    static bool largeItemActive; 
    
    static void setupGui();
    static void updateGui();
    static void startGui();
    
    static bool resetButton(string label);
    template <typename T>
    static bool resetButton(T& param, T& resetParam);
    template <typename T, typename T2>
    static bool resetButton(T& param, T2 resetParam);
    
    
     
    static bool addIntSlider(string label, int& target, int min, int max);
    static bool addFloatSlider(string label, float& target, float min, float max, const char* format="%.2f", float power = 1.0f) ;
    static bool addFloat2Slider(string label, glm::vec2& target, glm::vec2 min, glm::vec2 max, const char* format="%.2f", float power = 1.0f) ;
    static bool addFloat3Slider(string label, glm::vec3& target, glm::vec3 min, glm::vec3 max,  const char* format="%.2f", float power = 1.0f);
    static bool addDragSlider(string label, float& target, float speed, float min, float max, const char* format);
    static bool addFloat2Drag(string label, glm::vec2& target, float speed, glm::vec2 min, glm::vec2 max, const char* format="%.2f") ;
    static bool addFloat3Drag(string label, glm::vec3& target, float speed, glm::vec3 min, glm::vec3 max, const char* format="%.2f") ;
  
    
    static bool addFloatAsIntPercentage(string label, float& target, float min = 0, float max = 1);
    static bool addResettableFloatSlider(string label, float& target, float resetValue, float min, float max, string tooltip="", const char* format="%.2f", float power = 1.0f);
    static bool addResettableFloatAsIntPercentage(string label, float& target, float resetValue, float min = 0, float max = 1);
    static bool addNumberedCheckBox(int number, const char* label, bool* v, bool large, bool dangerColour = false);
    static bool addNumberedCheckBox(int number, const string& label, bool* v, bool large = true, bool dangerColour = false);

    
    // ofParameters
    
    static bool addParameter(ofAbstractParameter& param);
    static bool addParameter(shared_ptr<ofAbstractParameter>& param);
    
    static void addParameterGroup(ofParameterGroup& parameterGroup, bool showTitle = true);
    
    static bool addIntSlider(ofParameter<int>& param, string labelSuffix = "");
    static bool addFloatSlider(ofParameter<float>& param, const char* format="%.2f", float power = 1.0f, string labelSuffix = "") ;
    static bool addFloat2Slider(ofParameter<glm::vec2>& param, const char* format="%.2f", float power = 1.0f, string labelSuffix = "") ;
    static bool addFloat3Slider(ofParameter<glm::vec3>& parameter, const char* format="%.2f", float power = 1.0f, string labelSuffix = "");
    static bool addFloatDrag(ofParameter<float>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");
    static bool addFloat2Drag(ofParameter<glm::vec2>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");
    static bool addFloat3Drag(ofParameter<glm::vec3>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");
    
    static bool addRectDrag(ofParameter<ofRectangle>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");

    static bool addFloatAsIntSlider(ofParameter<float>& param, float multiplier, string labelSuffix = "");
    static bool addFloatAsIntPercentage(ofParameter<float>& param, string labelSuffix = "");
    
    static bool addResettableCheckbox(ofParameter<bool>&param, ofParameter<bool>&resetParam, string labelSuffix = "");
    static bool addResettableFloatSlider(ofParameter<float>& param, float resetParam, string tooltip="", const char* format="%.2f", float power = 1.0f, string labelSuffix = "");
    static bool addResettableFloatDrag(ofParameter<float>& param, float resetParam, float speed = 1, string tooltip="", const char* format="%.2f", string labelSuffix = "");
    static bool addResettableFloat2Drag(ofParameter<glm::vec2>& param, ofParameter<glm::vec2>& resetParam, float speed = 1, string tooltip="", const char* format="%.2f", string labelSuffix = "");
    static bool addResettableFloat3Drag(ofParameter<glm::vec3>& param, ofParameter<glm::vec3>& resetParam, float speed = 1, string tooltip="", const char* format="%.2f", string labelSuffix = "");
    static bool addResettableIntSlider(ofParameter<int>& param, int resetParam, string tooltip="", string labelSuffix = "");
    
    static bool addCheckbox(ofParameter<bool>&param, string labelSuffix = "");
    static bool addNumberedCheckbox(int number, ofParameter<bool>&param, string labelSuffix = "", bool large = true);
   
    static bool addColour(ofParameter<ofFloatColor>& parameter, bool alpha = false, string labelSuffix = "");
    static bool addColour(ofParameter<ofColor>& parameter, bool alpha = false, string labelSuffix = "");
    
    
    static void startGhosted() ;
    static void stopGhosted() ;
    
    static bool startWindow(string name, ImVec2 pos, ImVec2 size = ImVec2(0,0), ImGuiWindowFlags flags = 0, bool resetPosition = false, bool* openstate = nullptr) ;
    static void drawRectangle(float x, float y, float w, float h, ofColor colour, bool filled = false, bool fromCentre=false, float thickness = 2.0f) ;
    
    
    
    static ImU32 ofColorToImU32 (ofColor col) ;
    static void endWindow();
    
    
    static bool Button(string label, bool large = false, bool secondaryColour = false, const ImVec2& size_arg = ImVec2(0,0));
    
    static bool Button(const char* label, bool large = false, bool secondaryColour = false, const ImVec2& size_arg = ImVec2(0,0));
    static bool DangerButton(string& label, bool large = false, const ImVec2& size_arg = ImVec2(0,0));
    static void secondaryColourStart();
    static void secondaryColourEnd();
    static void dangerColourStart();
    static void dangerColourEnd() ;
 
    static void largeItemStart();
    static void largeItemEnd() ;
    static void extraLargeItemStart();
    static void extraLargeItemEnd() ;
    static bool updateMouse(ofMouseEventArgs &e);
    static bool mousePressed(ofMouseEventArgs &e) ;
    static bool mouseReleased(ofMouseEventArgs &e) ;
    static bool keyPressed(ofKeyEventArgs &e) ;
    static bool keyReleased(ofKeyEventArgs &e) ;
    static void render();
    
    static void toolTip(string& str);
  
    static void toolTip(const char* desc);
    static void addDelayedTooltip(const char* desc);
    
    static ofMesh dashedLineMesh;

    static void drawDashedLine(glm::vec3 p1, glm::vec3 p2, float spacing = 6, float scale = 1);
    
    static ImU32 getColourForState(int state);
    
    static glm::vec3 getScaleFromMatrix(const glm::mat4& m);
    
    
    
};




}

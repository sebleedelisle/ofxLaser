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
#include "GlobalScale.h"

namespace ofxLaser {

class UI {
    
    public :
    
    static ofxImGui::Gui imGuiOfx;
    static ImFont* font;
    static ImFont* mediumFont; 
    static ImFont* largeFont;
    static ImFont* symbolFont;
    static bool initialised;
    
    static bool ghosted;
    static bool disabled;
    static bool secondaryColourActive;
    static bool dangerColourActive;
    static bool customColourActive;
    static bool largeItemActive; 
    
    static void setupGui();
    static void updateGui();
    static void startGui();
    //static void endGui();
    
    static bool resetButton(string label);
    template <typename T>
    static bool resetButton(T& param, T& resetParam);
    template <typename T, typename T2>
    static bool resetButton(T& param, T2 resetParam);
    
    
     
    static bool addIntSlider(string label, int& target, int min, int max);
    static bool addFloatSlider(string label, float& target, float min, float max, const char* format="%.2f", ImGuiSliderFlags flags = 0) ;
    static bool addFloat2Slider(string label, glm::vec2& target, glm::vec2 min, glm::vec2 max, const char* format="%.2f", ImGuiSliderFlags flags = 0) ;
    static bool addFloat3Slider(string label, glm::vec3& target, glm::vec3 min, glm::vec3 max,  const char* format="%.2f", ImGuiSliderFlags flags = 0);
    static bool addFloatDrag(string label, float& target, float speed, float min, float max, const char* format="%.2f");
    static bool addIntDrag(string label, int& target, float speed, float min, float max, const char* format="%d");
    static bool addIntDragSmall(string label, int& target, float speed, float min, float max, const char* format="%d");
    static bool addFloat2Drag(string label, glm::vec2& target, float speed, glm::vec2 min, glm::vec2 max, const char* format="%.2f") ;
    static bool addFloat3Drag(string label, glm::vec3& target, float speed, glm::vec3 min, glm::vec3 max, const char* format="%.2f") ;
  
    
    static bool addFloatAsIntPercentage(string label, float& target, float min = 0, float max = 1);
    static bool addResettableFloatSlider(string label, float& target, float resetValue, float min, float max, string tooltip="", const char* format="%.2f", ImGuiSliderFlags flags = 0);
    static bool addResettableFloatAsIntPercentage(string label, float& target, float resetValue, float min = 0, float max = 1);
    static bool addNumberedCheckBox(int number, const char* label, bool* v, bool large, bool dangerColour = false);
    static bool addNumberedCheckBox(int number, const string& label, bool* v, bool large = true, bool dangerColour = false);

    static bool addMultiChoiceParam(ofParameter<int>& param, const vector<string>& labels);
    static bool addMultiChoiceInt(string label, int& param, const vector<string>& labels);
    
    
    // ofParameters
    
    static bool addParameter(ofAbstractParameter& param);
    //static bool addParameter(shared_ptr<ofAbstractParameter>& param);
//    static bool addParameter(ofParameter<bool>& param) {
//        return UI::addCheckbox(*parameterBoolPtr);
//    }
    
    
    static bool addParameterGroup(ofParameterGroup& parameterGroup, bool showTitle = true);
    
    
    
    
    static bool addIntSlider(ofParameter<int>& param, string labelSuffix = "");
    static bool addFloatSlider(string label, ofParameter<float>& param, const char* format="%.2f", ImGuiSliderFlags flags = 0) ;
    static bool addFloatSlider(ofParameter<float>& param, const char* format="%.2f", ImGuiSliderFlags flags = 0, string labelSuffix = "") ;
    static bool addFloat2Slider(ofParameter<glm::vec2>& param, const char* format="%.2f", ImGuiSliderFlags flags = 0, string labelSuffix = "") ;
    static bool addFloat3Slider(ofParameter<glm::vec3>& parameter, const char* format="%.2f", ImGuiSliderFlags flags = 0, string labelSuffix = "");
    static bool addFloatDrag(ofParameter<float>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");
    static bool addIntDrag(ofParameter<int>&param, float speed=1, const char* format="%d", string labelSuffix = "");
    static bool addFloatAsIntDrag(ofParameter<float>&param, float multiplier, float speed=1, string labelSuffix="");
    
    static bool addFloat2Drag(ofParameter<glm::vec2>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");
    static bool addFloat3Drag(ofParameter<glm::vec3>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");
    
    static bool addRectDrag(ofParameter<ofRectangle>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");
    static bool addRectDrag(ofRectangle&param, float speed=1, const char* format="%.2f", string labelSuffix = "");

    static bool addFloatAsIntSlider(string label, ofParameter<float>& param, float multiplier);
    static bool addFloatAsIntSlider(ofParameter<float>& param, float multiplier, string labelSuffix = "");
    static bool addFloatAsIntPercentage(ofParameter<float>& param, string labelSuffix = "");
    
    static bool addResettableCheckbox(ofParameter<bool>&param, ofParameter<bool>&resetParam, string labelSuffix = "");
    static bool addResettableFloatSlider(ofParameter<float>& param, float resetParam, string tooltip="", const char* format="%.2f", ImGuiSliderFlags flags = 0, string labelSuffix = "");
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
    static void startDisabled() ;
    static void stopDisabled() ;

    static bool startWindow(string name, ImVec2 pos, ImVec2 size = ImVec2(0,0), ImGuiWindowFlags flags = 0, bool resetPosition = false, bool* openstate = nullptr) ;
    static void drawRectangle(float x, float y, float w, float h, ofColor colour, bool filled = false, bool fromCentre=false, float thickness = 2.0f) ;
    static void drawRectangle(ofColor colour, bool filled = true) ;
   
    
    
    static ImU32 ofColorToImU32 (ofColor col) ;
    static void endWindow();
    
    
    static bool Button(string label, bool large = false, bool secondaryColour = false, const ImVec2& size_arg = ImVec2(0,0));
    
    static bool ToggleButton(string label, bool&value, bool large = false, const ImVec2& size_arg = ImVec2(0,0));
    static bool ToggleButton(ofParameter<bool>&param, string labelSuffix = "");
    static bool ToggleButton(string label, ofParameter<bool>&param);

    static bool Button(const char* label, bool large = false, bool secondaryColour = false, const ImVec2& size_arg = ImVec2(0,0));
    static bool DangerButton(string label, bool large = false, const ImVec2& size_arg = ImVec2(0,0));
    static void secondaryColourStart();
    static void secondaryColourEnd();
    static void dangerColourStart();
    static void dangerColourEnd() ;
    static void customColourStart(ofColor colour);
    static void customColourEnd();
    
    // Variadic functions have to be defined in the header, apparently! 
    template<typename... Args>
    static void TextCentered(std::string text, Args... args) {

        vector<string> lines = ofSplitString(text, "\n");
        if(lines.size()>1) {
            for(string& line : lines) {
                TextCentered(line, args...);
            }
            return;
        }
            
        
        string format = text;
        
        // wow this bullshit just to format the string with the arguments
        int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
        if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
        auto size = static_cast<size_t>( size_s );
        std::unique_ptr<char[]> buf( new char[ size ] );
        std::snprintf( buf.get(), size, format.c_str(), args ... );
        text =  std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
    

        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text(text.c_str());
        
       // va_end(args);
//        float win_width = ImGui::GetWindowSize().x;
//        float text_width = ImGui::CalcTextSize(text.c_str()).x;
//
//        // calculate the indentation that centers the text on one line, relative
//        // to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
//        float text_indentation = (win_width - text_width) * 0.5f;
//
//        // if text is too long to be drawn on one line, `text_indentation` can
//        // become too small or even negative, so we check a minimum indentation
//        float min_indentation = 20.0f;
//        if (text_indentation <= min_indentation) {
//            text_indentation = min_indentation;
//        }
//
//        ImGui::SameLine(text_indentation);
//        ImGui::PushTextWrapPos(win_width - text_indentation);
//        ImGui::TextWrapped(text.c_str());
//        ImGui::PopTextWrapPos();
    }
    static void TextWithColors( const char* fmt, ... );
    static bool ProcessInlineHexColor( const char* start, const char* end, ImVec4& color );

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
    
    static void toolTipWarning(string& str);
    static void toolTipWarning(const char* desc);
    
    static void addHover(string& str);
    static void addDelayedHover(string str);
    static void addHover(const char* desc);
    static void addDelayedTooltip(const char* desc);
    
    static ofMesh dashedLineMesh;

    static void drawDashedLine(glm::vec3 p1, glm::vec3 p2, float spacing = 6, float scale = 1);
    static void drawDashedLine(glm::vec2 p1, glm::vec2 p2, float spacing = 6, float scale = 1);
    
    static ImU32 getColourForState(int state);
    
    static glm::vec3 getScaleFromMatrix(const glm::mat4& m);
    
    static string imguiSavePath; 
    
};




}

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

namespace UI {

// probably need to access these from outside the namespace
extern ofxImGui::Gui imGuiOfx;
extern ImFont* font;
extern ImFont* mediumFont;
extern ImFont* largeFont;
extern ImFont* symbolFont;

void setupGui();
void updateGui();
void startGui();

void setStyles();

bool resetButton(string label);
template <typename T>
bool resetButton(T& param, T& resetParam);
template <typename T, typename T2>
bool resetButton(T& param, T2 resetParam);

bool addIntSlider(string label, int& target, int min, int max);
bool addFloatSlider(string label, float& target, float min, float max, const char* format="%.2f", ImGuiSliderFlags flags = 0) ;
bool addFloat2Slider(string label, glm::vec2& target, glm::vec2 min, glm::vec2 max, const char* format="%.2f", ImGuiSliderFlags flags = 0) ;
bool addFloat3Slider(string label, glm::vec3& target, glm::vec3 min, glm::vec3 max,  const char* format="%.2f", ImGuiSliderFlags flags = 0);
bool addFloatDrag(string label, float& target, float speed, float min, float max, const char* format="%.2f");
bool addIntDrag(string label, int& target, float speed, float min, float max, const char* format="%d");
bool addIntDragSmall(string label, int& target, float speed, float min, float max, const char* format="%d");
bool addFloat2Drag(string label, glm::vec2& target, float speed, glm::vec2 min, glm::vec2 max, const char* format="%.2f") ;
bool addFloat3Drag(string label, glm::vec3& target, float speed, glm::vec3 min, glm::vec3 max, const char* format="%.2f") ;


bool addFloatAsIntPercentage(string label, float& target, float min = 0, float max = 1);
bool addResettableFloatSlider(string label, float& target, float resetValue, float min, float max, string tooltip="", const char* format="%.2f", ImGuiSliderFlags flags = 0);
bool addResettableFloatAsIntPercentage(string label, float& target, float resetValue, float min = 0, float max = 1);
bool addNumberedCheckBox(int number, const char* label, bool* v, bool large, bool dangerColour = false);
bool addNumberedCheckBox(int number, const string& label, bool* v, bool large = true, bool dangerColour = false);

bool addMultiChoiceParam(ofParameter<int>& param, const vector<string>& labels);
bool addMultiChoiceInt(string label, int& param, const vector<string>& labels);

bool addParameter(ofAbstractParameter& param);

bool addParameterGroup(ofParameterGroup& parameterGroup, bool showTitle = true);

bool addIntSlider(ofParameter<int>& param, string labelSuffix = "");
bool addFloatSlider(string label, ofParameter<float>& param, const char* format="%.2f", ImGuiSliderFlags flags = 0) ;
bool addFloatSlider(ofParameter<float>& param, const char* format="%.2f", ImGuiSliderFlags flags = 0, string labelSuffix = "") ;
bool addFloat2Slider(ofParameter<glm::vec2>& param, const char* format="%.2f", ImGuiSliderFlags flags = 0, string labelSuffix = "") ;
bool addFloat3Slider(ofParameter<glm::vec3>& parameter, const char* format="%.2f", ImGuiSliderFlags flags = 0, string labelSuffix = "");
bool addFloatDrag(ofParameter<float>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");
bool addIntDrag(ofParameter<int>&param, float speed=1, const char* format="%d", string labelSuffix = "");
bool addFloatAsIntDrag(ofParameter<float>&param, float multiplier, float speed=1, string labelSuffix="");
bool addFloatAsIntDrag(float&value, float min, float max, float multiplier, float speed=1, string label="");
bool addFloat2Drag(ofParameter<glm::vec2>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");
bool addFloat3Drag(ofParameter<glm::vec3>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");

bool addRectDrag(ofParameter<ofRectangle>&param, float speed=1, const char* format="%.2f", string labelSuffix = "");
bool addRectDrag(ofRectangle&param, float speed=1, const char* format="%.2f", string labelSuffix = "");

bool addFloatAsIntSlider(string label, ofParameter<float>& param, float multiplier);
bool addFloatAsIntSlider(ofParameter<float>& param, float multiplier, string labelSuffix = "");
bool addFloatAsIntPercentage(ofParameter<float>& param, string labelSuffix = "");

bool addResettableCheckbox(ofParameter<bool>&param, ofParameter<bool>&resetParam, string labelSuffix = "");
bool addResettableFloatSlider(ofParameter<float>& param, float resetParam, string tooltip="", const char* format="%.2f", ImGuiSliderFlags flags = 0, string labelSuffix = "");
bool addResettableFloatDrag(ofParameter<float>& param, float resetParam, float speed = 1, string tooltip="", const char* format="%.2f", string labelSuffix = "");
bool addResettableFloat2Drag(ofParameter<glm::vec2>& param, ofParameter<glm::vec2>& resetParam, float speed = 1, string tooltip="", const char* format="%.2f", string labelSuffix = "");
bool addResettableFloat3Drag(ofParameter<glm::vec3>& param, ofParameter<glm::vec3>& resetParam, float speed = 1, string tooltip="", const char* format="%.2f", string labelSuffix = "");
bool addResettableIntSlider(ofParameter<int>& param, int resetParam, string tooltip="", string labelSuffix = "");

bool addCheckbox(ofParameter<bool>&param, string labelSuffix = "");
bool addNumberedCheckbox(int number, ofParameter<bool>&param, string labelSuffix = "", bool large = true);

bool addColour(ofParameter<ofFloatColor>& parameter, bool alpha = false, string labelSuffix = "");
bool addColour(ofParameter<ofColor>& parameter, bool alpha = false, string labelSuffix = "");


void startGhosted() ;
void stopGhosted() ;
void startDisabled() ;
void stopDisabled() ;

bool startWindow(string name, ImVec2 pos, ImVec2 size = ImVec2(0,0), ImGuiWindowFlags flags = 0, bool resetPosition = false, bool* openstate = nullptr) ;
void drawRectangle(float x, float y, float w, float h, ofColor colour, bool filled = false, bool fromCentre=false, float thickness = 2.0f, float rounding = 0.0f) ;
void drawRectangle(ofColor colour, bool filled = true) ;



ImU32 ofColorToImU32 (ofColor col) ;
void endWindow();


bool Button(string label, bool large = false, bool secondaryColour = false, const ImVec2& size_arg = ImVec2(0,0));

bool ToggleButton(string label, bool&value, bool large = false, const ImVec2& size_arg = ImVec2(0,0));
bool ToggleButton(ofParameter<bool>&param, string labelSuffix = "");
bool ToggleButton(string label, ofParameter<bool>&param);

bool Button(const char* label, bool large = false, bool secondaryColour = false, const ImVec2& size_arg = ImVec2(0,0));
bool DangerButton(string label, bool large = false, const ImVec2& size_arg = ImVec2(0,0));
void secondaryColourStart();
void secondaryColourEnd();
void dangerColourStart();
void dangerColourEnd() ;
void customColourStart(ofColor colour);
void customColourEnd();



void TextWithColors( const char* fmt, ... );
bool ProcessInlineHexColor( const char* start, const char* end, ImVec4& color );

void largeItemStart();
void largeItemEnd() ;
void extraLargeItemStart();
void extraLargeItemEnd() ;
//bool updateMouse(ofMouseEventArgs &e);
//bool mousePressed(ofMouseEventArgs &e) ;
//bool mouseReleased(ofMouseEventArgs &e) ;
bool keyPressed(ofKeyEventArgs &e) ;
bool keyReleased(ofKeyEventArgs &e) ;
void render();

void toolTip(string& str);
void toolTip(const char* desc);

void toolTipWarning(string& str);
void toolTipWarning(const char* desc);

void addHover(string& str);
void addDelayedHover(string str);
void addHover(const char* desc);
void addDelayedTooltip(const char* desc);



void drawDashedLine(glm::vec3 p1, glm::vec3 p2, float spacing = 6, float scale = 1);
void drawDashedLine(glm::vec2 p1, glm::vec2 p2, float spacing = 6, float scale = 1);

ImU32 getColourForState(int state);

glm::vec3 getScaleFromMatrix(const glm::mat4& m);

void drawImGuiTexture(GLuint& textureid, int x, int y, int w, int h, bool sameLine = false);


void shiftCursorY(float distance = 10);
void shiftCursorX(float distance = 10);

void ColumnSeparator();

// Variadic functions have to be defined in the header, apparently!

template<typename... Args>
void TextCentered(std::string text, Args&&... args){
    
    vector<string> lines = ofSplitString(text, "\n");
    if(lines.size()>1) {
        for(string& line : lines) {
            TextCentered(line, args...);
        }
        return;
    }
    
    
    string format = text;
    
    // wow this bullshit just to format the string with the arguments
    int size_s = std::snprintf( nullptr, 0, format.c_str(), std::forward<Args>(args)... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), std::forward<Args>(args) ... );
    text =  std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
    
    
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;
    
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text(text.c_str());
    
}

void OpenConfirmationPopup(const std::string& message, std::function<void()> onOk);
void RenderConfirmationPopup() ;




}
}

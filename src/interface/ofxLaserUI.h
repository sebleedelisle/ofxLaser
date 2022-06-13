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
    static ImFont* largeFont;
    static ImFont* symbolFont;
    static bool initialised;
    
    static bool ghosted;
    
    static void setupGui();
    static void updateGui();
    static void startGui();
    
    static bool addIntSlider(ofParameter<int>& param);
    static bool addFloatSlider(ofParameter<float>& param, const char* format="%.2f", float power = 1.0f) ;
    static bool addFloat2Slider(ofParameter<glm::vec2>& param, const char* format="%.2f", float power = 1.0f) ;
    static bool addFloat3Slider(ofParameter<glm::vec3>& parameter, const char* format="%.2f", float power = 1.0f, string name = "");

   
    static bool addFloatAsIntSlider(ofParameter<float>& param, float multiplier);
    static bool addFloatAsIntPercentage(ofParameter<float>& param);
    
    static bool addResettableFloatSlider(ofParameter<float>& param, float resetParam, string tooltip="", const char* format="%.2f", float power = 1.0f);
    static bool addResettableIntSlider(ofParameter<int>& param, int resetParam, string tooltip="");

    

    static bool addCheckbox(ofParameter<bool>&param);
    static bool addNumberedCheckbox(int number, ofParameter<bool>&param);
    
    static bool NumberedCheckBox(int number, const char* label, bool* v);
    
    static bool addColour(ofParameter<ofFloatColor>& parameter, bool alpha = false);
    static bool addColour(ofParameter<ofColor>& parameter, bool alpha = false);
    
    static bool addParameter(ofAbstractParameter& param);
    static bool addParameter(shared_ptr<ofAbstractParameter>& param);
    
    static void addParameterGroup(ofParameterGroup& parameterGroup, bool showTitle = true);
    
    static void startGhosted() {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
        ghosted = true;
    }
    static void stopGhosted() {
        if(ghosted) {
            ImGui::PopStyleVar();
            ghosted = false;
        } 
    }
    
    static bool startWindow(string name, ImVec2 pos, ImVec2 size = ImVec2(0,0), ImGuiWindowFlags flags = 0, bool resetPosition = false, bool* openstate = nullptr) {
        ImGuiWindowFlags window_flags = flags;
        window_flags |= ImGuiWindowFlags_NoNav;
        //      if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
        //      if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
        //      if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
        //
        //
        //      if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
        //      if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
        //      if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
        //      if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        //      if (no_docking)         window_flags |= ImGuiWindowFlags_NoDocking;
        //      if (no_close)           p_open = NULL; // Don't pass our bool* to Begin
        
        
        // set the main window size and position
        ImGui::SetNextWindowSize(size, ImGuiCond_Once);
        ImGui::SetNextWindowPos(pos, resetPosition ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
        
        // start the main window!
        return ImGui::Begin(name.c_str(), openstate, window_flags);
        
        
    }
    
    static void drawRectangle(float x, float y, float w, float h, ofColor colour, bool filled = false, bool fromCentre=false, float thickness = 2.0f) {
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ofRectangle rect(x, y, w, h);
        if(fromCentre) rect.setFromCenter(x, y, w, h);
        
        ImU32 imCol = ofColorToImU32(colour);
        
        if(filled) {
            draw_list->AddRectFilled(ImVec2(rect.getLeft(), rect.getTop()), ImVec2(rect.getRight(), rect.getBottom()), imCol, 0.0f,  0);
        } else {
            draw_list->AddRect(ImVec2(rect.getLeft(), rect.getTop()), ImVec2(rect.getRight(), rect.getBottom()), imCol, 0.0f,  0, thickness);
        }
    }
    
    
    
    static ImU32 ofColorToImU32 (ofColor col) {
        return ImGui::GetColorU32(ImVec4((float)col.r/255.0f, (float)col.g/255.0f, (float)col.b/255.0f, (float)col.a/255.0f));
        
    }
    static void endWindow() {
        ImGui::End();
    }
    
    
    static bool Button(string& label, bool large = false, bool secondaryColour = false, const ImVec2& size_arg = ImVec2(0,0)){
       
        return Button(label.c_str(), large, secondaryColour, size_arg);
    }
    
    static bool Button(const char* label, bool large = false, bool secondaryColour = false, const ImVec2& size_arg = ImVec2(0,0)){
        bool returnvalue;
        
        if(large) UI::largeItemStart();
        if(secondaryColour) UI::secondaryColourButtonStart();
        returnvalue =  ImGui::ButtonEx(label, size_arg, 0);
        
        if(large) UI::largeItemEnd();
        if(secondaryColour) UI::secondaryColourButtonEnd();
        return returnvalue;
    }
    static void secondaryColourButtonStart() {
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
        
        ImGui::PushStyleColor(ImGuiCol_CheckMark, (ImVec4)ImColor::HSV(0.0f, 0.0f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.8f));
        
    }
    
    static void secondaryColourButtonEnd() {
        ImGui::PopStyleColor(7);
    }
    
    static void largeItemStart() {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 10.0f)); // 3 Size of elements (padding around contents);
        // increase the side of the slider grabber
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 26.0f); // 4 minimum size of slider grab
        
    }
    static void largeItemEnd() {
        
        ImGui::PopStyleVar(2);
    }
    static void extraLargeItemStart() {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(78.0f, 80.0f)); // 3 Size of elements (padding around contents);
        // increase the side of the slider grabber
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 26.0f); // 4 minimum size of slider grab
        
    }
    static void extraLargeItemEnd() {
        
        ImGui::PopStyleVar(2);
    }
    static bool updateMouse(ofMouseEventArgs &e) {
		ImGui::GetIO().MousePos = ImVec2((float)e.x, (float)e.y);
		//ofLogNotice("Mouse updated " + ofToString(ImGui::GetIO().MousePos.x) +" " +ofToString(ImGui::GetIO().MousePos.y));
		return false; // propogate events 
    }
    static bool mousePressed(ofMouseEventArgs &e) {
        int iobutton = e.button;
        if(iobutton == 2) iobutton = 1; // 1 is right click in imgui
        ImGui::GetIO().MouseDown[iobutton] = true;
        //cout << (ImGui::GetIO().WantCaptureMouse)<< endl;
        if(ImGui::GetIO().WantCaptureMouse) {
            //ofLogNotice("ImGui captured mouse press");
            return true;
          
        }
        else {
            //ofLogNotice("ImGui no capture mouse press");
            return false;
        }
    }
    static bool mouseReleased(ofMouseEventArgs &e) {
        int iobutton = e.button;
        if(iobutton == 2) iobutton = 1; // 1 is right click in imgui
        ImGui::GetIO().MouseDown[iobutton] = false;
        if(ImGui::GetIO().WantCaptureMouse) return true;
        else return false;
    }
    static bool keyPressed(ofKeyEventArgs &e) {
       // ImGui::GetIO().KeysDown[e.key] = true;
        
        if(ImGui::GetIO().WantCaptureKeyboard) {
            
           // ofLogNotice("ImGui captured key press");
            return true;
        }
        else {
            //ofLogNotice("ImGui no capture key press");
            return false;
        }
    }
    static bool keyReleased(ofKeyEventArgs &e) {
        // TODO check but I think this happens twice...
       // ImGui::GetIO().KeysDown[e.key] = false;
        if(ImGui::GetIO().WantCaptureKeyboard) {
            return false;
        }
        else return false;
    }
    static void render();
    
    static void toolTip(string& str) {
        toolTip(str.c_str());
    }
  
    static void toolTip(const char* desc)
    {
        ImGui::SameLine(0,3);
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered() )
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 15.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
    static void addDelayedTooltip(const char* desc) {
        if (ImGui::IsItemHovered() && (GImGui->HoveredIdTimer >1)) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 15.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
        
    }
    
    static ofMesh dashedLineMesh;

    static void drawDashedLine(glm::vec3 p1, glm::vec3 p2, float spacing = 6, float scale = 1);
    
    
    
    static ImU32 getColourForState(int state) {
        const ImVec4 stateCols[] = {{0,1,0,1}, {1,1,0,1}, {1,0,0,1}};
        return ImGui::GetColorU32(stateCols[state]);
    }
    
    static glm::vec3 getScaleFromMatrix(const glm::mat4& m) {
        glm::vec3 pos;
        glm::quat rot;
        glm::vec3 scale;
        
        pos = m[3];
        for(int i = 0; i < 3; i++)
            scale[i] = glm::length(glm::vec3(m[i]));
        const glm::mat3 rotMtx(
            glm::vec3(m[0]) / scale[0],
            glm::vec3(m[1]) / scale[1],
            glm::vec3(m[2]) / scale[2]);
        rot = glm::quat_cast(rotMtx);
        return scale;
    }
    
    
    
};




}

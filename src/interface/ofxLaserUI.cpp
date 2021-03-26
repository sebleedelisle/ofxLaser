//
//  ofxLaserUI.cpp
//  UIExperiment
//
//  Created by Seb Lee-Delisle on 23/03/2021.
//

#include "ofxLaserUI.h"


using namespace ofxLaser;

ofxImGui::Gui ofxLaser::UI::imGui;
 ImFont* ofxLaser::UI::font;
bool ofxLaser::UI::initialised = false;

void UI::render() {
    ImGui::Render();
    
}


void UI::setupGui() {
    
    if(initialised) {
        ofLogError("UI::setupGui has been called more than once and it should not be");
        throw;
       
    }
    
    ImGuiIO& io = ImGui::GetIO();
    // io.Fonts->AddFontDefault();
    font  = io.Fonts->AddFontFromFileTTF(ofToDataPath("verdana.ttf", true).c_str(),13);
    io.Fonts->Build();
    
    
    ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO();
    
    io.DisplaySize = ImVec2((float)ofGetWidth(), (float)ofGetHeight());
    io.MouseDrawCursor = false;
    
    bool autoDraw = true;
    imGui.engine.setup(autoDraw);
    
    
    //ofxImGui::DefaultTheme* defaultTheme = new ofxImGui::DefaultTheme();
    //setTheme((BaseTheme*)defaultTheme);
    //defaultTheme->setup();
    
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    
    ImGui::GetIO().MouseDrawCursor = false;
    
    // TODO remove on close down ? 
    ofEvents().mouseMoved.add(&UI::updateMouse,OF_EVENT_ORDER_APP);
    ofEvents().mouseDragged.add(&UI::updateMouse,OF_EVENT_ORDER_APP);
    // ofAddListener(ofEvents().mouseDragged, this, &Zone::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
}

void UI::updateGui() {
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = ofGetLastFrameTime();
    
    // Update settings
    // todo make dependent of OS
    ImGui::GetIO().KeyCtrl = ofGetKeyPressed(OF_KEY_CONTROL);
    io.MouseDown[0] = ofGetMousePressed();
    for (int i = 1; i < 5; i++) {
        io.MouseDown[i] = imGui.engine.mousePressed[i];
    }
}

void UI::startGui() {
    
    ImGui::NewFrame();
    
    //ImGui::ShowStyleEditor() ;
    ImGui::ShowDemoWindow();
    //ImGui::Show;
    
    
}

bool UI::addIntSlider(ofParameter<int>& param) {
    if(ImGui::SliderInt(param.getName().c_str(), (int*)&param.get(), param.getMin(), param.getMax(), "%d")){
        param.set(param.get());
        return true;
    } else {
        return false;
    }
}
bool UI::addFloatSlider(ofParameter<float>& param, const char* format, float power) {
    if(ImGui::SliderFloat(param.getName().c_str(), (float*)&param.get(), param.getMin(), param.getMax(),format, power)){
        param.set(param.get());
        return true;
    } else {
        return false;
    }
}
bool UI::addFloatAsIntSlider(ofParameter<float>& param, float multiplier) {
    int value = param*multiplier;
    if (ImGui::SliderInt(param.getName().c_str(), &value, param.getMin()*multiplier, param.getMax()*multiplier, "%d")) {
        param.set((float)value/multiplier);
        return true;
    } else {
        return false;
    }
}
bool UI::addCheckbox(ofParameter<bool>&param) {
    if(ImGui::Checkbox(param.getName().c_str(), (bool*)&param.get())) {
        param.set(param.get()); // trigger the events
        return true;
    } else {
        return false;
    }
}

bool UI::addParameter(shared_ptr<ofAbstractParameter>& param) {
    auto parameterBoolPtr = std::dynamic_pointer_cast<ofParameter<bool>>(param);
    if(parameterBoolPtr) {
        return UI::addCheckbox(*parameterBoolPtr);
    }
    
    auto parameterFloat = std::dynamic_pointer_cast<ofParameter<float>>(param);
    if(parameterFloat) {
        return UI::addFloatSlider(*parameterFloat);
    }
    
    auto parameterInt = std::dynamic_pointer_cast<ofParameter<int>>(param);
    if(parameterInt) {
        return UI::addIntSlider(*parameterInt);
    }
    // throw error here?
    return false;
    
}

void UI::addParameterGroup(ofParameterGroup& parameterGroup) {
    for(auto& param : parameterGroup) {
        addParameter(param);
        
    }
    
}

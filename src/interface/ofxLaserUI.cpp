//
//  ofxLaserUI.cpp
//  ofxLaser
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
    io.Fonts->AddFontFromMemoryCompressedTTF(&RobotoMedium_compressed_data,RobotoMedium_compressed_size, 13);
//    font  = io.Fonts->AddFontFromFileTTF(ofToDataPath("verdana.ttf", true).c_str(),13);
//    font  = io.Fonts->AddFontFromFileTTF(ofToDataPath("DroidSans.ttf", true).c_str(),13);
//    font  = io.Fonts->AddFontFromFileTTF(ofToDataPath("Karla-Regular.ttf", true).c_str(),13);
//    font  = io.Fonts->AddFontFromFileTTF(ofToDataPath("Cousine-Regular.ttf", true).c_str(),13);
//    io.Fonts->Build();
    ImGui::StyleColorsDark();
   
    ImGui::GetStyle().WindowRounding = 1.0f;
    ImGui::GetStyle().IndentSpacing = 0.0f;
    ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f,0.0f,0.4f);
   // ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDarkening]
    ImGui::GetStyle().ItemSpacing = ImVec2(8.0f,5.0f);
    ImGui::GetStyle().ItemInnerSpacing = ImVec2(6.0f,6.0f);
    
    ImGui::CreateContext();
    
    io.DisplaySize = ImVec2((float)ofGetWidth(), (float)ofGetHeight());
    io.MouseDrawCursor = false;
    
    bool autoDraw = true;
    imGui.engine.setup(autoDraw);
    
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    
    ImGui::GetIO().MouseDrawCursor = false;
    
    // TODO remove on close down ? 
    ofEvents().mouseMoved.add(&UI::updateMouse, OF_EVENT_ORDER_BEFORE_APP);
    ofEvents().mouseDragged.add(&UI::updateMouse, OF_EVENT_ORDER_BEFORE_APP);
    ofEvents().mousePressed.add(&UI::mousePressed,OF_EVENT_ORDER_BEFORE_APP);
    ofEvents().mouseReleased.add(&UI::mouseReleased,OF_EVENT_ORDER_BEFORE_APP);
  //  ofEvents().keyPressed.add(&UI::keyPressed,OF_EVENT_ORDER_BEFORE_APP);
  //  ofEvents().keyReleased.add(&UI::keyReleased,OF_EVENT_ORDER_BEFORE_APP);
}

void UI::updateGui() {
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = ofGetLastFrameTime();
    
    // Update settings
    // todo make dependent of OS
#ifdef TARGET_OSX
    ImGui::GetIO().KeyCtrl = ofGetKeyPressed(OF_KEY_COMMAND);
#else
    ImGui::GetIO().KeyCtrl = ofGetKeyPressed(OF_KEY_CONTROL);
#endif
}

void UI::startGui() {
    
    ImGui::NewFrame();
    
    //ImGui::ShowStyleEditor() ;
    //ImGui::ShowDemoWindow();
    
  
}

bool UI::addResettableFloatSlider(ofParameter<float>& param, float resetParam, string tooltip, const char* format, float power){
    
    bool returnvalue = UI::addFloatSlider(param, format, power);
    if(tooltip!="") UI::toolTip(tooltip);
    //cout << param.getName()<< " " << param << " " << resetParam.getName() << " " << resetParam << endl; 
    if(param!=resetParam) {
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        if (ImGui::Button("R")) param.set(resetParam);
    }
	return returnvalue; 
}
bool UI::addResettableIntSlider(ofParameter<int>& param, int resetParam, string tooltip){
    
    bool returnvalue = UI::addIntSlider(param);
    if(tooltip!="") UI::toolTip(tooltip);
    if(param!=resetParam) {
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        if (ImGui::Button("R")) param.set(resetParam);
    }
	return returnvalue; 
}

bool UI::addIntSlider(ofParameter<int>& param) {
    
    string label = param.getName();
    ofParameterGroup parent = param.getFirstParent();
    if(parent) label = label+"##"+parent.getName();
    
    if(ImGui::SliderInt(label.c_str(), (int*)&param.get(), param.getMin(), param.getMax(), "%d")){
        param.set(ofClamp(param.get(), param.getMin(), param.getMax()));
        return true;
    } else {
        return false;
    }
}
bool UI::addFloatSlider(ofParameter<float>& param, const char* format, float power) {
    string label = param.getName();
    ofParameterGroup parent = param.getFirstParent();
    label = label+"##"+parent.getName();

    if(ImGui::SliderFloat(label.c_str(), (float*)&param.get(), param.getMin(), param.getMax(),format, power)){
        param.set(ofClamp(param.get(), param.getMin(), param.getMax()));
        return true;
    } else {
        return false;
    }
}

bool UI::addFloat2Slider(ofParameter<glm::vec2>& parameter, const char* format, float power) {
    
    auto tmpRef = parameter.get();
    if (ImGui::SliderFloat2(parameter.getName().c_str(), glm::value_ptr(tmpRef), parameter.getMin().x, parameter.getMax().x, format, power)) {
        parameter.set(tmpRef);
        return true;
    }
    return false;
}

bool UI::addFloat3Slider(ofParameter<glm::vec3>& parameter, const char* format, float power) {
    
    auto tmpRef = parameter.get();
    if (ImGui::SliderFloat3(parameter.getName().c_str(), glm::value_ptr(tmpRef), parameter.getMin().x, parameter.getMax().x, format, power)) {
        parameter.set(tmpRef);
        return true;
    }
    return false;
}

bool UI::addColour(ofParameter<ofFloatColor>& parameter, bool alpha) {
    
    auto tmpRef = parameter.get();
    if (alpha)
    {
        if (ImGui::ColorEdit4(parameter.getName().c_str(), &tmpRef.r, ImGuiColorEditFlags_DisplayHSV))
        {
            parameter.set(tmpRef);
            return true;
        }
    }
    else if (ImGui::ColorEdit3(parameter.getName().c_str(), &tmpRef.r, ImGuiColorEditFlags_DisplayHSV))
    {
        parameter.set(tmpRef);
        return true;
    }
    
    return false;
}
bool UI::addColour(ofParameter<ofColor>& parameter, bool alpha) {
    
    ofFloatColor tmpRef = parameter.get();
    if (alpha)
    {
        if (ImGui::ColorEdit4(parameter.getName().c_str(), &tmpRef.r, ImGuiColorEditFlags_DisplayHSV))
        {
            parameter.set(tmpRef);
            return true;
        }
    }
    else if (ImGui::ColorEdit3(parameter.getName().c_str(), &tmpRef.r, ImGuiColorEditFlags_DisplayHSV))
    {
        parameter.set(tmpRef);
        return true;
    }
    
    return false;
}
bool UI::addFloatAsIntSlider(ofParameter<float>& param, float multiplier) {
    int value = param*multiplier;
    if (ImGui::SliderInt(param.getName().c_str(), &value, param.getMin()*multiplier, param.getMax()*multiplier, "%d")) {
        param.set(ofClamp((float)value/multiplier, param.getMin(), param.getMax()));
        return true;
    } else {
        return false;
    }
}
bool UI::addFloatAsIntPercentage(ofParameter<float>& param) {
    float multiplier = 100.0f;
    int value = param*multiplier;
    if (ImGui::SliderInt(param.getName().c_str(), &value, param.getMin()*multiplier, param.getMax()*multiplier, "%d%%")) {
        param.set(ofClamp((float)value/multiplier, param.getMin(), param.getMax()));
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
    
    auto parameterVec2 = std::dynamic_pointer_cast<ofParameter<glm::vec2>>(param);
    if(parameterVec2) {
        return UI::addFloat2Slider(*parameterVec2);
    }
    auto parameterVec3 = std::dynamic_pointer_cast<ofParameter<glm::vec3>>(param);
    if(parameterVec3) {
        return UI::addFloat3Slider(*parameterVec3);
    }
    
    auto parameterFloatColour = std::dynamic_pointer_cast<ofParameter<ofFloatColor>>(param);
    if (parameterFloatColour){
        return UI::addColour(*parameterFloatColour);
    }
    
    auto parameterColour = std::dynamic_pointer_cast<ofParameter<ofColor>>(param);
    if (parameterColour){
        return UI::addColour(*parameterColour);
    }
    
    auto parameterString = std::dynamic_pointer_cast<ofParameter<string>>(param);
    if (parameterString){
        vector<string> lines = ofSplitString(parameterString->get(), "\n");
        for(string& line : lines) {
            ImGui::Text("%s", line.c_str()); 
        }
        return true;
    }
    // throw error here?
    return false;
    
}

void UI::addParameterGroup(ofParameterGroup& parameterGroup) {
    for(auto& param : parameterGroup) {
        addParameter(param);
        
    }
    
}

ofMesh UI::dashedLineMesh;
void UI::drawDashedLine(glm::vec3 p1, glm::vec3 p2){
   UI::dashedLineMesh.clear();
   
   float l = glm::length(p2-p1);

   for(int p = 0; p<l; p+=6) {
       UI::dashedLineMesh.addVertex(glm::mix(p1, p2, ofMap(p,0,l,0,1)));
   }
   ofPushStyle();
   ofNoFill();
//   ofSetColor(colour);
   ofSetLineWidth(1);

//   UI::dashedLineMesh.setMode(OF_PRIMITIVE_LINES);
    UI::dashedLineMesh.setMode(OF_PRIMITIVE_POINTS);
   UI::dashedLineMesh.draw();
   ofPopStyle();

}


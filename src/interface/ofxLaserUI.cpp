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
ImFont* ofxLaser::UI::largeFont;
ImFont* ofxLaser::UI::symbolFont;
bool ofxLaser::UI::initialised = false;
bool ofxLaser::UI::ghosted = false;

void UI::render() {
    ImGui::Render();
    
}


void UI::setupGui() {
    
    if(initialised) {
        ofLogError("UI::setupGui has been called more than once and it should not be");
        throw;
       
    }
    
    ImGuiIO& io = ImGui::GetIO();
    font = io.Fonts->AddFontFromMemoryCompressedTTF(&RobotoMedium_compressed_data, RobotoMedium_compressed_size, 13);
    
    //symbolFont->
    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13;
    static const ImWchar icon_ranges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
   // symbolFont = io.Fonts->AddFontFromFileTTF(ofToDataPath("forkawesome-webfont.ttf").c_str(), 13, &config, icon_ranges);
    symbolFont = io.Fonts->AddFontFromMemoryCompressedTTF(&ForkAwesome_compressed_data, ForkAwesome_compressed_size,13, &config, icon_ranges);
    
    largeFont = io.Fonts->AddFontFromMemoryCompressedTTF(&RobotoBold_compressed_data, RobotoBold_compressed_size, 24);
    
    //    largeFont = io.Fonts->AddFontFromFileTTF(ofToDataPath("Roboto/Roboto-Bold.ttf").c_str(), 24);
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
    //ofEvents().keyPressed.add(&UI::keyPressed,OF_EVENT_ORDER_BEFORE_APP);
    //ofEvents().keyReleased.add(&UI::keyReleased,OF_EVENT_ORDER_BEFORE_APP);
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
   
    if(param!=resetParam) {
        
        string label = param.getName();
        if(param.getFirstParent()) label = label+param.getFirstParent().getName();
        
        if(resetButton(label)) {
            param.set(resetParam);
            returnvalue = true;
        }
    }
	return returnvalue; 
}

bool UI::addResettableFloatSlider(string label, float& target, float resetValue, float min, float max, string tooltip, const char* format, float power){
    
    bool returnvalue = UI::addFloatSlider(label, target, min, max, format, power);
    if(tooltip!="") UI::toolTip(tooltip);

    if(target!=resetValue) {

       if(resetButton(label)) {
            target = resetValue;
           returnvalue = true;
        }
    }
    return returnvalue;
}


bool UI::addResettableIntSlider(ofParameter<int>& param, int resetParam, string tooltip){
    
    bool returnvalue = UI::addIntSlider(param);
    if(tooltip!="") UI::toolTip(tooltip);
    if(param!=resetParam) {
        
        string label = param.getName();
        if(param.getFirstParent()) label = label+param.getFirstParent().getName();
        
        if(resetButton(label)) {
            param.set(resetParam);
            returnvalue = true;
        }
    }
	return returnvalue; 
}
bool UI::addIntSlider(string label, int& target, int min, int max){
    
    return ImGui::SliderInt(label.c_str(), (int*)&target, min, max, "%d");
    
}
bool UI::addFloatSlider(string label, float& target, float min, float max, const char* format, float power) {
    return ImGui::SliderFloat(label.c_str(), &target, min, max, format, power);
}

bool UI::addFloat2Slider(string label, glm::vec2& target, glm::vec2 min, glm::vec2 max, const char* format, float power){
    return ImGui::SliderFloat2(label.c_str(), glm::value_ptr(target), min.x, max.x, format, power);
    
}
bool UI::addFloat3Slider(string label, glm::vec3& target, glm::vec3 min, glm::vec3 max,  const char* format, float power, string name){
    return ImGui::SliderFloat3(label.c_str(), glm::value_ptr(target), min.x, max.x, format, power);
}

bool UI::addIntSlider(ofParameter<int>& param) {
    
    string label = param.getName();
    ofParameterGroup parent = param.getFirstParent();
    if(parent) label = label+"##"+parent.getName();
    int value = param.get();
    if(addIntSlider(label, value, param.getMin(), param.getMax())) {
        param.set(ofClamp(value, param.getMin(), param.getMax()));
        return true;
    } else {
        return false;
    }
}
bool UI::addFloatSlider(ofParameter<float>& param, const char* format, float power) {
    string label = param.getName();
    ofParameterGroup parent = param.getFirstParent();
    if(parent) label = label+"##"+parent.getName();
    float value =param.get();
    if(addFloatSlider(label, value, param.getMin(), param.getMax(), format, power)){
        param.set(ofClamp(value, param.getMin(), param.getMax()));
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

bool UI::addFloat3Slider(ofParameter<glm::vec3>& parameter, const char* format, float power, string label) {
    if(label == "") label =parameter.getName();
    glm::vec3 tmp = parameter.get();
    if(addFloat3Slider(label, tmp, parameter.getMin(), parameter.getMax(), format, power)){
    //if (ImGui::SliderFloat3(label.c_str(), glm::value_ptr(tmpRef), parameter.getMin().x, parameter.getMax().x, format, power)) {
        parameter.set(tmp);
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

bool UI::resetButton(string label) {
    
    label = ofToString(ICON_FK_UNDO)+"##"+label;
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    return (ImGui::Button(label.c_str()));
        
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

bool UI::addFloatAsIntPercentage(string label, float& target, float min, float max) {
    float multiplier = 100.0f;
    int value = target*multiplier;
    if (ImGui::SliderInt(label.c_str(), &value, min*multiplier, max*multiplier, "%d%%")) {
        target =ofClamp((float)value/multiplier,min,max);
        return true;
    } else {
        return false;
    }
}

bool UI::addResettableFloatAsIntPercentage(string label, float& target, float resetvalue, float min, float max) {
    
    bool changed =addFloatAsIntPercentage(label, target, min, max);
    
    if((target!=resetvalue) && (resetButton(label))) {
        target = resetvalue;
        changed = true;
    }
    return changed;
}

bool UI::addCheckbox(ofParameter<bool>&param) {
    if(ImGui::Checkbox(param.getName().c_str(), (bool*)&param.get())) {
        param.set(param.get()); // trigger the events
        return true;
    } else {
        return false;
    }
}

bool UI::addNumberedCheckbox(int number, ofParameter<bool>&param) {
    if(NumberedCheckBox(number, param.getName().c_str(), (bool*)&param.get())) {
        param.set(param.get()); // trigger the events
        return true;
    } else {
        return false;
    }
    
}

bool UI::NumberedCheckBox(int number, const char* label, bool* v){
    
    using namespace ImGui;
    
    bool useSecondaryColour = *v;
    

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    
    if (useSecondaryColour) {
        secondaryColourButtonStart();
    }
    
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id)) {
        if (useSecondaryColour) {
            secondaryColourButtonEnd();
        }
        return false;
    }
  
    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
    RenderNavHighlight(total_bb, id);
    

    RenderFrame(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);


    ImU32 check_col = GetColorU32(ImGuiCol_CheckMark);
//    if (window->DC.ItemFlags & ImGuiItemFlags_MixedValue)
//    {
//        // Undocumented tristate/mixed/indeterminate checkbox (#2644)
//        ImVec2 pad(ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)), ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)));
//        window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad, check_col, style.FrameRounding);
//    }
//    else if (*v)
//    {
//        const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
//        RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad*2.0f);
//
//    }
    
    // BIG NUMBER IN CHECK BOX
    string numString = ofToString(number).c_str();
    ImGui::PushFont(largeFont);
    const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
    ImVec2 textArea   = ImGui::CalcTextSize(numString.c_str());
    RenderText(check_bb.GetCenter() - (textArea*0.5f), numString.c_str());
    ImGui::PopFont();
    
    if (g.LogEnabled)
        LogRenderedText(&total_bb.Min, *v ? "[x]" : "[ ]");
    if (label_size.x > 0.0f)
        RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y), label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
    if (useSecondaryColour) {
        secondaryColourButtonEnd();
    }
    return pressed;
}


bool UI::addParameter(ofAbstractParameter& param) {
    shared_ptr<ofAbstractParameter> ref = param.newReference();
    return addParameter(ref);
}
bool UI::addParameter(shared_ptr<ofAbstractParameter>& param) {
    
    auto parameterGroupPtr = std::dynamic_pointer_cast<ofParameterGroup>(param);
    if(parameterGroupPtr) {
        
        
        //    bool treevisible = ImGui::TreeNode(parameterGroupPtr->getName().c_str());
        //    if (treevisible){
        
        
        
        for(auto& param : *parameterGroupPtr) {
            addParameter(param);
        }

    
        
        
        //       ImGui::TreePop();
     //   }
//        } else {
//            for(auto& param : *parameterGroupPtr) {
//                addParameter(param);
//            }
//
//        }
            
    }
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
       // if(parameterString->getName()=="") {
            vector<string> lines = ofSplitString(parameterString->get(), "\n");
            for(string& line : lines) {
                ImGui::Text("%s", line.c_str());
            }
       // } else {
            //vector<string> lines = ofSplitString(parameterString->get(), "\n");
            //if(lines.size>1) {
            //char *buf = *parameterString.get().c_str();
           // char* str = parameterString->get().c_str();
            
            //string inputtext = parameterString->get();
            //ImGui::InputTextMultiline("Text", &inputtext);
            
        //}
        
        return true;
    }
    // throw error here?
    return false;
    
}

void UI::addParameterGroup(ofParameterGroup& parameterGroup, bool showTitle){
    
    if(showTitle) {
        ImGui::Separator();
        if((parameterGroup.getName()!="")) {
            ImGui::Text("%s", parameterGroup.getName().c_str());
        }
         
    }
    
    addParameter(parameterGroup);
//    for(auto& param : parameterGroup) {
//        addParameter(param);
//
//    }
//
}

ofMesh UI::dashedLineMesh;
void UI::drawDashedLine(glm::vec3 p1, glm::vec3 p2, float spacing, float scale){
    
    UI::dashedLineMesh.clear();
    
    float l = glm::length(p2-p1);
    
    
    spacing/=scale; 
    float dotsize = ofGetStyle().lineWidth / scale; // assumes proportional scaling
    for(float p = 0; p<l; p+=spacing) {
        UI::dashedLineMesh.addVertex(glm::mix(p1, p2, ofMap(p,0,l,0,1)));
        UI::dashedLineMesh.addVertex(glm::mix(p1, p2, ofMap(p+dotsize,0,l,0,1)));
    }
    ofPushStyle();
    ofNoFill();
    //   ofSetColor(colour);
    //ofSetLineWidth(5);
    
    UI::dashedLineMesh.setMode(OF_PRIMITIVE_LINES);
    //UI::dashedLineMesh.setMode(OF_PRIMITIVE_POINTS);
    UI::dashedLineMesh.draw();
    ofPopStyle();

}



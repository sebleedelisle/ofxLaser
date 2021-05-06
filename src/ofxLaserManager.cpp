//
//  ofxLaserManager.cpp
//  example_HelloLaser 2
//
//  Created by Seb Lee-Delisle on 06/05/2021.
//

#include "ofxLaserManager.h"

using namespace ofxLaser;

Manager :: Manager() {
    currentProjector = -1;
    guiIsVisible = true;
    
}

bool Manager :: deleteProjector(Projector* projector) {
    bool success = ManagerBase::deleteProjector(projector);
    if(success) {
        currentProjector = -1;
        return true;
    } else {
        return false;
    }
    
}

void Manager::nextProjector() {
    int next = currentProjector+1;
    if(next>=(int)projectors.size()) next=-1;
    
    setCurrentProjector(next);
    
}

void Manager::previousProjector() {
    int prev = currentProjector-1;
    if(prev<-1) prev=(int)projectors.size()-1;
    setCurrentProjector(prev);
    
}
int Manager::getCurrentProjector(){
    return currentProjector;
}
void Manager::setCurrentProjector(int i){
    if(currentProjector!=i) {
        currentProjector = i;
    }
}
bool Manager::isProjectorSelected() {
    return currentProjector>=0;
}


void Manager:: drawUI(bool expandPreview){
    
    drawPreviews(expandPreview);
    
    ofxLaser::UI::updateGui();
    ofxLaser::UI::startGui();
    
    drawLaserGui();
    ofxLaser::UI::render();
    
}

void Manager :: drawPreviews(bool expandPreview) {
    
    
    // if expandPreview is true, then we expand the preview area to the
    // maximum space that we have available.
    
    
    // figure out the top and bottom section height
    
    // If the height of the laser is > 2/3 of the screen height (including spacing)
    // then shrink it to be 2/3 of the height.
    
    // we're showing the previews then they go at the top
    

    int lowerSectionHeight = 310;
    
    int thirdOfHeight = (ofGetHeight()-(guiSpacing*3))/3;
  
    if(lowerSectionHeight>thirdOfHeight) lowerSectionHeight = thirdOfHeight;
    
    // showPreview determines whether we show the preview
    // laser graphics on screen or not.
    if(showInputPreview) {
        
        ofPushStyle();
        
        // work out the scale for the preview...
        // default scale is 1 with an 8 pixel margin
        previewScale = 1;
        previewOffset = glm::vec2(guiSpacing,guiSpacing);
        
        if(height>(thirdOfHeight*2)) {
            previewScale = (float)(thirdOfHeight*2) / (float)height;
        }
        // but if we're viewing a projector warp ui
        // then shrink the preview down and move it underneath
        if(currentProjector>=0) {
            int positionY = 800 + guiSpacing*2;
            if((thirdOfHeight*2)+(guiSpacing*2) < positionY ) {
                positionY = (thirdOfHeight*2)+(guiSpacing*2);
                
            }
            previewOffset = glm::vec2(guiSpacing,positionY);
            previewScale = (float)lowerSectionHeight/(float)height;
            
            
            // but if we're expanding the preview, then work out the scale
            // to fill the whole screen
        } else if(expandPreview) {
            previewOffset = glm::vec2(0,0);
            previewScale = (float)ofGetWidth()/(float)width;
            if(height*previewScale>ofGetHeight()) {
                previewScale = (float)ofGetHeight()/(float)height;
            }
            
        }
        
        renderPreview();
        
        // this renders the input zones in the graphics source space
        for(size_t i= 0; i<zones.size(); i++) {
            
            
            zones[i]->offset.set(previewOffset);
            zones[i]->scale = previewScale;
            
            zones[i]->draw();
        }
        
        ofPushMatrix();
        laserMask.setOffsetAndScale(previewOffset,previewScale);
        laserMask.draw(showBitmapMask);
        ofTranslate(previewOffset);
        ofScale(previewScale, previewScale);
        
        
        ofPopMatrix();
        ofPopStyle();
        
    }
    
    
    ofPushStyle();
    
    // if none of the projectors are selected then draw
    // the path previews below
    if(currentProjector==-1) {
        ofPushMatrix();
        float scale = 1 ;
        if((lowerSectionHeight+guiSpacing)*(int)projectors.size()>ofGetWidth()-(guiSpacing*2)) {
            scale = ((float)ofGetWidth()-(guiSpacing*2))/((float)(lowerSectionHeight+guiSpacing)*(float)projectors.size());
            //ofScale(scale, scale);
        }
        
        //ofTranslate(guiSpacing,(height*previewScale)+(guiSpacing*2));
        
        for(size_t i= 0; i<projectors.size(); i++) {
            if((!expandPreview)&&(showOutputPreviews)) {
                ofRectangle projectorPreviewRect(guiSpacing+((lowerSectionHeight*scale) +guiSpacing)*i,(height*previewScale)+(guiSpacing*2),lowerSectionHeight*scale, lowerSectionHeight*scale);
                
                ofFill();
                ofSetColor(0);
                ofDrawRectangle(projectorPreviewRect);
                
                projectors[i]->drawTransformAndPath(projectorPreviewRect);
               
               
            }
            // disables the warp interfaces
            projectors[i]->disableTransformGui();
        }
        
        ofPopMatrix();
        
        // if we're not filling the preview to fit the screen, draw the projector
        // gui elements
        
        
    } else  {
        // ELSE we have a currently selected projector, so draw the various UI elements
        // for that...
        
        for(size_t i= 0; i<projectors.size(); i++) {
            if((int)i==currentProjector) {
                
                ofFill();
                ofSetColor(0);
                float size = 800;
                if(size>thirdOfHeight*2) size = thirdOfHeight*2;
                if(expandPreview) size =  (float)ofGetHeight()-(guiSpacing*2);
                
                ofDrawRectangle(guiSpacing,guiSpacing,size,size);
                projectors[i]->enableTransformGui();
                projectors[i]->drawLaserPath(guiSpacing,guiSpacing,size,size);
                projectors[i]->drawTransformUI(guiSpacing,guiSpacing,size,size);
               
                
            } else {
                projectors[i]->disableTransformGui();
            }
            
        }
        
    }
    
    ofPopStyle();
    

    if((!expandPreview) && (guiIsVisible)) {
        
        // if this is the current projector or we have 2 or fewer projectors, then render the gui
        if(!showProjectorSettings) {
            
    
            
            int w = dacStatusBoxSmallWidth;
            int x = ofGetWidth() - 220; // gui.getPosition().x-w-guiSpacing;
            
            // draw all the status boxes but small
            
            for(size_t i= 0; i<projectors.size(); i++) {

                //projectors[i]->renderStatusBox(x, i*(dacStatusBoxHeight+guiSpacing)+10, w,dacStatusBoxHeight);
            }
            
            
        }
        
        
    }
    
    
}
void Manager :: renderPreview() {
    

    if((previewFbo.getWidth()!=width*previewScale) || (previewFbo.getHeight()!=height*previewScale)) {
       // previewFbo.clear();
        previewFbo.allocate(width*previewScale, height*previewScale, GL_RGBA, 3);
    }
    
    previewFbo.begin();
    
    ofClear(0,0,0,0);
    ofPushStyle();
    // ofEnableSmoothing();
    ofPushMatrix();
    //ofTranslate(previewOffset);
    ofScale(previewScale, previewScale);
    
    
    // draw outline of laser output area
    ofSetColor(0);
    ofFill();
    ofDrawRectangle(0,0,width,height);
    ofSetColor(50);
    ofNoFill();
    ofDrawRectangle(0,0,width,height);
    
    // Draw laser graphics preview ----------------
    ofMesh mesh;
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    mesh.clear();
    mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    // Draw the preview laser graphics, with zones overlaid
    for(size_t i= 0; i<shapes.size(); i++) {
        shapes[i]->addPreviewToMesh(mesh);
    }
    
    ofRectangle laserRect(0,0,width, height);
    if(useBitmapMask) {
        const vector<glm::vec3>& points = mesh.getVertices();
        std::vector<ofFloatColor>& colours = mesh.getColors();
        
        for(size_t i= 0;i<points.size(); i++ ){
            
            ofFloatColor& c = colours.at(i);
            const glm::vec3& p = points[i];
            
            
            float brightness;
            
            if(laserRect.inside(p)) {
                brightness = laserMask.getBrightness(p.x, p.y);
            } else {
                brightness = 0;
            }
            
            c.r*=brightness;
            c.g*=brightness;
            c.b*=brightness;
            
        }
    }
    
    ofSetLineWidth(1.5);
    mesh.draw();
    
    std::vector<ofFloatColor>& colours = mesh.getColors();
    
    for(size_t i= 0; i<colours.size(); i++) {
        colours[i].r*=0.4;
        colours[i].g*=0.4;
        colours[i].b*=0.4;
    }
    
    ofSetLineWidth(4);
    mesh.draw();
    
    ofDisableBlendMode();
    // ofDisableSmoothing();
    
    ofPopMatrix();
    
    ofPopStyle();
    previewFbo.end();
    previewFbo.draw(previewOffset);
}

bool Manager ::toggleGui(){
    guiIsVisible = !guiIsVisible;
    return guiIsVisible;
}
void Manager ::setGuiVisible(bool visible){
    guiIsVisible = visible;
}
bool Manager::isGuiVisible() {
    return guiIsVisible;
}


void Manager::setDefaultHandleSize(float size) {
    defaultHandleSize = size;
    for(Projector* projector : projectors) {
        projector->setDefaultHandleSize(defaultHandleSize);
    }
    
}

void Manager::addCustomParameter(ofAbstractParameter& param){
    customParams.add(param);
}


bool Manager::togglePreview(){
    showInputPreview = !showInputPreview;
    return showInputPreview;
};


void Manager::drawLaserGui() {
    
    ofxLaser::ManagerBase& laser = *this;
    // auto mainSettings = ofxImGui::Settings();
    
    int mainpanelwidth = 270;
    int projectorpanelwidth = 280;
    int spacing = 8;
    
//    ImGuiWindowFlags window_flags = 0;
//
//    window_flags |= ImGuiWindowFlags_NoMove;
//    window_flags |= ImGuiWindowFlags_NoResize;
//    window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
//    window_flags |= ImGuiWindowFlags_NoNav;
   
    
    
    // calculate x position of main window
    int x = ofGetWidth() - mainpanelwidth - spacing;
    // if we're also showing the projector settings, make space
    // TODO max 2 projectors
//    if(laser.showProjectorSettings){
//        //int numProjectors =laser.getNumProjectors();
//        //if(numProjectors>2) numProjectors = 1;
//        int numProjectors = 1;
//        x-=(numProjectors*projectorpanelwidth);
//        x-=(spacing*numProjectors);
//    }
    
    UI::startWindow("Settings", ImVec2(x, spacing), ImVec2(mainpanelwidth, 0), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize, true );
    
    
    // some custom styles - to do put in a theme?
//    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 5.0f)); // 1 Spacing between items
//    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(6.0f, 6.0f)); // 2 gap between element and label
    
    
    // START BIG BUTTONS
    UI::largeItemStart();

    // the arm and disarm buttons
    bool useRedButton =laser.areAllLasersArmed();
    if(useRedButton) UI::secondaryColourButtonStart();
    // change the colour for the arm all button if we're armed
    int buttonwidth = (mainpanelwidth-(spacing*3))/2;
    if(ImGui::Button("ARM ALL LASERS", ImVec2(buttonwidth, 0.0f) )) {
        laser.armAllProjectors();
    }
    if(useRedButton) UI::secondaryColourButtonEnd();
    
    ImGui::SameLine();
    if(ImGui::Button("DISARM ALL LASERS",  ImVec2(buttonwidth, 0.0f))) {
        laser.disarmAllProjectors();
    }
    
      // change width of slider vs label
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 120.0f);
    
    // add intensity slide
    //UI::addFloatAsIntSlider(laser.masterIntensity, 100);
    ImGui::PushItemWidth(mainpanelwidth-(spacing*2));
    float multiplier = 100;
    int value = laser.globalBrightness*multiplier;
    if (ImGui::SliderInt("##int", &value, laser.globalBrightness.getMin()*multiplier, laser.globalBrightness.getMax()*multiplier, "GLOBAL BRIGHTNESS %d")) {
        laser.globalBrightness.set((float)value/multiplier);
        
    }
    ImGui::PopItemWidth();
    
    string label;
    if(projectors.size()>0) {
        label = showProjectorSettings? "CLOSE PROJECTOR SETTINGS" : "OPEN PROJECTOR SETTINGS";
        
        if(ImGui::Button(label.c_str())) {
            showProjectorSettings = !showProjectorSettings;
        }
    }
    
//    for(int i =0; i<(int)projectors.size(); i++) {
//
//        if(i>0) ImGui::SameLine();
//        string label = ofToString(i+1);
//
//        if(ImGui::Button(label.c_str())) {
//            if(currentProjector!=i) currentProjector = i;
//            else currentProjector = -1;
//        }
//
//
//    }
//
    
    if(ImGui::Button("ADD PROJECTOR", ImVec2(buttonwidth, 0.0f))) {
        createAndAddProjector();
    }
    ImGui::SameLine();
    if(ImGui::Button("ADD ZONE", ImVec2(buttonwidth, 0.0f))) {
        addZone();
        lockInputZones = false;
    }
    // END BIG BUTTONS
    UI::largeItemEnd();
  
    
    
    // SHOW LIST OF PROJECTORS
    
    for(int i = 0; i<laser.getNumProjectors(); i++) {
        
        ofxLaser::Projector& projector = laser.getProjector(i);
        string projectorNumberString = ofToString(i+1);
        bool showsecondarycolour = false;
        
        // PROJECTOR BUTTONS
        if(ImGui::Button(projectorNumberString.c_str())) {
            if(currentProjector!=i) currentProjector = i;
            else currentProjector = -1;
        }
        ImGui::SameLine();
        
        // ARM BUTTONS
        if(projector.armed) {
            UI::secondaryColourButtonStart();
            showsecondarycolour = true;
        }
        string armlabel = "ARM##"+ofToString(i+1);
        if(ImGui::Button(armlabel.c_str())){
            projector.toggleArmed();
        }
        if(showsecondarycolour) UI::secondaryColourButtonEnd();
        
        // FRAME RATES
        
        ImGui::SameLine();
        label = "##framerate"+projectorNumberString;
        ImGui::PushItemWidth(100);
        ImGui::PlotLines(label.c_str(), projector.frameTimeHistory, projector.frameTimeHistorySize, projector.frameTimeHistoryOffset, "", 0, 0.1f);
        ImGui::PopItemWidth();
        
        // DAC STATUSES
        ImGui::SameLine();
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 size = ImVec2(19,19); // ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
      
        int radius = 4;
        ImVec2 p = ImGui::GetCursorScreenPos();
        p.x+=radius-2;
        p.y+=radius+4;
        ImU32 col = UI::getColourForState(projector.getDacConnectedState());
        
        draw_list->AddCircleFilled(p,radius, col);
        ImGui::InvisibleButton("##invisible", ImVec2(radius*2, radius*2) - ImVec2(2,2));
 
        
        
        
        
    }

    UI::addIntSlider(laser.testPattern);
    
    UI::addParameterGroup(laser.interfaceParams);
    
    
    if((!lockInputZones) && (currentProjector ==-1)) {
        
        ImGui::Separator();
        
        for(Zone* zone : zones) {

            string buttonlabel ="DELETE "+zone->zoneLabel;
            string modallabel ="Delete "+zone->zoneLabel+"?";
            
            UI::secondaryColourButtonStart();
            if(ImGui::Button(buttonlabel.c_str())) {
                ImGui::OpenPopup(modallabel.c_str());
            }
            UI::secondaryColourButtonEnd();
            
            if (ImGui::BeginPopupModal(modallabel.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Are you sure? All the zone settings will be deleted.\n\n");
                ImGui::Separator();

                UI::secondaryColourButtonStart();
                if (ImGui::Button("DELETE", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    deleteZone(zone);
                    
                }
                UI::secondaryColourButtonEnd();
                
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    
                }
                ImGui::EndPopup();
            }

             
            
            
        
        }
    }
    
    if(laser.customParams.size()>0) {
        ImGui::Separator();
        ImGui::Text("CUSTOM PARAMETERS");
        UI::addParameterGroup(laser.customParams);

    }
    
    
   // ImGui::PopStyleVar(2);
    
    ImGui::PopItemWidth();
    
    UI::endWindow();
   // ImGui::End();
    
    
    // show projector settings :
    
    if(laser.showProjectorSettings) {
        x-=(projectorpanelwidth+spacing);

        int projectorIndexToShow = currentProjector;
        if(projectorIndexToShow ==-1) projectorIndexToShow = 0;
        drawProjectorPanel(&getProjector(projectorIndexToShow), projectorpanelwidth, spacing, x);

        
    }
    
    // Show projector zone settings mute / solo / etc
    if(currentProjector!=-1)  {
        
        
        // PROJECTOR ZONE SETTINGS
        Projector* projector = projectors[currentProjector];
        
        glm::vec2 projectorZonePos = previewOffset + (previewScale*glm::vec2(width, 0));
        
        UI::startWindow("Projector zones", ImVec2(projectorZonePos.x+spacing, projectorZonePos.y), ImVec2(340,0));
    
        ImGui::Columns(3, "Projector zones columns");
        ImGui::SetColumnWidth(0, 80.0f);
        ImGui::SetColumnWidth(1, 80.0f);
        ImGui::SetColumnWidth(2, 180.0f);
        // MUTE SOLO
        for(ProjectorZone* projectorZone : projector->projectorZones) {
            
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, projectorZone->getEnabled()?0.5f:1.0f);
            
            string muteLabel = "M##"+projectorZone->getLabel();
            if(ImGui::Button(muteLabel.c_str(), ImVec2(20,20))) {
                projectorZone->muted = !projectorZone->muted;
            };
            ImGui::PopStyleVar();
            
            ImGui::SameLine();
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, projectorZone->soloed?1.0f:0.5f);
            string soloLabel = "S##"+projectorZone->getLabel();
            if(ImGui::Button(soloLabel.c_str(), ImVec2(20,20))){
                projectorZone->soloed = !projectorZone->soloed;
            }
            ImGui::PopStyleVar();
            ImGui::SameLine();
            ImGui::Text("%s",projectorZone->getLabel().c_str());
            
        }
        ImGui::NextColumn();
       // ImGui::SetCursorPosX(200);
        for(Zone* zone : zones) {
            bool checked = projector->hasZone(zone);
            
            if(ImGui::Checkbox(zone->displayLabel.c_str(), &checked)) {
                if(checked) {
                    projector->addZone(zone, width, height);
                } else {
                    
                    projector->removeZone(zone);
                }
                
            }
        }
        ImGui::NextColumn();
        
        
        MaskManager& maskManager = projector->maskManager;
        if(ImGui::Button("ADD MASK")) {
            maskManager.addQuadMask();
        }
        ImDrawList*   draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        draw_list->AddLine(ImVec2(p.x - 9999, p.y), ImVec2(p.x + 9999, p.y),  ImGui::GetColorU32(ImGuiCol_Border));
        ImGui::Dummy(ImVec2(0.0f, 2.0f));
        for(QuadMask* mask : maskManager.quads){
            string label = "##"+mask->displayLabel;
            ImGui::Text("MASK %s", mask->displayLabel.c_str());
            ImGui::SameLine();
            ImGui::PushItemWidth(40);
            int level = mask->maskLevel;
            if (ImGui::DragInt(label.c_str(),&level,1,0,100,"%d%%")) {
                mask->maskLevel = level;
            }
            
            ImGui::PopItemWidth();
            ImGui::SameLine();
            string buttonlabel = "DELETE "+mask->displayLabel+"##mask";
            if(ImGui::Button(buttonlabel.c_str())) {
                maskManager.deleteQuadMask(mask);
                
            }
            
        }
        
        
        
        
        
        
        
        ImGui::Columns();
        ImGui::Separator();
        UI::addIntSlider(projector->testPattern);
        UI::addCheckbox(projector->hideContentDuringTestPattern);
        UI::toolTip("Disable this if you want to see the laser content at the same time as the text patterns");
        
        UI::endWindow();
        
        
        // Projector Masks
        
        
        for(ProjectorZone* projectorZone : projector->projectorZones) {
        
            if(projectorZone->zoneTransform.getSelected()) {
                ImVec2 pos(projectorZone->zoneTransform.getRight(),projectorZone->zoneTransform.getCentre().y);
                ImVec2 size(200,0);
                UI::startWindow(projectorZone->getLabel()+"##"+projector->getLabel(),pos, size, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);


                UI::addParameterGroup(projectorZone->zoneTransform.params);
                ImGui::Text("Edge masks");
                UI::addFloatSlider(projectorZone->bottomEdge);
                UI::addFloatSlider(projectorZone->topEdge);
                UI::addFloatSlider(projectorZone->leftEdge);
                UI::addFloatSlider(projectorZone->rightEdge);
                
                UI::endWindow();
            }
        }
        
     
    }
    
}


void Manager :: drawProjectorPanel(ofxLaser::Projector* projector, float projectorpanelwidth, float spacing, float x) {
    
//    ImGuiWindowFlags window_flags = 0;
//
//    window_flags |= ImGuiWindowFlags_NoMove;
//    window_flags |= ImGuiWindowFlags_NoResize;
//    window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
//    window_flags |= ImGuiWindowFlags_NoNav;
//
//    ImGui::SetNextWindowSize(ImVec2(projectorpanelwidth,0), ImGuiCond_Appearing);
//    ImGui::SetNextWindowPos(ImVec2(x,spacing));
//
//
//    ImGui::Begin(projector->getLabel().c_str(), NULL, window_flags);
    
    UI::startWindow(projector->getLabel(), ImVec2(x, spacing), ImVec2(projectorpanelwidth,0), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |ImGuiWindowFlags_AlwaysAutoResize, true, (bool*)&showProjectorSettings.get());
    
   
    
    UI::largeItemStart();
    // change width of slider vs label
    ImGui::PushItemWidth(140);
    
    UI::addCheckbox(projector->armed);
    //UI::addFloatAsIntSlider(projector->intensity, 100);
    ImGui::PushItemWidth(projectorpanelwidth-(spacing*2));
    float multiplier = 100;
    int value = projector->intensity*multiplier;
    if (ImGui::SliderInt("##int", &value, projector->intensity.getMin()*multiplier, projector->intensity.getMax()*multiplier, "BRIGHTNESS %d")) {
        projector->intensity.set((float)value/multiplier);
        
    }
    ImGui::PopItemWidth();
    
    
    UI::largeItemEnd();
    
    UI::addIntSlider(projector->testPattern);
    //UI::addButton(resetDac);
    
    
    // THE DAC STATUS COLOUR - TO BE IMPROVED
    ImGui::Separator();
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 size = ImVec2(19,19); // ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
      
        ImVec2 p = ImGui::GetCursorScreenPos();
        int state = projector->getDacConnectedState();
        
        ImU32 col = UI::getColourForState(projector->getDacConnectedState());
        
        draw_list->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), col);
        ImGui::InvisibleButton("##gradient2", size - ImVec2(2,2));
       
    }
    
    ImGui::SameLine();
    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0,2));
    //ImGui::Text("DAC:");
    //ImGui::SameLine();
    // TODO add a method in projector that tells us if
    // it's using the empty dac?
    if(!projector->hasDac() && (projector->dacId.get()!="") ) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
        ImGui::Text("Waiting for %s", projector->dacId.get().c_str());
        ImGui::PopStyleVar();
    } else {
        ImGui::Text("%s", projector->getDacLabel().c_str());
    }

    ImGui::SameLine(projectorpanelwidth-30);
    if(ImGui::Button("^", ImVec2(19,19))){
        
        
    }
    
    // DAC LIST -------------------------------------------------------------
    
    ImGui::PushItemWidth(projectorpanelwidth-spacing*2);
    
    // get the dacs from the dacAssigner
    const vector<DacData>& dacList = dacAssigner.getDacList();
//    string comboLabel;
//    if(dacList.size()>0) comboLabel = projector->getDacLabel();
//    else comboLabel = "No DACs discovered";
        
    if (ImGui::ListBoxHeader("##listbox", MIN(5, MAX(1,dacList.size())))){
        
        if(dacList.empty()) {
         
            ImGui::Selectable("No laser controllers found", false, ImGuiSelectableFlags_Disabled );
   
        } else {
                
                
            // add a combo box item for every element in the list
            for(const DacData& dacdata : dacList) {
                
                // get the dac label (usually type + unique ID)
                string itemlabel = dacdata.label;
                
                ImGuiSelectableFlags selectableflags = 0;
                
                if(!dacdata.available) {
                   // ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
                   //itemlabel += " - no longer available";
                    selectableflags|=ImGuiSelectableFlags_Disabled;
                } else {
                   //
                }
                // if this dac is assigned to a projector, show which projector
                //  - this could be done at the other end?
                
                
                
                if (ImGui::Selectable(itemlabel.c_str(), (dacdata.assignedProjector == projector), selectableflags)) {
                    // then select dac
                    // TODO : show a warning yes / no if :
                    //      - we already are connected to a DAC
                    //      - the chosen DAC is already being used by another projector
                    dacAssigner.assignToProjector(dacdata.label, *projector);
                }
                
                if(dacdata.assignedProjector!=nullptr) {
                    ImGui::SameLine(projectorpanelwidth - 100);
                    string label =" > " + dacdata.assignedProjector->getLabel();
                    ImGui::Text("%s",label.c_str());
                }
                
                //ImGui::PopStyleVar();
            }
        }
        //    if (is_selected)
        //       ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        //ImGui::EndCombo();
        ImGui::ListBoxFooter();
    }
    //ImGui::PopStyleVar();
    ImGui::PopItemWidth();
    
    if(projector->hasDac()) {
        if(ImGui::Button("Disconnect DAC")) {
            dacAssigner.disconnectDacFromProjector(*projector);
        }
        ImGui::SameLine();
    }
    
    if(ImGui::Button("Refresh DAC list")) {
        dacAssigner.updateDacList();
        
    }
    // ----------------------------------------------
    
    

    ImGui::Separator();
    ImGui::Text("OUTPUT / ZONE SETTINGS");
    
    UI::largeItemStart();
    ImGui::PushItemWidth(190);
    UI::addFloatAsIntPercentage(projector->speedMultiplier);
    UI::toolTip("Scanner speed adjustment (NB this works mathematically, it doesn't change the point rate)");
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(170);
    UI::addFloatSlider(projector->colourChangeShift);
    UI::toolTip("Shifts the laser colours to match the scanner position (AKA blank shift)");
    ImGui::PopItemWidth();
    UI::largeItemEnd();
    
   
 
    // ZONES
    UI::addCheckbox(projector->flipX);
    UI::addCheckbox(projector->flipY);
    
    // FINE OUTPUT SETTINGS
    bool treevisible = ImGui::TreeNode("Fine position adjustments");
        
    UI::toolTip("These affect all output zones for this projector and can be used to re-align projectors if they have moved slightly since setting them up");
    if (treevisible){
        
       /// ImGui::PushItemWidth(projectorpanelwidth-120);
        
        ofParameter<float>& param = projector->rotation;
        if(ImGui::DragFloat("Rotation", (float*)&param.get(), 0.01f,-10,10)) { //  param.getMin(), param.getMax())) {
            param.set(param.get());
            
        }
        //UI::addFloatSlider(projector->rotation, "%.2f", 2.0);
        if(projector->rotation!=0) {
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::Button("Reset")) projector->rotation = 0;
        }
        
        ofParameter<glm::vec2>& param2 = projector->outputOffset;
        if(ImGui::DragFloat2("Position", (float*)&param2.get().x, 0.01f, -50.0f,50.0f)) { //  param.getMin(), param.getMax())) {
            param2.set(param2.get());
            
        }
        //UI::addFloat2Slider(projector->outputOffset, "%.2f", 2.0);
        
        
        
        glm::vec2 zero2;
        if(projector->outputOffset.get()!=zero2) {
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::Button("Reset")) projector->outputOffset.set(zero2);
        }
       // ImGui::PopItemWidth();
        ImGui::TreePop();
    }

   
    
    
    // TODO IMPLEMENT PROJECTOR PROFILES
    /*
    // PROJECTOR PROFILE
    ImGui::Separator();
    ImGui::Text("PROJECTOR PROFILE");
    
    // TODO :
    // check if the settings are different from the preset, if they are
    // show a "save" button, also save as?
    //
    // when an option is selected, update all the params
    
    if (ImGui::BeginCombo("##combo", "LightSpace Unicorn RGB11000")) // The second parameter is the label previewed before opening the combo.
    {
        
        if (ImGui::Selectable("LightSpace Unicorn RGB11000", true)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("OPT PD4", false)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("OPT PD25", false)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("LaserCube 2W", false)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("LaserCube 1W", false)) {
            //item_current = items[n];
        }
        
        //    if (is_selected)
        //       ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        ImGui::EndCombo();
    }
    */
    
    // SCANNER PROFILE SETTINGS
    
    
    ImGui::Separator();
    ImGui::Text("SCANNER SETTINGS");
    
    
    PresetManager& presetManager = *PresetManager::instance();
    // TODO :
    // check if the settings are different from the preset, if they are
    // show a "save" button, also save as?
    //
    // when an option is selected, update all the params
    const vector<string>& presets = presetManager.getPresetNames();
    string label =projector->scannerSettings.getLabel();
    ScannerSettings& currentPreset = *presetManager.getPreset(label);
   
    
    bool presetEdited = (projector->scannerSettings!=currentPreset);
    if (presetEdited){
        label+="(edited)";
        
        
    }
   
    
    if (ImGui::BeginCombo("##Scanner presets", label.c_str())) { // The second parameter is the label previewed before opening the combo.

        for(const string presetName : presets) {

            if (ImGui::Selectable(presetName.c_str(), presetName == projector->scannerSettings.getLabel())) {
                //get the preset and make a copy of it
                // uses operator overloading to create a clone
                projector->scannerSettings = *presetManager.getPreset(presetName);
            }
        }
     
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    
    if(ImGui::Button("EDIT")) {
        //ImGui::OpenPopup("Edit Scanner Preset");
        showEditScannerPreset = true;
    }
    
//    ImGui::SetNextWindowPos,
    ImGui::SetNextWindowSize({760,0});
    // centre popup
 
    ImGui::SetNextWindowPos({(float)ofGetWidth()/2, (float)ofGetHeight()/2}, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // SCANNER PRESET POPUP
    
    if (showEditScannerPreset && ImGui::Begin("Edit Scanner Preset", &showEditScannerPreset, ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoDocking))
    {
        ImGui::Text("SCANNER SETTINGS - %s",projector->getLabel().c_str());
        ImGui::Separator();
        if (ImGui::BeginCombo("Scanner presets", label.c_str())) { // The second parameter is the label previewed before opening the combo.

            for(const string presetName : presets) {

                if (ImGui::Selectable(presetName.c_str(), presetName == projector->scannerSettings.getLabel())) {
                    //get the preset and make a copy of it
                    // uses operator overloading to create a clone
                    projector->scannerSettings = *presetManager.getPreset(presetName);
                }
            }
         
            ImGui::EndCombo();
        }
        
        ImGui::SameLine();
      
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (presetEdited ? 1.0f : 0.5f));
        if(ImGui::Button("SAVE")) {
            if(presetEdited)ImGui::OpenPopup("Save Preset");
            
        }
        ImGui::PopStyleVar();
        
        if (ImGui::BeginPopupModal("Save Preset", 0)){
            string presetlabel = projector->scannerSettings.getLabel();

            ImGui::Text("Are you sure you want to overwrite the preset \"%s\"?", presetlabel.c_str());
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                PresetManager::addPreset(presetlabel, projector->scannerSettings);
                ImGui::CloseCurrentPopup();
                
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                
            }
            ImGui::EndPopup();
            
            
        }
        static char newPresetLabel[255]; // = presetlabel.c_str();
        
    
        ImGui::SameLine();
        if(ImGui::Button("SAVE AS")){
            strcpy(newPresetLabel, projector->scannerSettings.getLabel().c_str());
            ImGui::OpenPopup("Save Preset As");
            
        };
        
        if (ImGui::BeginPopupModal("Save Preset As", 0)){
            //string presetlabel = projector->scannerSettings.getLabel();
            
            if(ImGui::InputText("1", newPresetLabel, IM_ARRAYSIZE(newPresetLabel))){
                //presetlabel = buf;
                //PresetManager::addPreset(presetlabel, projector->scannerSettings);
                
            }
            
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                string presetlabel = newPresetLabel;
                // TODO CHECK PRESET EXISTS AND ADD POP UP
                PresetManager::addPreset(presetlabel, projector->scannerSettings);
                ImGui::CloseCurrentPopup();
                
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                
            }
            ImGui::EndPopup();
            
            
        }
        
        
        UI::addResettableFloatSlider(projector->scannerSettings.moveSpeed, currentPreset.moveSpeed,"How quickly the mirrors move between shapes", "%.1f", 3.0f);

        ImGui::Columns(2);
        UI::addResettableIntSlider(projector->scannerSettings.shapePreBlank, currentPreset.shapePreBlank, "The length of time that the laser is switched off and held at the beginning of a shape");
     
        UI::addResettableIntSlider(projector->scannerSettings.shapePreOn, currentPreset.shapePreOn, "The length of time that the laser is switched on and held at the beginning of a shape");
        ImGui::NextColumn();
       
        UI::addResettableIntSlider(projector->scannerSettings.shapePostBlank, currentPreset.shapePostBlank,"The length of time that the laser is switched off and held at the end of a shape" );
        UI::addResettableIntSlider(projector->scannerSettings.shapePostOn, currentPreset.shapePostOn,"The length of time that the laser is switched on and held at the end of a shape" );

        ImGui::Columns(1);
        
        ImGui::Text("Render profiles");
        UI::toolTip("Every scanner setting has three profiles for rendering different qualities of laser effects. Unless otherwise specified, the default profile is used. The fast setting is good for long curvy lines, the high detail setting is good for complex pointy shapes.");

        ImGui::Separator();
        //bool firsttreeopen = true;
        ImGui::Columns(3);
        ImGui::SetColumnWidth(0, 250);
        ImGui::SetColumnWidth(1, 250);
        ImGui::SetColumnWidth(1, 250);
       

        for (auto & renderProfilePair : projector->scannerSettings.renderProfiles) {
            ImGui::PushItemWidth(120);
            string name =renderProfilePair.first;
            RenderProfile& profile = renderProfilePair.second;
           
            RenderProfile& resetProfile = currentPreset.renderProfiles.at(name);
            
            ImGui::Text("%s", name.c_str());
            UI::addResettableFloatSlider(profile.speed,resetProfile.speed, "",  "%.1f", 3.0f);
            UI::addResettableFloatSlider(profile.acceleration,resetProfile.acceleration, "",  "%.2f", 3.0f);
            UI::addResettableIntSlider(profile.dotMaxPoints, resetProfile.dotMaxPoints);
            UI::addResettableFloatSlider(profile.cornerThreshold, resetProfile.cornerThreshold);
            
            ImGui::PopItemWidth();
            ImGui::NextColumn();
        }
        
       
        ImGui::End();
    }

    
    
    // COLOUR SETTINGS
    ImGui::Separator();
    ImGui::Text("COLOUR");
   
    if(ImGui::TreeNode("Colour calibration")){
        
        UI::addParameterGroup(projector->colourSettings.params);
        
        ImGui::TreePop();
    }
    
   
    
    ImGui::Separator();
    ImGui::Text("ADVANCED SETTINGS");

    // ADVANCED
  
    
    if(ImGui::TreeNode("Advanced")){
        // POINT RATE
        ImGui::PushItemWidth(projectorpanelwidth-60);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 26.0f);
        int ppsslider = projector->pps;
        string ppsstring = "Point rate : " + ofToString(ppsslider);
        if(ImGui::SliderInt("##Point rate", &ppsslider, projector->pps.getMin(), projector->pps.getMax(), ppsstring.c_str())){
            projector->pps.set(ppsslider/100*100);
          
        }
        UI::toolTip("The actual points sent to the projector - YOU DON'T NEED TO ADJUST THIS unless you want to actually change the data rate, or you need better resolution for very fast scanners. The speed of the scanners can be fully adjusted without changing the point rate. ");
        ImGui::PopItemWidth();
        ImGui::PopStyleVar(1);
        
        UI::addParameterGroup(projector->advancedParams);
        ImGui::TreePop();
    }
    
    
    // the arm and disarm buttons
    //int buttonwidth = (mainpanelwidth-(spacing*3))/2;
    UI::secondaryColourButtonStart();
    if(ImGui::Button("DELETE PROJECTOR")) {
        ImGui::OpenPopup("Delete?");
    }
    UI::secondaryColourButtonEnd();
    
    
    if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure? All the zone settings will be deleted.\n\n");
        ImGui::Separator();

        //static int dummy_i = 0;
        //ImGui::Combo("Combo", &dummy_i, "Delete\0Delete harder\0");

//        static bool dont_ask_me_next_time = false;
//        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
//        ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
//        ImGui::PopStyleVar();

        
        UI::secondaryColourButtonStart();
        
        if (ImGui::Button("DELETE", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            deleteProjector(projector);
            
        }
        UI::secondaryColourButtonEnd();
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::EndPopup();
    }

     
    
  
    
    
    ImGui::PopItemWidth();
    
    
    // draw a flashing dot during saving
    if(projector->getSaveStatus() && (ofGetElapsedTimeMillis()%300)<150) {
        ImDrawList*   draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetWindowPos();
        p.x+=ImGui::GetContentRegionAvailWidth();
        p.y+=30;// + ImGui::GetScrollY();
       // if(ImGui::GetScrollY()>0) p.x-=14;
        //ImGui::GetContentRegionAvailWidth()
        draw_list->AddCircleFilled(p, 4, ImGui::GetColorU32(ImGuiCol_Border));
    }
    
    
    //ImGui::End();
    UI::endWindow();
}


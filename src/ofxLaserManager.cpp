//
//  ofxLaserManager.cpp
//  ofxLaser 2
//
//  Created by Seb Lee-Delisle on 06/05/2021.
//

#include "ofxLaserManager.h"

using namespace ofxLaser;

Manager * Manager :: laserManager = NULL;

Manager * Manager::instance() {
    if(laserManager == NULL) {
        laserManager = new Manager();
    }
    return laserManager;
}


Manager :: Manager() {
    
    if(laserManager == NULL) {
        laserManager = this;
    } else {
        ofLog(OF_LOG_ERROR, "Multiple ofxLaser::Manager instances created");
    }
    
    showInputPreview = true;
    lockInputZones = false;
    
    initAndLoadSettings();
    
    // if no lasers are loaded make one and add a zone
    if(lasers.size()==0) {
        createAndAddLaser();

        if(canvasTarget.zones.size()==0) createDefaultZone();
        lasers[0]->addZone(0);
       
    }

    selectedLaserIndex = 0;
    viewMode  = OFXLASER_VIEW_CANVAS;

    guiIsVisible = true;
    guiLaserSettingsPanelWidth = 320;
    guiSpacing = 8;
     
    setDefaultPreviewOffsetAndScale();
    
    params.add(showGuideImage.set("Show guide image", false));
    params.add(guideImageColour.set("Guide image colour", ofColor::white));
    params.add(guideImageFilename.set("Guide image filename", ""));
    dacSettingsTimeSlice.set("Magnification", 0.5, 0.1, 20);
    
    // loadedJson is filled on loadSettings. So now check the extras.
    if(!loadedJson.empty()) {
        if(loadedJson.contains("Laser")) {
          
            try {
                ofDeserialize(loadedJson["Laser"], showGuideImage);
                ofDeserialize(loadedJson["Laser"], guideImageColour);
                ofDeserialize(loadedJson["Laser"], guideImageFilename);
                if(guideImageFilename.get()!="") setGuideImage("guideImages/" + guideImageFilename.get());
               
            } catch(...) {
                //cout << showGuideImage << " " <<loadJson["Laser"]["Show_guide_image"]<< endl;
            }
        }
    }
    
    params.add(customParams);
    
    ofAddListener(ofEvents().mouseMoved, this, &Manager::mouseMoved, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mousePressed, this, &Manager::mousePressed, OF_EVENT_ORDER_BEFORE_APP);

    ofAddListener(ofEvents().mouseReleased, this, &Manager::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseDragged, this, &Manager::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseScrolled, this, &Manager::mouseScrolled, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().keyPressed, this, &Manager::keyPressed, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().keyReleased, this, &Manager::keyReleased, OF_EVENT_ORDER_BEFORE_APP);


}

Manager::~Manager() {
    ofRemoveListener(ofEvents().mouseMoved, this, &Manager::mouseMoved, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseScrolled, this, &Manager::mouseScrolled, OF_EVENT_ORDER_BEFORE_APP);

    ofRemoveListener(ofEvents().mousePressed, this, &Manager::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseReleased, this, &Manager::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseDragged, this, &Manager::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(params.parameterChangedE(), this, &Manager::paramChanged);
    ofRemoveListener(ofEvents().keyPressed, this, &Manager::keyPressed, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().keyReleased, this, &Manager::keyReleased, OF_EVENT_ORDER_BEFORE_APP);
}

void Manager :: createAndAddLaser()  {
    int laserindex = lasers.size();
    ManagerBase:: createAndAddLaser();
    setLaserDefaultPreviewOffsetAndScale(laserindex);
    laserZoneViews.emplace_back(lasers.back());
    laserZoneViews.back().setOutputRect(getZonePreviewRect());
    laserZoneViews.back().autoFitToOutput();
    laserZoneViews.back().setGrid(zoneGridSnap, zoneGridSize);
    
}



void Manager :: setLaserDefaultPreviewOffsetAndScale(int lasernum) {
    LaserZoneViewController* laserview = getLaserViewControllerByIndex(lasernum);
    if(laserview!=nullptr) {
        laserview->autoFitToOutput();
    }
}

void Manager :: initAndLoadSettings() {
    
    if(initialised) {
        ofLogError("ofxLaser::Manager::initAndLoadSettings() called twice - NB you no longer need to call this in your code, it happens automatically");
        return ;
    }
    
    ofxLaser::UI::setupGui();
   
    interfaceParams.setName("Interface");
    interfaceParams.add(lockInputZones.set("Lock input zones", true));
    interfaceParams.add(showInputZones.set("Show input zones", true));
    interfaceParams.add(showInputPreview.set("Show preview", true));
    interfaceParams.add(showOutputPreviews.set("Show path previews", true));
    //interfaceParams.add(useBitmapMask.set("Use bitmap mask", false));
    //interfaceParams.add(showBitmapMask.set("Show bitmap mask", false));
    //interfaceParams.add(laserMasks.set("Laser mask shapes", false));
    params.add(interfaceParams);
    
    customParams.setName("CUSTOM PARAMETERS");
    
    // is this still used ?
    params.add(zoneEditorShowLaserPath.set("Show path in zone editor", true));
    params.add(zoneEditorShowLaserPoints.set("Show points in zone editor", false));
    params.add(zoneGridSnap.set("Zone snap to grid", true));
    params.add(zoneGridSize.set("Zone grid size", 20,1,50));
    params.add(globalLatency.set("Latency (ms)", 150,0,400));

   // params.add(showDacAssignmentWindow.set("showDacAssignmentWindow", false));
    params.add(showCustomParametersWindow.set("showCustomParametersWindow", true));
    params.add(showLaserManagementWindow.set("showLaserManagementWindow", true));
    params.add(showLaserOutputSettingsWindow.set("showLaserOutputSettingsWindow", true));

    loadSettings();
    showDacAssignmentWindow = false; 
    // param changed updates zone settings and global latency on all
    paramChanged(params);
    ofAddListener(params.parameterChangedE(), this, &Manager::paramChanged);
 
    showInputPreview = true;
    
    copyParams.add(copyScannerSettings.set("Copy scanner / speed settings", false));
    copyParams.add(copyAdvancedSettings.set("Copy advanced settings", false));
    copyParams.add(copyColourSettings.set("Copy colour settings", false));
    copyParams.add(copyZonePositions.set("Copy output zone positions", false));

}
void Manager :: paramChanged(ofAbstractParameter& e) {
    for(Laser* laser : lasers) {
        //laser->setGrid(zoneGridSnap, zoneGridSize);
        laser->maxLatencyMS = globalLatency;
    }
    for(LaserZoneViewController& laserview : laserZoneViews) {
        laserview.setGrid(zoneGridSnap, zoneGridSize);
        
    }
   //ofLogNotice() << "paramChanged " << e.getName();
    saveSettings();
}


void Manager :: update() {
 
    ManagerBase :: update();

    // bit of a nasty way to update the canvas zones
    // will be better as a listener view / control system in future
    canvasViewController.setSourceRect(ofRectangle(0,0,canvasTarget.getWidth(), canvasTarget.getHeight()));
    canvasViewController.setOutputRect(ofRectangle(10,10,canvasTarget.getWidth(), canvasTarget.getHeight()));
    if(canvasViewController.update()) {
        canvasViewController.updateZonesFromUI(canvasTarget.zones);
        saveSettings();
    }
    canvasViewController.updateUIFromZones(canvasTarget.zones);
    canvasViewController.setLockedAll(lockInputZones);
   
    
    
    
}

void Manager :: mouseMoved(ofMouseEventArgs &e) {
    for(LaserZoneViewController& zoneView : laserZoneViews) {
        zoneView.mouseMoved(e);
    }
    canvasViewController.mouseMoved(e);
}


bool Manager :: mousePressed(ofMouseEventArgs &e){

   // ofLogNotice("Manager :: mousePressed"); 
    if (viewMode  == OFXLASER_VIEW_CANVAS) {
        canvasViewController.mousePressed(e);

    } else if (viewMode  == OFXLASER_VIEW_OUTPUT) {
       
        
        LaserZoneViewController* currentLaserView = getCurrentLaserViewController();
        if(currentLaserView!=nullptr) {
            currentLaserView->mousePressed(e);
        } else {
            ofLogError("ERROR - missing view for laser");
        }
        
    } else if(viewMode == OFXLASER_VIEW_3D) {
        visualiser3D.mousePressed(e);
        
    }
    
    return false;
}
LaserZoneViewController*  Manager ::getCurrentLaserViewController() {
    
    return getLaserViewControllerByIndex(selectedLaserIndex);
}
LaserZoneViewController*  Manager ::getLaserViewControllerByIndex(int index) {

    if((index>=0) && (index<laserZoneViews.size())) {
        return &laserZoneViews[index];
    } else {
        return nullptr;
    }

}

void Manager :: setDefaultPreviewOffsetAndScale(){

    canvasViewController.autoFitToOutput();
    
}

bool Manager :: mouseReleased(ofMouseEventArgs &e){

    for(LaserZoneViewController& zoneView : laserZoneViews) {
        zoneView.mouseReleased(e);
    }
    visualiser3D.mouseReleased(e);
    canvasViewController.mouseReleased(e);
    return false;
}

bool Manager :: mouseDragged(ofMouseEventArgs &e){
    visualiser3D.mouseDragged(e);
    
    for(LaserZoneViewController& zoneView : laserZoneViews) {
        zoneView.mouseDragged(e);
    }
    canvasViewController.mouseDragged(e);

    return true;
}

void Manager :: mouseScrolled(ofMouseEventArgs &e){
    //visualiser3D.mouseDragged(e);
    
    for(LaserZoneViewController& zoneView : laserZoneViews) {
        zoneView.mouseScrolled(e);
    }
    canvasViewController.mouseScrolled(e); 
   // return false;
    
}

bool Manager :: keyPressed(ofKeyEventArgs &e) {
    // false means we keep the event bubbling
    return false;
}
       
bool Manager :: keyReleased(ofKeyEventArgs &e){
    //false means we keep the event bubbling
    return false;
}


bool Manager :: deleteLaser(Laser* laser) {
    
    int laserindex = getLaserIndex(laser);
    
    bool success = ManagerBase::deleteLaser(laser);
    if(success) {
        if(selectedLaserIndex>=getNumLasers()) selectedLaserIndex = getNumLasers()-1;
        if(getNumLasers()==0) {
            showLaserOutputSettingsWindow = false;
            viewMode = OFXLASER_VIEW_CANVAS;
        }
        
        if(laserindex!=-1) {
            // laserZoneViews is not a pointer so memory should be freed
            laserZoneViews.erase(laserZoneViews.begin() + laserindex);
        }
        
        return true;
    } else {
        return false;
    }
    
}

void Manager::selectNextLaser() {
    if(lasers.size()>1) {
        int next = selectedLaserIndex+1;
        if(next>=(int)lasers.size()) next=0;
        
        setSelectedLaserIndex(next);
    } else {
        if(viewMode == OFXLASER_VIEW_OUTPUT) {
            viewMode = OFXLASER_VIEW_CANVAS;
        } else {
            viewMode = OFXLASER_VIEW_OUTPUT;
        }
    }
}

void Manager::selectPreviousLaser() {
    int prev = selectedLaserIndex-1;
    if(prev<0) prev=(int)lasers.size()-1;
    setSelectedLaserIndex(prev);
    
}
int Manager::getSelectedLaserIndex(){
    return selectedLaserIndex;
}
void Manager::setSelectedLaserIndex(int i){
    if((selectedLaserIndex!=i) && (i<getNumLasers())) {
        selectedLaserIndex = i;
    }
}

Laser* Manager::getSelectedLaser() {
    
    if((selectedLaserIndex>=0) && (selectedLaserIndex<lasers.size())) {
        return lasers.at(selectedLaserIndex);
    } else {
        return nullptr;
    }
}
    


int Manager::getLaserIndex(Laser* laser) {
    for(int i = 0; i<lasers.size(); i++) {
        if(lasers[i] == laser) {
            return i;
        }
    }
    return -1; 
    
    
}

bool Manager::setGuideImage(string filename){
    return guideImage.load(filename);
}

void Manager:: drawUI(){
    drawPreviews();
    ofxLaser::UI::startGui();
    drawLaserGui();
    finishLaserUI();
}

void Manager :: startLaserUI() {
    ofxLaser::UI::updateGui();
    ofxLaser::UI::startGui();
    
}
void Manager :: finishLaserUI() {
    ofxLaser::UI::render();
    renderCustomCursors();
}

void Manager :: renderCustomCursors() {
    
}

void Manager :: drawPreviews() {
    
    // update visibility and size on views
    for(LaserZoneViewController& laserZoneView : laserZoneViews) {
        laserZoneView.setIsVisible((viewMode==OFXLASER_VIEW_OUTPUT) &&(laserZoneView.laser==getSelectedLaser()));
        laserZoneView.setOutputRect(getZonePreviewRect(), true);
    }
    
    canvasViewController.setIsVisible(viewMode==OFXLASER_VIEW_CANVAS);
    
    
    if(viewMode == OFXLASER_VIEW_3D) {
        
        float previewheight = (ofGetHeight()/2)-menuBarHeight-iconBarHeight;
        ofRectangle rect3D(10,menuBarHeight+iconBarHeight,previewheight/9*16, previewheight); // 16/9 ratio
        // Draw 3D visualiser
        visualiser3D.draw(rect3D, lasers, true);
        
        // this is same as other views - should break it out
        if(showOutputPreviews) {
            // int numrows = 1;
            float outputpreviewscale = 0.375;
            float outputpreviewsize = 800*outputpreviewscale;
            
            float spaceatbottom = (ofGetHeight() - getPreviewRect().getBottom() ) -(guiSpacing*2);
            if (spaceatbottom<50) spaceatbottom = 50;
            if(outputpreviewsize>spaceatbottom) outputpreviewsize = spaceatbottom;
            
            // so we have spaceatbottom which is the gap at the bottom
            // then we have the height and width of the previews
            // which is outputpreviewsize.
            // We know this will fit vertically but we don't know if it
            // fit horizontally
            float availablespace = ofGetWidth()-guiLaserSettingsPanelWidth- (guiSpacing*2);
            if(outputpreviewsize*lasers.size() > availablespace) {
                outputpreviewsize = (availablespace/lasers.size())-guiSpacing;
            }
            
            for(size_t i= 0; i<lasers.size(); i++) {
                
                ofRectangle laserOutputPreviewRect(guiSpacing+((outputpreviewsize+guiSpacing)*i),ofGetHeight()-guiSpacing-outputpreviewsize,outputpreviewsize,outputpreviewsize);
                
                ofFill();
                ofSetColor(0);
                ofDrawRectangle(laserOutputPreviewRect);
                
            }
            
        }
        
        
    } else if(viewMode == OFXLASER_VIEW_CANVAS) {
        
        

       
         
        canvasViewController.drawFrame();
        canvasViewController.beginViewPort();
        canvasViewController.drawEdges();
        canvasViewController.drawMoveables();
        renderPreview();
        canvasViewController.endViewPort();
    

        //
//        if(showInputPreview) {
//
//            ofPushStyle();
//            ofFill();
//            ofSetColor(0);
//            ofPushMatrix();
//            ofTranslate(previewOffset);
//            ofScale(previewScale);
//            ofDrawRectangle(0,0,canvasTarget.getWidth(), canvasTarget.getHeight());
//            ofPopMatrix();
//            ofPopStyle();
//
//
//            if(showInputZones) {
//                // this renders the input zones in the graphics source space
//                for(size_t i= 0; i<zones.size(); i++) {
//                    zones[i]->setOffsetAndScale(previewOffset,previewScale);
//                    // zones[i]->setEditable(!lockInputZones);
//                    zones[i]->setVisible(true);
//                    zones[i]->draw();
//                }
//            } else {
//                // this renders the input zones in the graphics source space
//                for(size_t i= 0; i<zones.size(); i++) {
//
//                    zones[i]->setVisible(false);
//
//                }
//            }
//
//            renderPreview();
//        }
        
    } else if(viewMode == OFXLASER_VIEW_OUTPUT){
        
        // make the selected laser visible
        for(LaserZoneViewController& laserZoneView : laserZoneViews) {
            laserZoneView.update();
            laserZoneView.draw();
            if(laserZoneView.getIsVisible()) drawBigNumber(laserZoneView.getLaserIndex());
        }
        
    }
    
    
    
}
void Manager :: renderPreview() {
    ofPushStyle();

    // Draw laser graphics preview ----------------
    ofMesh mesh;
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    mesh.clear();
    mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    // TODO - render preview move to canvas object!
    // Which then also means pushing all the bitmap mask,
    // guide image, width height etc into the canvas object!
    
    std::deque <ofxLaser::Shape*>& shapes = canvasTarget.shapes;
    // Draw the preview laser graphics, with zones overlaid
    for(size_t i= 0; i<shapes.size(); i++) {
        shapes[i]->addPreviewToMesh(mesh);
    }
    
//    if(useBitmapMask) {
//        ofRectangle laserRect(0,0,canvasTarget.getWidth(), canvasTarget.getHeight());
//        const vector<glm::vec3>& points = mesh.getVertices();
//        std::vector<ofFloatColor>& colours = mesh.getColors();
//
//        for(size_t i= 0;i<points.size(); i++ ){
//
//            ofFloatColor& c = colours.at(i);
//            const glm::vec3& p = points[i];
//
//
//            float brightness;
//
//            if(laserRect.inside(p)) {
//                brightness = laserMask.getBrightness(p.x, p.y);
//            } else {
//                brightness = 0;
//            }
//
//            c.r*=brightness;
//            c.g*=brightness;
//            c.b*=brightness;
//
//        }
//    }
    
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
    
    
    if(showGuideImage && guideImage.isAllocated()) {
        ofSetColor(guideImageColour);
        guideImage.draw(0,0,canvasTarget.getWidth(), guideImage.getHeight() * ((float)canvasTarget.getWidth()/(float)guideImage.getWidth()));
        
        
    }
    ofDisableBlendMode();

    ofPopStyle();
}

void Manager::setCanvasSize(int width, int height) {
    ManagerBase::setCanvasSize(width, height);
    setDefaultPreviewOffsetAndScale();
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

void Manager::addCustomParameter(ofAbstractParameter& param, bool loadFromSettings){
    customParams.add(param);
    if(loadFromSettings){
        //loadJson.find("custom");
        if(!loadedJson.empty()) {
            if(loadedJson.contains("Laser")) {
                if(loadedJson["Laser"].contains("CUSTOM_PARAMETERS")) {
                    //     auto value = loadJson["Laser"]["Custom"][param.getName()];
                    try {
                        ofDeserialize(loadedJson["Laser"]["CUSTOM_PARAMETERS"], param);
                    } catch(...) {
                        
                    }
                    
                }
            }
        }
        //ofDeserialize(loadJson["custom"], param);
        //ofLogNotice(loadJson.dump(3));
        //ofLogNotice(loadJson["Laser"].dump(3));
        //ofLogNotice(loadJson["Laser"]["Custom"].dump(3));
        //ofLogNotice(loadJson["Laser"]["Custom"][param.getName()].dump(3));
        
    }
}

bool Manager::togglePreview(){
    showInputPreview = !showInputPreview;
    return showInputPreview;
};

glm::vec2 Manager::screenToLaserInput(glm::vec2& pos){
    
    glm::vec2 returnpos= pos ;//
    return returnpos;
    
}


void Manager::drawLaserGui() {
    
    ImGui::ShowDemoWindow();
    
    guiMenuBar();
    guiTopBar(menuBarHeight-1);
    guiLaserOverview();
    guiLaserSettings(getSelectedLaser());
    guiCustomParameters();
    
    Laser* currentLaser = getSelectedLaser();
        
        // TODO check null laser
    
    if(viewMode == OFXLASER_VIEW_OUTPUT){
    
        if(UI::startWindow("Laser select", ImVec2(10,60), ImVec2(800,iconBarHeight),ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize, true, nullptr )) {
            
            ImGui::PushFont(UI::mediumFont);
            
            if(ImGui::Button(ICON_FK_PLUS, ImVec2(0,22))) {
                createAndAddLaser();
                saveSettings();
            }
            ImGui::SameLine();

           
            //ImGui::SetCursorPosY(ImGui::GetCursorPosY()+3);
            
            ImGui::Text("LASER :"); ImGui::SameLine();
            //ImGui::SetCursorPosY(ImGui::GetCursorPosY()-3);
            
            for(int i = 0; i< getNumLasers(); i++ ) {
                
                string laserNumberString = ofToString(i+1);
               
                if(i==getSelectedLaserIndex()) UI::secondaryColourStart();
                // LASER BUTTONS
                if(ImGui::Button(laserNumberString.c_str(), ImVec2(25,22))) {
                    setSelectedLaserIndex(i);
                }
                ImGui::SameLine();
                UI::secondaryColourEnd();
                
            }
            ImGui::PopFont();
        }
        UI::endWindow();
    
        //string label ="Add zone ";
        
        if(UI::startWindow("Zone View Icons", ImVec2(10,100), ImVec2(800,iconBarHeight), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar , true, nullptr )) {
            
            
            ImGui::PushFont(UI::mediumFont);
            //ImGui::PushItemWidth(170);
            
            if(UI::Button(ofToString(ICON_FK_LIB_ADDZONE) + "##AddZone", false, false)) {
                
                // MAKE NEW ZONE panel
                //ImGui::OpenPopup("Add zone");
                addCanvasZone(0,0,200,200);
                int zonenum = canvasTarget.zones.size()-1;
                int lasernum = getSelectedLaserIndex();
                addZoneToLaser(zonenum, lasernum);
                
            }
            UI::addDelayedTooltip("Add new zone");
            
            if(UI::Button(ICON_FK_LIB_ADDMASK, false, false)) {
                
                // MAKE NEW MASK panel
                currentLaser->maskManager.addQuadMask();
            }
            UI::addDelayedTooltip("Add new mask");

            if(UI::Button(ICON_FK_LIB_TESTPATTERN, false, currentLaser->testPatternActive)) {
                currentLaser->testPatternActive = !currentLaser->testPatternActive;
            }
            UI::addDelayedTooltip("Show test pattern");
            
            if(!currentLaser->testPatternActive) {
                
                UI::startGhosted();
            }
            ImGui::PushItemWidth(21);
            if(ImGui::DragInt("##testpattern", &currentLaser->testPattern, 0.2, 1, TestPatternGenerator::getNumTestPatterns())) {
            }
            UI::addDelayedTooltip("Select test pattern");
           
                
            UI::stopGhosted();
            
            ImGui::PopItemWidth();

            
            
            if(UI::Button(ICON_FK_LIB_GRID, false, zoneGridSnap.get())) {
                zoneGridSnap.set(!zoneGridSnap.get());
                // choose test pattern
                
            }
            UI::addDelayedTooltip("Snap to Grid");
            
            ImGui::PushItemWidth(21);
            if(!zoneGridSnap.get()) {
                UI::startGhosted();
            }
            int gridExponent = 0; //zoneGridSize.get();
            while(pow(2,gridExponent) < zoneGridSize.get())
                gridExponent++;
            if(ImGui::DragInt("##gridexponent", &gridExponent, 0.3, 0, 8)) {
                zoneGridSize.set(pow(2,gridExponent));
            }
            UI::stopGhosted();
            UI::addDelayedTooltip("Grid size");
            //ImGui::PopItemWidth();
            ImGui::PopFont();
            
//            for(int i = 0; i<0xff; i++) {
//                string icon = "\xef\x8e";
//                icon = icon +  (char)i;
//                //cout << icon;
//                ImGui::Text("%s%s ", ofToHex(i+0xef8e00-0xee9a40).c_str(), icon.c_str());
//                ImGui::SameLine();
//
//            }
        }
        
        /*
        if(ImGui::BeginPopupModal("Add zone", NULL)) {
            
            static int type = 0;
            
            if(type==0) UI::secondaryColourStart();
            if(ImGui::Button("QUAD")) {
                type = 0;
            }
            UI::secondaryColourEnd();
            ImGui::SameLine();
            if(type==1)  UI::secondaryColourStart();
            if(ImGui::Button("LINE")) {
                type = 1;
            }
            UI::secondaryColourEnd();
            
            ImGui::Separator();

            // TODO add selector to add existing zone
            // TODO add lable
            
            
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
               
            }
            
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                
                addZone(0,0,200,200);
                int zonenum = zones.size()-1;
                int lasernum = getSelectedLaserIndex();
                addZoneToLaser(zonenum, lasernum);
             
                
                ImGui::CloseCurrentPopup();
                
            }
            
            
            ImGui::EndPopup();
        }*/
        
        UI::endWindow();
        
       
        
    }
    

    // show laser settings :
   
    ofxLaser::Manager& laserManager = *this;
    
    // Show laser zone settings mute / solo / etc
    //if(viewMode == OFXLASER_VIEW_OUTPUT)  {

        // LASER ZONE SETTINGS
        //guiLaserOutputSettings();
    //}
    guiDacAssignment();
    
    visualiser3D.drawUI();

    for(LaserZoneViewController& laserzoneview : laserZoneViews ) {
        laserzoneview.drawImGui();
    }
    canvasViewController.drawImGui();
    guiCopyLaserSettings();
    
}

void Manager::guiLaserOutputSettings() {
    
    Laser* laser = lasers[selectedLaserIndex];
    vector<OutputZone*> activeZones = laser->getActiveZones();
    
    if(UI::startWindow("Laser output zones", ImVec2(800+guiSpacing, guiSpacing+menuBarHeight), ImVec2(380,500))) {
        
        ImGui::PushFont(UI::largeFont);
        ImGui::Text("%s", laser->getLabel().c_str());
        ImGui::PopFont();
      
        ImGuiTreeNodeFlags collapsingheaderflags = ImGuiTreeNodeFlags_None;
        if(activeZones.size()==0) collapsingheaderflags|=ImGuiTreeNodeFlags_DefaultOpen;
        
        if (ImGui::CollapsingHeader("Add / remove zones and masks",collapsingheaderflags)) {
                ImGui::Columns(3, "Laser zones columns");

            // ADD / REMOVE ZONES
            ImGui::Text("Zones");
            for(InputZone* zone : canvasTarget.zones) {
                int zoneIndex = zone->getIndex();
                
                bool checked = laser->hasZone(zoneIndex);

                if(ImGui::Checkbox(ofToString(zoneIndex+1).c_str(), &checked)) {
                    if(checked) {
                        laser->addZone(zoneIndex);
                    } else {
                        laser->removeZone(zoneIndex);
                    }
                }
            }
            ImGui::NextColumn();
            ImGui::Text("Alternate zones");
            for(InputZone* zone : canvasTarget.zones) {
                int zoneIndex = zone->getIndex();
                bool checked = laser->hasAltZone(zoneIndex);
                string label = ofToString(zoneIndex+1) + "##alt";
                if(ImGui::Checkbox(label.c_str(), &checked)) {
                    if(checked) {
                        laser->addAltZone(zoneIndex);
                    } else {
                        laser->removeAltZone(zoneIndex);
                    }
                }
            }
            ImGui::NextColumn();
            
            
            // ADD / REMOVE MASKS
            MaskManager& maskManager = laser->maskManager;
            if(ImGui::Button("ADD MASK")) {
                maskManager.addQuadMask();
            }
            ImDrawList*   draw_list = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos();
            draw_list->AddLine(ImVec2(p.x - 9999, p.y), ImVec2(p.x + 9999, p.y),  ImGui::GetColorU32(ImGuiCol_Border));
            ImGui::Dummy(ImVec2(0.0f, 2.0f));
            
            for(size_t i = 0; i<maskManager.quads.size(); i++){
                QuadMask* mask = maskManager.quads[i];
                string label = "##"+ofToString(i+1);
                ImGui::Text("MASK %lu", i+1);
                ImGui::SameLine();
                ImGui::PushItemWidth(40);
                int level = mask->maskLevel;
                if (ImGui::DragInt(label.c_str(),&level,1,0,100,"%d%%")) {
                    mask->maskLevel = level;
                }
                
                ImGui::PopItemWidth();
                ImGui::SameLine();
                string buttonlabel = "DELETE "+ofToString(i+1)+"##mask";
                if(ImGui::Button(buttonlabel.c_str())) {
                    maskManager.deleteQuadMask(mask);
                    
                }
                
            }
            ImGui::Columns();
        // -------------------
        }
        
        
        
       //
        ImGui::Separator();
        
        UI::addCheckbox(laser->useAlternate);
        UI::addCheckbox(laser->muteOnAlternate);
        
        bool soloActive = laser->areAnyZonesSoloed();
        
        // ZONE MUTE SOLO
       
        for(OutputZone* laserZone : laser->outputZones) {
            
            if(laserZone->getIsAlternate()) continue;
            
            bool zonemuted = laserZone->muted;
            
//            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, soloActive?0.5f:1.0f);
            if(soloActive) UI::startGhosted();
            string muteLabel = "MUTE##"+laserZone->getLabel();
            if(zonemuted) UI::secondaryColourStart();
            if(ImGui::Button(muteLabel.c_str())) { // }, ImVec2(20,20))) {
                //laser->toggleZoneMute(laserZone);
//                laserZone->muted = !laserZone->muted;
                if(laserZone->muted) { laser->unMuteZone(laserZone->getZoneIndex());
                } else {
                    laser->muteZone(laserZone->getZoneIndex());
                }
            };
            UI::addDelayedTooltip("Mute zone");
            
            if(zonemuted) UI::secondaryColourEnd();
            //ImGui::PopStyleVar();
            UI::stopGhosted();
            
            ImGui::SameLine();
            bool soloed = laserZone->soloed;
            if(soloed) UI::secondaryColourStart();
            string soloLabel = "SOLO##"+laserZone->getLabel();
            if(ImGui::Button(soloLabel.c_str())) { // }, ImVec2(20,20))){
                //laser->toggleZoneSolo(laserZone);
               // laserZone->soloed = !laserZone->soloed;
                if(laserZone->soloed) { laser->unSoloZone(laserZone->getZoneIndex());
                } else {
                    laser->soloZone(laserZone->getZoneIndex());
                }
            }
            UI::secondaryColourEnd();
            UI::addDelayedTooltip("Solo zone");
            
            ImGui::SameLine();
            ImGui::Text("ZONE %s",laserZone->getLabel().c_str());
           
            
        }
        
        ImGui::Separator();
        
        UI::addIntSlider("Test Pattern", laser->testPattern, 1, TestPatternGenerator::getNumTestPatterns());
        UI::addCheckbox(laser->hideContentDuringTestPattern);
        UI::toolTip("Disable this if you want to see the laser content at the same time as the test patterns");
        
        UI::addParameter(zoneEditorShowLaserPath);
        UI::addParameter(zoneEditorShowLaserPoints);
       
        if(laser->paused && ((ofGetElapsedTimeMillis()%600)<300)) UI::startGhosted();
        string label = ofToString(ICON_FK_PLAY ) + ofToString(ICON_FK_PAUSE);
        if(UI::Button( label, false, laser->paused)) {
            laser->paused = !laser->paused;
        }
        UI::stopGhosted();
        
        //UI::addCheckbox(laser->paused);
        UI::toolTip("Pauses the output of the laser (useful for adjusting the settings)");
        
        UI::addParameter(zoneGridSnap);
        UI::addParameter(zoneGridSize);
        
        // Laser Output Zones
        
        for(OutputZone* laserZone : laser->outputZones) {
           
            if(true) { // laserZone->getSelected()) {

                ImGui::Separator();
                ImGui::PushFont(UI::largeFont);
                ImGui::Text("Zone %d", laserZone->getZoneIndex()+1);
                ImGui::PopFont();
                ImGui::PushID(laserZone->getLabel().c_str());
                
                bool lineZone = laserZone->transformType == 1;
                
                if(!lineZone) UI::secondaryColourStart();
                if(ImGui::Button("QUAD")) {
                    laserZone->transformType = 0;
                }
                UI::secondaryColourEnd();
                ImGui::SameLine();
                if(lineZone)  UI::secondaryColourStart();
                if(ImGui::Button("LINE")) {
                    laserZone->transformType = 1;
                }
                UI::secondaryColourEnd();

                if(!lineZone) {
                    ZoneTransformQuadData* ztq = dynamic_cast<ZoneTransformQuadData*>(&laserZone->getZoneTransform());
                    if(ztq!=nullptr) {
                        if(ztq->isAxisAligned()) {
                            UI::startGhosted();
                        }
                        if(UI::Button("Reset to square")) {
                            ztq->resetToSquare();
                           
                        }
                        UI::stopGhosted();
                        UI::toolTip("Removes any distortion in the zone and makes all the corners right angles");
                        ImGui::SameLine();
                        if(UI::Button(ICON_FK_SQUARE_O))  {
                            ztq->setDst(ofRectangle(200,240,400,200));
                        }
                        UI::toolTip("Reset zone to default");
                        
                        UI::addParameterGroup(ztq->transformParams, false);
                    }
                } else {
                        
                    ZoneTransformLineData* ztl = dynamic_cast<ZoneTransformLineData*>(&laserZone->getZoneTransform());
                    if(ztl!=nullptr) {
                        //UI::addParameterGroup(laserZone->getZoneTransform().transformParams, false);
                        
                        UI::addFloatSlider(ztl->zoneWidth, "%.2f", 3);
                        
                        vector<BezierNode>& nodes = ztl->getNodes();
                        for(int i = 0; i<nodes.size(); i++) {
                            ImGui::PushID(i);
                            BezierNode& node = nodes[i];
                            int mode = node.mode;
                            ImGui::Text("%d", i+1);
                            ImGui::SameLine();
                            //ofxLaser::UI::addCheckbox(synchroniser->useMidi);
                            ImGui::RadioButton("LINES", &mode, 0); ImGui::SameLine();
                            ImGui::RadioButton("FREE BEZIER", &mode, 1); ImGui::SameLine();
                            ImGui::RadioButton("SMOOTH BEZIER", &mode, 2);
                            
                            if(mode!=node.mode) {
                                node.mode = mode;
                                ztl->setDirty(true);
                            }
                            if(nodes.size()>2) {
                                ImGui::SameLine();
                                
                                string label = ofToString(ICON_FK_MINUS_CIRCLE) + "##" + ofToString(i);
                                if (UI::DangerButton(label, false)) {
                                    ztl->deleteNode(i);
                                    
                                }
                            }
                            
                            ImGui::PopID();
                            
                            
                        }
                        label = ofToString(ICON_FK_PLUS_CIRCLE) + "##addnode";
                        if (UI::Button(label, false)) {
                            ztl->addNode();
                            
                        }
                    }
                    
                }
                ImGui::PopID();
               
            }
             
        }
        
    }
    UI::endWindow();
    
    
    
}

void Manager::guiMenuBar() {
    
    ImGui::PushFont(UI::mediumFont);
    ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 8, 12 ) );
    
    if (ImGui::BeginMainMenuBar())
    {
//        if (ImGui::BeginMenu("File"))
//        {
//            //ShowExampleMenuFile();
//            ImGui::EndMenu();
//        }
//        if (ImGui::BeginMenu("Edit"))
//        {
////            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
////            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
////            ImGui::Separator();
////            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
////            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
////            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
//            ImGui::EndMenu();
//        }
        if(ImGui::BeginMenu("View") ) {
            if(ImGui::BeginMenu("Set up") ) {
                if (ImGui::MenuItem("Laser overview", "CMD+L", showLaserManagementWindow)) {
                    showLaserManagementWindow = !showLaserManagementWindow;
                }
                if (ImGui::MenuItem("Laser output settings", "", showLaserOutputSettingsWindow)) {
                    showLaserOutputSettingsWindow = !showLaserOutputSettingsWindow;
                }
    //            if (ImGui::MenuItem("Scanner settings", "CMD+L", showScannerSettingsWindow)) {
    //                showScannerSettingsWindow = !showScannerSettingsWindow;
    //                //ImGui::SetWindowFocus("Laser Settings");
    //            }
                if (ImGui::MenuItem("Controller assignment", "CMD+D",showDacAssignmentWindow )) {
                    showDacAssignmentWindow= !showDacAssignmentWindow;
                   //ImGui::SetWindowFocus("Controller Assignment");
                }
                if (ImGui::MenuItem("3D Visualiser settings", "CMD+D", visualiser3D.showSettingsWindow)) {
                    visualiser3D.showSettingsWindow = !visualiser3D.showSettingsWindow;
                }
                if ((customParams.size()>0) && (ImGui::MenuItem("Custom parameters", "", showCustomParametersWindow))) {
                    showCustomParametersWindow = !showCustomParametersWindow;
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        menuBarHeight = ImGui::GetWindowHeight();
        ImGui::EndMainMenuBar();
    }
    ImGui::PopFont();
    ImGui::PopStyleVar();
    
    
}

void Manager :: guiLaserOverview() {
    
    if(!showLaserManagementWindow) return;
    ofxLaser::Manager& laserManager = *this;
    
    // calculate x position of main window
    int x = ofGetWidth() - guiLaserSettingsPanelWidth - guiSpacing;

    if(UI::startWindow("Laser overview", ImVec2(x, guiSpacing+menuBarHeight), ImVec2(guiLaserSettingsPanelWidth, 0),0,false, (bool*)&showLaserManagementWindow.get())){
        
        float buttonwidth = (guiLaserSettingsPanelWidth-(guiSpacing*3))/2;

        if(UI::Button(ICON_FK_LIB_TESTPATTERN, false, testPatternGlobalActive)) {
            testPatternGlobalActive = !testPatternGlobalActive;
            updateGlobalTestPattern();
        }
        ImGui::SameLine();
        
        ImGui::PushItemWidth(140);
        if(!testPatternGlobalActive) UI::startGhosted();
        if(UI::addIntSlider("Global Test Pattern", testPatternGlobal, 1, TestPatternGenerator::getNumTestPatterns())) {
            updateGlobalTestPattern();
        }
        UI::stopGhosted();
        ImGui::PopItemWidth();
//
//        if(dontCalculateDisconnected) UI::secondaryColourStart();
//        string label = "OPTIMSED MODE : ";
//        if(dontCalculateDisconnected) {
//            label+="ON";
//        } else {
//            label+="OFF";
//        }
//        if(UI::Button(label) ){
//            dontCalculateDisconnected = !dontCalculateDisconnected.get();
//
//        }
//        UI::secondaryColourEnd();
           
        
        // SHOW LIST OF LASERS
        
        for(int i = 0; i<getNumLasers(); i++) {
            
            string label;
            
            ofxLaser::Laser* laserobject = &laserManager.getLaser(i);
            string laserNumberString = ofToString(i+1);
           
            // LASER BUTTONS
            if(ImGui::Button(laserNumberString.c_str(), ImVec2(20,0))) {
                if((viewMode == OFXLASER_VIEW_CANVAS) || (selectedLaserIndex!=i)) {
                    selectedLaserIndex = i;
                    viewMode = OFXLASER_VIEW_OUTPUT;
                } else {
                    viewMode = OFXLASER_VIEW_CANVAS;
                }
            }
            ImGui::SameLine();
            
            // ARM BUTTONS
            if(laserobject->armed) {
                UI::dangerColourStart();
    
            }
            string armlabel = "ARM##"+ofToString(i+1);
            if(ImGui::Button(armlabel.c_str())){
                laserobject->toggleArmed();
            }
            UI::dangerColourEnd();
            
            ImGui::SameLine();
            
            // LASER SETTINGS BUTTONS
            label = ofToString(ICON_FK_SLIDERS)+"##"+ofToString(i);
            if(ImGui::Button(label.c_str())) {
                showLaserOutputSettingsWindow = true;
                selectedLaserIndex = i;
            }
            
            ImGui::SameLine();
            
            // ZONE ADJUST BUTTONS
            label = ofToString(ICON_FK_PENCIL_SQUARE_O)+"##"+ofToString(i);
            
            if(ImGui::Button(label.c_str())) {
                if((viewMode == OFXLASER_VIEW_CANVAS) || (selectedLaserIndex!=i)) {
                    selectedLaserIndex = i;
                    viewMode = OFXLASER_VIEW_OUTPUT;
                } else {
                    viewMode = OFXLASER_VIEW_CANVAS;
                }
            }
            ImGui::SameLine();
             
            if(laserobject->paused && (((ofGetElapsedTimeMillis()-(i*15))%600)<300)) UI::startGhosted();
            label = ofToString(ICON_FK_PAUSE)+"##"+ofToString(i);
            if(UI::Button( label, false, laserobject->paused)) {
                laserobject->paused = !laserobject->paused;
            }
            UI::stopGhosted();
            
            // FRAME RATES
            
            ImGui::SameLine();
            label = "##framerate"+laserNumberString;
            ImGui::PushItemWidth(60);
            ImGui::PlotHistogram(label.c_str(), laserobject->frameTimeHistory, laserobject->frameTimeHistorySize, laserobject->frameTimeHistoryOffset, "", 0, 0.1f);
            ImGui::PopItemWidth();
            
            // DAC STATUSES
            ImGui::SameLine();
            
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 size = ImVec2(15,15); // ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
            
            int radius = 4;
            ImVec2 p = ImGui::GetCursorScreenPos();
            p.x+=0;
            p.y+=2;
    //        p.x+=radius-2;
    //        p.y+=radius+4;
            ImU32 col = UI::getColourForState(laserobject->getDacConnectedState());
            
            //draw_list->AddCircleFilled(p,radius, col);
            //ImGui::InvisibleButton("##invisible", ImVec2(radius*2, radius*2) - ImVec2(2,2));
            
            draw_list->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), col);
            ImGui::InvisibleButton("##gradient2", size - ImVec2(0,2));

            if (lasers.size() > 1) {
                ImGui::SameLine();
                
                if(guiDeleteLaserButtonAndPopup(laserobject, i)) {
                    // then laser object was deleted!
                    laserobject = nullptr;
                }
            }
           
            
            if(laserobject!=nullptr) {
                float framerate = laserobject->getFrameRate();
                if(framerate!=INFINITY) {
                    ImGui::SameLine();
                    label = ofToString(round(framerate));
                    ImGui::Text("%s",label.c_str());
                }
            }
        
            
        }
        
        string label = "ADD LASER";
        if(UI::Button( label, false, false)) {
            // add laser
            laserManager.createAndAddLaser();
            saveSettings(); 
        }
    
        ImGui::SameLine();
        if(ImGui::Button("ADD ZONE", ImVec2(buttonwidth, 0.0f))) {
            addCanvasZone(0,0,280,110);
            lockInputZones = false;
            saveSettings();
        }
    
        if(UI::Button("CONTROLLERS", false, false)) {
            // add laser
            showDacAssignmentWindow = true;
            ImGui::SetWindowFocus("Controller Assignment");
            
        }
        UI::addIntSlider(globalLatency);
        
        if(viewMode == OFXLASER_VIEW_CANVAS) {
//            if(UI::addParameter(canvasTarget.getWidth())) {
//                saveSettings();
//            }
//           
//            if(UI::addParameter(canvasTarget.getHeight())) {
//                saveSettings();
//            }
                
            
            if(UI::Button("LOAD CANVAS GUIDE IMAGE")) {
                
                ofFileDialogResult dialogResult = ofSystemLoadDialog();
                if(dialogResult.bSuccess) {
                    if(setGuideImage(dialogResult.filePath)){
                        showGuideImage =true;
                        guideImage.save("guideimages/"+dialogResult.getName());
                        guideImageFilename = dialogResult.getName();
                        saveSettings();
                    }
                }
            }
            if(guideImage.isAllocated()) {
                UI::addParameter(showGuideImage);
                UI::addParameter(guideImageColour);
            }
                
            if(UI::addParameter(lockInputZones)) {
                saveSettings();
            }
        }
    }
    
    UI::endWindow();
   
}

void Manager :: guiLaserSettings(ofxLaser::Laser* laser) {
    // TODO CHECK THIS FOR PUSH POP STYLES

    if(!showLaserOutputSettingsWindow) return;
    
    int x = ofGetWidth() - (guiLaserSettingsPanelWidth + guiSpacing);
    
    if(UI::startWindow("Laser output", ImVec2(x,200), ImVec2(guiLaserSettingsPanelWidth,0), ImGuiWindowFlags_None, false, (bool*)&showLaserOutputSettingsWindow.get())) {
        
        int laserIndexToShow = selectedLaserIndex;
        if(laserIndexToShow ==-1) laserIndexToShow = 0;
        // drawUIPanelScannerSettings(&getLaser(laserIndexToShow), laserpanelwidth, spacing, 0);
        
        // draw a flashing dot during saving
        if(laser->getSaveStatus() && (ofGetElapsedTimeMillis()%300)<150) {
            ImDrawList*   draw_list = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetWindowPos();
            p.x+=ImGui::GetContentRegionAvailWidth();
            p.y+=30;// + ImGui::GetScrollY();
            // if(ImGui::GetScrollY()>0) p.x-=14;
            //ImGui::GetContentRegionAvailWidth()
            draw_list->AddCircleFilled(p, 4, ImGui::GetColorU32(ImGuiCol_Border));
        }
        
        
        
        // ARM BUTTON ---------------------------------------------------------------
        UI::largeItemStart();
        ImGui::PushItemWidth(140);
        string label = "ARM";
        if(UI::addNumberedCheckBox(laser->laserIndex+1, label.c_str(), (bool*)&laser->armed.get(), true, true)) {
            laser->armed.set(laser->armed.get()); // trigger the events
        }
        ImGui::PushItemWidth(guiLaserSettingsPanelWidth-(guiSpacing*2));
        float multiplier = 100;
        int value = laser->intensity*multiplier;
        if (ImGui::SliderInt("##int", &value, laser->intensity.getMin()*multiplier, laser->intensity.getMax()*multiplier, "BRIGHTNESS %d")) {
            laser->intensity.set((float)value/multiplier);
            
        }
        ImGui::PopItemWidth();
        UI::largeItemEnd();
        
        // TEST PATTERN ---------------------------------------------------------------
        if(UI::Button(ICON_FK_LIB_TESTPATTERN, false, laser->testPatternActive)) {
            laser->testPatternActive = !laser->testPatternActive;
        }
        UI::addDelayedTooltip("Show test pattern");
        ImGui::SameLine();
        UI::addIntSlider("Test Pattern", laser->testPattern, 1, TestPatternGenerator::getNumTestPatterns());
        
        // ORIENTATION ----------------------------------------------------------------
        static bool selected[4] = { false, false, false, false};
        static string icon[4] = {ICON_FK_ARROW_UP, ICON_FK_ARROW_RIGHT, ICON_FK_ARROW_DOWN, ICON_FK_ARROW_LEFT};
        for (int i = 0; i < 4; i++)
        {
            selected[i] = (i==laser->mountOrientation);
            
            if(i!=laser->mountOrientation) UI::startGhosted();
            if (UI::Button(icon[i].c_str())){
                laser->mountOrientation = i;
            }
            UI::stopGhosted();
            ImGui::SameLine();
        }
        ImGui::Text("Orientation");
        UI::toolTip("The orientation of the laser itself. Useful if you are rigging the laser sideways or upside down.");
        
        // FINE OUTPUT SETTINGS -------------------------------------------------------
        bool treevisible = ImGui::TreeNode("Fine position adjustments");
        
        UI::toolTip("These affect all output zones for this laser and can be used to re-align the output if moved slightly since setting them up");
        if (treevisible){
            ofParameter<float>& param = laser->rotation;
            if(ImGui::DragFloat("Rotation", (float*)&param.get(), 0.01f,-30,30)) { //  param.getMin(), param.getMax())) {
                param.set(param.get());
                
            }
            if(laser->rotation!=0) {
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                if (ImGui::Button("Reset")) laser->rotation = 0;
            }
            
            ofParameter<glm::vec2>& param2 = laser->outputOffset;
            if(ImGui::DragFloat2("Position", (float*)&param2.get().x, 0.01f, -50.0f,50.0f)) { //  param.getMin(), param.getMax())) {
                param2.set(param2.get());
                
            }
            
            glm::vec2 zero2;
            if(laser->outputOffset.get()!=zero2) {
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                if (ImGui::Button("Reset")) laser->outputOffset.set(zero2);
            }
            // ImGui::PopItemWidth();
            ImGui::TreePop();
        }
        
        // FLIP X / Y ----------------------------------------------------------------
        UI::addCheckbox(laser->flipX);
        ImGui::SameLine();
        UI::addCheckbox(laser->flipY);
        //
        // TODO put this in a menu item
        if(ImGui::Button("COPY LASER SETTINGS")) {
            copySettingsWindowOpen = !copySettingsWindowOpen;
        }

        
        // SCANNER SETTINGS ----------------------------------------------------------
        ImGui::Separator();
        ImGui::Text("Scanner settings");
        UI::largeItemStart();
        ImGui::PushItemWidth(190);
        UI::addFloatAsIntPercentage(laser->speedMultiplier);
        UI::toolTip("Scanner speed adjustment (NB this works mathematically, it doesn't change the point rate)");
        ImGui::PopItemWidth();
        ImGui::PushItemWidth(170);
        UI::addFloatSlider(laser->colourChangeShift);
        UI::toolTip("Shifts the laser colours to match the scanner position (AKA blank shift)");
        ImGui::PopItemWidth();
        UI::largeItemEnd();
        
        
        
        // PresetManager& presetManager = *PresetManager::instance();
        // TODO :
        // check if the settings are different from the preset, if they are
        // show a "save" button, also save as?
        //
        // when an option is selected, update all the params
        
        ScannerSettings& currentPreset = *scannerPresetManager.getPreset(laser->scannerSettings.getLabel());
        scannerPresetManager.drawComboBox(laser->scannerSettings);
        
        ImGui::SameLine();
        
        if(ImGui::Button("EDIT")) {
            //ImGui::OpenPopup("Edit Scanner Preset");
            showEditScannerPreset = true;
        }
        
        // SCANNER PRESET POPUP
        
        if (showEditScannerPreset) {
            
            //    ImGui::SetNextWindowPos,
            ImGui::SetNextWindowSize({760,0});
            // centre popup
            
            ImGui::SetNextWindowPos({(float)ofGetWidth()/2, (float)ofGetHeight()/2}, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            
            
            if(ImGui::Begin("Edit Scanner Preset", &showEditScannerPreset, ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoDocking))
            {
                ImGui::Text("SCANNER SETTINGS - %s",laser->getLabel().c_str());
                ImGui::Separator();
                scannerPresetManager.drawComboBox(laser->scannerSettings);
                
                
                ImGui::SameLine();
                scannerPresetManager.drawSaveButtons(laser->scannerSettings);
                
                UI::addResettableFloatSlider(laser->scannerSettings.moveSpeed, currentPreset.moveSpeed,"How quickly the mirrors move between shapes", "%.1f", 3.0f);
                
                ImGui::Columns(2);
                UI::addResettableIntSlider(laser->scannerSettings.shapePreBlank, currentPreset.shapePreBlank, "The length of time that the laser is switched off and held at the beginning of a shape");
                
                UI::addResettableIntSlider(laser->scannerSettings.shapePreOn, currentPreset.shapePreOn, "The length of time that the laser is switched on and held at the beginning of a shape");
                ImGui::NextColumn();
                
                UI::addResettableIntSlider(laser->scannerSettings.shapePostBlank, currentPreset.shapePostBlank,"The length of time that the laser is switched off and held at the end of a shape" );
                UI::addResettableIntSlider(laser->scannerSettings.shapePostOn, currentPreset.shapePostOn,"The length of time that the laser is switched on and held at the end of a shape" );
                
                ImGui::Columns(1);
                
                ImGui::Text("Render profiles");
                UI::toolTip("Every scanner setting has three profiles for rendering different qualities of laser effects. Unless otherwise specified, the default profile is used. The fast setting is good for long curvy lines, the high detail setting is good for complex pointy shapes.");
                
                ImGui::Separator();
                //bool firsttreeopen = true;
                ImGui::Columns(3);
                ImGui::SetColumnWidth(0, 250);
                ImGui::SetColumnWidth(1, 250);
                ImGui::SetColumnWidth(1, 250);
                
                
                for (auto & renderProfilePair : laser->scannerSettings.renderProfiles) {
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
                
                
                
            }
            ImGui::End();
        }
        
        
        
        // COLOUR SETTINGS
        ImGui::Separator();
        ImGui::Text("COLOUR");
        
        if(ImGui::TreeNode("Colour calibration")){
            colourPresetManager.drawComboBox(laser->colourSettings);
            colourPresetManager.drawSaveButtons(laser->colourSettings);
            UI::addParameterGroup(laser->colourSettings.params);
            
            ImGui::TreePop();
        }
        
        
        
        ImGui::Separator();
        ImGui::Text("ADVANCED SETTINGS");
        
        // ADVANCED
        
        
        if(ImGui::TreeNode("Advanced")){
            // POINT RATE
            ImGui::PushItemWidth(ImGui::GetWindowWidth()-60);
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 26.0f);
            int ppsslider = laser->pps;
            string ppsstring = "Point rate : " + ofToString(ppsslider);
            if(ImGui::SliderInt("##Point rate", &ppsslider, laser->pps.getMin(), laser->pps.getMax(), ppsstring.c_str())){
                laser->pps.set(ppsslider/100*100);
                
            }
            UI::toolTip("The actual points sent to the laser - YOU DON'T NEED TO ADJUST THIS unless you want to actually change the data rate, or you need better resolution for very fast scanners. The speed of the scanners can be fully adjusted without changing the point rate. ");
            ImGui::PopStyleVar(1);
            ImGui::PopItemWidth();
            
            
            UI::addParameterGroup(laser->advancedParams);
            ImGui::TreePop();
            
            
        }
        
        
    }
    
    
    
    UI::endWindow();
    
            
    
    
}


void Manager :: guiTopBar(int ypos) {

    ofxLaser::Manager& laserManager = *this;
    
        if(UI::startWindow("Icon bar", ImVec2(0,ypos), ImVec2(800,iconBarHeight),ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize, true, nullptr )) {

        //ImGui::Columns(3, "Icon bar columns");
    //    ImGui::SetColumnWidth(0, 200);
    //    ImGui::SetColumnWidth(1, 200);
    //    ImGui::SetColumnWidth(3, 100);
       
        bool useRedButton = areAllLasersArmed();
        if(useRedButton) UI::dangerColourStart();
        // change the colour for the arm all button if we're armed
        int buttonwidth = 80;
        if(ImGui::Button(useRedButton ? "ALL ARMED" : "ARM ALL", ImVec2(buttonwidth, 0.0f) )) {
            armAllLasers();
        }
        if(useRedButton) UI::dangerColourEnd();
        
        ImGui::SameLine();
        if(ImGui::Button("DISARM ALL",  ImVec2(buttonwidth, 0.0f))) {
            disarmAllLasers();
        }
        if(hasAnyAltZones()) {
            ImGui::SameLine();
            if(areAllLasersUsingAlternateZones()) UI::dangerColourStart();
            if(ImGui::Button("ALT ZONES",  ImVec2(buttonwidth, 0.0f))) {
                if(areAllLasersUsingAlternateZones()) {
                    unSetAllAltZones();
                } else {
                    setAllAltZones();
                }
            }
            UI::dangerColourEnd();
            
        }
        ImGui::SameLine();
        
        
        float multiplier = 100;
        int value = laserManager.globalBrightness*multiplier;
        if (ImGui::SliderInt("##intglobalbrightnessicon", &value, laserManager.globalBrightness.getMin()*multiplier, laserManager.globalBrightness.getMax()*multiplier, "BRIGHTNESS %d")) {
            laserManager.globalBrightness.set((float)value/multiplier);
            
        }
        ImGui::SameLine();
        //ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
        
        
       
        
        if(UI::Button("3D", false, viewMode==OFXLASER_VIEW_3D)) {
            viewMode = OFXLASER_VIEW_3D;
        }
        ImGui::SameLine();
        if(UI::Button("CANVAS", false, viewMode==OFXLASER_VIEW_CANVAS)) {
            viewMode = OFXLASER_VIEW_CANVAS;
        }
        ImGui::SameLine();
        if(UI::Button("OUTPUT", false, viewMode==OFXLASER_VIEW_OUTPUT)) {
            viewMode = OFXLASER_VIEW_OUTPUT;
        }
        
        
//        ImGui::SameLine();
//        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
//       //ImGui::PushFont(UI::symbolFont);
//        if(UI::Button(ICON_FK_MOUSE_POINTER, false, mouseMode==OFXLASER_MOUSE_DEFAULT)) {
//            mouseMode = OFXLASER_MOUSE_DEFAULT;
//        }
//        ImGui::SameLine();
//        if(UI::Button(ICON_FK_HAND_PAPER_O, false, mouseMode==OFXLASER_MOUSE_DRAG)) {
//            mouseMode = OFXLASER_MOUSE_DRAG;
//        }ImGui::SameLine();
//
//        glm::vec2 centre = glm::vec2(canvasTarget.getWidth()/2, canvasTarget.getHeight()/2);
//        ofxLaser::Laser& currentLaser = *lasers[getSelectedLaserIndex()];
//        
//        if(UI::Button(ICON_FK_PLUS, false, false)) {
//            if(viewMode == OFXLASER_VIEW_CANVAS) {
//                zoomPreviewAroundPoint(centre,1.2);
//            } else if(viewMode == OFXLASER_VIEW_OUTPUT) {
//                //currentLaser.zoomAroundPoint(centre,1.2);
//       
//            }
//        }
//        ImGui::SameLine();
//        if(UI::Button(ICON_FK_MINUS, false, false)) {
//            if(viewMode == OFXLASER_VIEW_CANVAS) {
//                zoomPreviewAroundPoint(centre,0.8);
//            } else if(viewMode == OFXLASER_VIEW_OUTPUT) {
//                //currentLaser.zoomAroundPoint(centre,0.8);
//       
//            }
//        }
//        ImGui::SameLine();
//        if(UI::Button(ICON_FK_ARROWS_ALT, false, false)) {
//            // reset display ;// mouseMode = OFXLASER_MOUSE_ZOOM_OUT;
//            if(viewMode == OFXLASER_VIEW_CANVAS) setDefaultPreviewOffsetAndScale();
//            else if(viewMode == OFXLASER_VIEW_OUTPUT) {
//                setLaserDefaultPreviewOffsetAndScale(getSelectedLaserIndex());
//            }
//        }
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
       
        ImGui::Text("%s",ofToString(ofToString(round(ofGetFrameRate()))).c_str());
        //ImGui::PopFont();
        
    }
    UI::endWindow();
    
}

void Manager :: guiCustomParameters() {
    if(showCustomParametersWindow && (customParams.size()>0)) {
        if(UI::startWindow("Custom Paramenters", ImVec2(ofGetWidth()-guiSpacing -guiLaserSettingsPanelWidth,620), ImVec2(guiLaserSettingsPanelWidth,0),0,false,(bool*)&showCustomParametersWindow.get())) {
            
            UI::addParameterGroup(customParams);
            
            
           
        }
        UI::endWindow();
    }
    
}

void Manager :: guiDacAssignment() {
    
    if(showDacAssignmentWindow) {
        if(UI::startWindow("Controller Assignment", ImVec2(100, 100), ImVec2(550,0), ImGuiWindowFlags_None, false, &showDacAssignmentWindow)) {
            
            
            // get the dacs from the dacAssigner
            const vector<DacData>& dacList = dacAssigner.getDacList();
            
            //        bool useRedButton = areAllLasersArmed();
            //        if(useRedButton) UI::secondaryColourButtonStart();
            //        // change the colour for the arm all button if we're armed
            int buttonwidth = 160;
            //        if(ImGui::Button(useRedButton ? "ALL LASERS ARMED" : "ARM ALL LASERS", ImVec2(buttonwidth, 0.0f) )) {
            //            armAllLasers();
            //        }
            //        if(useRedButton) UI::secondaryColourButtonEnd();
            //
            //        ImGui::SameLine();
            //        if(ImGui::Button("DISARM ALL LASERS",  ImVec2(buttonwidth, 0.0f))) {
            //            disarmAllLasers();
            //        }
            //
            
            
            if(ImGui::Button("RECONNECT ALL",  ImVec2(buttonwidth, 0.0f))){
                for(Laser* laser : lasers) {
                    
                    // if dac is available then disconnect it
                    
                    //string daclabel = laser->getDacLabel();
                    //if(!dacAssigner.assignToLaser(daclabel, *laser)) {
                    //    laser->dacLabel = daclabel;
                    //
                    //}
                    string daclabel = laser->dacLabel;
                    
                    dacAssigner.assignToLaser(daclabel, *laser);
                    
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("DISCONNECT ALL",  ImVec2(buttonwidth, 0.0f))){
                for(Laser* laser : lasers) {
                    
                    // if dac is available then disconnect it
                    
                    //string daclabel = laser->getDacLabel();
                    //if(!dacAssigner.assignToLaser(daclabel, *laser)) {
                    //    laser->dacLabel = daclabel;
                    //
                    //}
                    string daclabel = laser->dacLabel;
                    
                    dacAssigner.disconnectDacFromLaser(*laser);
                    laser->dacLabel = daclabel;
                    
                }
            }
            
            ImGui::Separator();
            
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 330);
            
            for (int n = 0; n < lasers.size(); n++){
                Laser* laser = lasers[n];
                ImGui::PushID(n);
                
                
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 size = ImVec2(19,19); // ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
                
                ImVec2 p = ImGui::GetCursorScreenPos();
                int state = laser->getDacConnectedState();
                
                ImU32 col = ImGui::GetColorU32({0.3,0.3,0.3,1});
                if(laser->hasDac()) col = UI::getColourForState(laser->getDacConnectedState());
                
                draw_list->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), col);
                //draw_list->AddText(p, ImU32(0xffffff), "1");
                // ImGui::Text("%s", "11.45");
                ImGui::InvisibleButton("##gradient2", size - ImVec2(2,2));
                //ImGui::SetCursorPosX(ImGui::GetCursorPosX()+size.x-2);
                ImGui::SameLine();
                
                // ARM BUTTONS
                if(laser->armed) {
                    UI::dangerColourStart();
                }
                string armlabel = "ARM##"+ofToString(n+1);
                if(ImGui::Button(armlabel.c_str())){
                    laser->toggleArmed();
                }
                UI::dangerColourEnd();
                
                ImGui::SameLine();
                
                ImGui::Text("%02d : ", (n+1));
                ImGui::SameLine();
                
                string label;
                if(!laser->hasDac() ) {
                    label = dacAssigner.getAliasForLabel(laser->dacLabel.get());
                    //if(!laser->hasDac()) label = "";
                    //UI::startGhosted();
                    ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_Disabled, ImVec2(130,19) );
                    //UI::stopGhosted();
                    
                } else {
                    
                    label = dacAssigner.getAliasForLabel(laser->getDacLabel());
                    ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_None, ImVec2(130,19) );
                    
                    
                    // only draggable if there is a dac here
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        ImGui::SetDragDropPayload("DAC_ASSIGN", &n, sizeof(int));    // Set payload to carry the index of our item (could be anything)
                        
                        ImGui::Text("Reassign %s", label.c_str());
                        
                        ImGui::EndDragDropSource();
                    }
                }
                
                // but all slots have a drop target
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DAC_ASSIGN"))
                    {
                        IM_ASSERT(payload->DataSize == sizeof(int));
                        int payload_n = *(const int*)payload->Data;
                        
                        // DO THE SWAP
                        //const char* tmp = names[n];
                        //names[n] = names[payload_n];
                        //names[payload_n] = tmp;
                        string dacId;
                        if(payload_n<lasers.size()) {
                            dacId = lasers[payload_n]->dacLabel.get();
                        } else {
                            dacId = dacList[payload_n-lasers.size()].getLabel();
                        }
                        dacAssigner.assignToLaser(dacId, *laser);
                        
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();
                
                //ImVec2 pos = ImGui::GetCursorPos();
                //ImGui::SetCursorPos(ImVec2(200,pos.y));
                if(laser->dacLabel.get()!="") {
                    ImGui::SameLine();
                    string label = ofToString(ICON_FK_TIMES) + "##" + ofToString(n);
                    if(UI::Button(label)){
                        if(laser->hasDac()) {
                            string daclabel = laser->dacLabel;
                            dacAssigner.disconnectDacFromLaser(*laser);
                            laser->dacLabel.set(daclabel);
                        } else {
                            laser->dacLabel = "";
                        }
                    }
                    label = ofToString(ICON_FK_UNDO) + "##" + ofToString(n);
                    ImGui::SameLine();
                    if(UI::Button(label)) {
                        string daclabel = laser->dacLabel;
                        dacAssigner.assignToLaser(daclabel, *laser);
                    }
                    
                    
                }
                
                if(laser->dacLabel.get()!="") {
                    ImGui::SameLine();
                    guiEditDacAliasButtonAndPopup(laser->dacLabel.get());
                }
            }
            
            ImGui::NextColumn();
            ImGui::Text("Available controllers : ");
            ImGui::SameLine();
            if(UI::Button("REFRESH")){
                dacAssigner.updateDacList();
            }
            
            for(int i = 0; i<dacList.size(); i++) {
                const DacData& dacdata = dacList[i];
                if(dacdata.assignedLaser==nullptr) {
                    int id =lasers.size()+i;
                    ImGui::PushID(id);
                    string label = dacAssigner.getAliasForLabel(dacdata.getLabel()); // dacdata.alias; // ->getDacLabel();
                    
                    ImGuiSelectableFlags selectableflags = 0;
                    
                    if(!dacdata.available) {
                        // ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
                        //itemlabel += " - no longer available";
                        selectableflags|=ImGuiSelectableFlags_Disabled;
                    } else {
                        //
                    }
                    
                    ImGui::Selectable(label.c_str(), false, selectableflags, ImVec2(160,19));
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)){
                        // if DAC is double clicked add it to the next empty slot
                        for (int n = 0; n < lasers.size(); n++){
                            Laser* laser = lasers[n];
                            if(!laser->hasDac()) {
                                dacAssigner.assignToLaser(dacdata.getLabel(), *laser);
                                break;
                            }
                            
                        }
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        ImGui::SetDragDropPayload("DAC_ASSIGN", &id, sizeof(int));    // Set payload to carry the index of our item (could be anything)
                        
                        ImGui::Text("Assign %s", label.c_str());
                        
                        ImGui::EndDragDropSource();
                    }
                    ImGui::SameLine();
                    ImGui::PopID();
                    
                    guiEditDacAliasButtonAndPopup(dacdata.getLabel());
                    
                    
                    
                }
                
            }
            
            
            
            
        }
        UI::endWindow();
    }
}

void Manager :: guiEditDacAliasButtonAndPopup(string daclabel) {
    
    string id ="dacalias"+daclabel;
    ImGui::PushID(id.c_str());
    string label;
    static char newDacAlias[255];
    string alias = dacAssigner.getAliasForLabel(daclabel);
    label = "Edit "+alias+" alias";
    if(ImGui::Button(ICON_FK_PENCIL)) {
        strcpy(newDacAlias, alias.c_str());
        ImGui::OpenPopup(label.c_str());
    }
    
    if (ImGui::BeginPopupModal(label.c_str(), 0)){
        
        if(ImGui::InputText("##1", newDacAlias, IM_ARRAYSIZE(newDacAlias))){
           // don't need to do anything here
        }
        
        ImGui::Separator();
        label = "OK## "+daclabel;
        if (ImGui::Button(label.c_str(),  ImVec2(120, 0))) {
            string newalias = newDacAlias;
            // TODO - check existing already
            dacAssigner.addAliasForLabel(newalias,daclabel, true);
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        label = "Cancel## "+daclabel;
        if (ImGui::Button(label.c_str(), ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

bool Manager :: guiDeleteLaserButtonAndPopup(Laser* laser, int index) {
    
    string label ="Delete Laser "+ofToString(index+1)+"?";
    bool deleted = false;
    
    string buttonlabel = ofToString(ICON_FK_MINUS_CIRCLE) + "##" + ofToString(index);
    if (UI::DangerButton(buttonlabel, false)) {
        // delete laser
        if(ofGetKeyPressed(OF_KEY_COMMAND)) {
            deleteLaser(laser);
            deleted = true;
        } else {
            ImGui::OpenPopup(label.c_str());
        }
    }
    
    if (ImGui::BeginPopupModal(label.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to delete this laser? All the zone settings will be deleted.\n\n");
        ImGui::Separator();
        
        UI::dangerColourStart();
        
        if (ImGui::Button("DELETE", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            deleteLaser(laser);
            deleted = true;
        }
        
        UI::dangerColourEnd();
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::EndPopup();
    }
    return deleted;
}

void Manager :: guiCopyLaserSettings() {

    if(!copySettingsWindowOpen) return ;
    
    if(UI::startWindow("Copy laser settings", ImVec2(100, 100), ImVec2(0,0), ImGuiWindowFlags_None, false, &copySettingsWindowOpen )) {
        
        while(lasersToCopyTo.size()<numLasers) {
            lasersToCopyTo.push_back(false);
        }
        if(numLasers>lasersToCopyTo.size()) {
            lasersToCopyTo.resize(numLasers);
        }
        
        
        ImGui::Text("Copy settings from laser %d to : ", selectedLaserIndex+1);
        for(int i = 0; i<lasersToCopyTo.size(); i++) {
            if((i>0) &&(i%8!=0)) ImGui::SameLine();
            
            bool copyactive = (i==selectedLaserIndex) ? false :lasersToCopyTo[i];
            if(i==selectedLaserIndex) {
                UI::startDisabled();
            }
            if(UI::addNumberedCheckBox(i+1, "##" + ofToString(i) + "laserToCopyTo", &copyactive, false)) {
                lasersToCopyTo[i] =  copyactive;
                
            }
            UI::stopDisabled();
            
            if(i%8==7) {
                ImGui::SameLine();
                if(UI::Button("ALL##line"+ofToString(i/8))) {
                    for(int j = 0; j<8 ;j++) {
                        lasersToCopyTo[i-j] = true;
                    }
                }
                ImGui::SameLine();
                if(UI::Button("NONE##line"+ofToString(i/8))) {
                    for(int j = 0; j<8 ;j++) {
                        lasersToCopyTo[i-j] = false;
                    }
                }
            }
        }
        
        if(UI::Button("ALL")) {
            for(bool& copyvalue : lasersToCopyTo) copyvalue = true;
        }
        ImGui::SameLine();
        if(UI::Button("NONE")) {
            for(bool& copyvalue : lasersToCopyTo) copyvalue = false;
        }
        UI::addParameter(copyParams);
        //ImGui::Text("(Shouldn't copy orientation or colour shift)");
        
        UI::dangerColourStart();
        if(UI::Button("COPY SETTINGS")) {
            Laser& sourceLaser = getLaser(getSelectedLaserIndex());
            
            for(int i = 0; i<lasersToCopyTo.size(); i++) {
                
                if((!lasersToCopyTo[i]) || (i==getSelectedLaserIndex())) continue;
                
                Laser& targetLaser = getLaser(i);
                
                if(copyScannerSettings) {
                    targetLaser.speedMultiplier = sourceLaser.speedMultiplier;
                    ofJson scannerjson;
                    sourceLaser.scannerSettings.serialize(scannerjson);
                    targetLaser.scannerSettings.deserialize(scannerjson);
                }
                
                if(copyAdvancedSettings) {
                    ofJson advancedjson;
                    ofSerialize(advancedjson, sourceLaser.advancedParams);
                    ofDeserialize(advancedjson, targetLaser.advancedParams);
                    targetLaser.pps = sourceLaser.pps;
                }
                
                if(copyColourSettings) {
                    ofJson colourjson;
                    ofSerialize(colourjson, sourceLaser.colourSettings.params);
                    ofDeserialize(colourjson, targetLaser.colourSettings.params);
                }
                
                if(copyZonePositions) {
                    
                    vector<OutputZone*> sourcezones = sourceLaser.getSortedOutputZones();
                    vector<OutputZone*> targetzones = targetLaser.getSortedOutputZones();
                    for(int i=0; (i<sourcezones.size()) && (i<targetzones.size()); i++ ){
                        
                        OutputZone* sourcezone = sourcezones[i];
                        OutputZone* targetzone = targetzones[i];
                        int sourcezoneindex = sourcezone->getZoneIndex();
                        int targetzoneindex = targetzone->getZoneIndex();
                        
                        ofJson zonejson;
                        sourcezone->serialize(zonejson);
                        targetzone->deserialize(zonejson);
                        
                        
                        if(sourceLaser.hasAltZone(sourcezoneindex)) {
                            
                            
                            if(!targetLaser.hasAltZone(targetzoneindex)) {
                                targetLaser.addAltZone(targetzoneindex);
                            }
                            OutputZone* sourceAltZone = sourceLaser.getLaserAltZoneForZoneIndex(sourcezone->getZoneIndex());
                            OutputZone* targetAltZone = targetLaser.getLaserAltZoneForZoneIndex(targetzone->getZoneIndex());
                            ofJson zonejson;
                            sourceAltZone->serialize(zonejson);
                            targetAltZone->deserialize(zonejson);
                            
                            
                        } else {
                            // remove alt zone from target
                            targetLaser.removeAltZone(targetzoneindex);
                        }
                        
                        
                        
                    }

                    
                }
                
                
                
            }
            copySettingsWindowOpen = false;
            
        }
        UI::dangerColourEnd();
        UI::endWindow();
    }
}

void Manager::guiDacAnalytics() {
    
    if(!showDacAnalytics) return;
    
    string label;
    
    ofxLaser::Laser* laser = &getLaser(getSelectedLaserIndex());
    
    if(UI::startWindow("Controller Analytics", ImVec2(guiSpacing, ofGetHeight()-guiSpacing-guiSpacing-600), ImVec2(ofGetWidth()-guiSpacing-guiSpacing, 600), ImGuiWindowFlags_None, false, &showDacAnalytics )) {
        
        //UI::extraLargeItemStart();
        
    //        label = "Frame time ";
    //        ImGui::PlotHistogram(label.c_str(), laser->frameTimeHistory, laser->frameTimeHistorySize, laser->frameTimeHistoryOffset, "", 0, 0.1f, ImVec2(0,80));
        
        DacBaseThreaded* dac =  dynamic_cast<DacBaseThreaded*> (laser->getDac());
        if(dac!=nullptr) {
            uint64_t visibledurationmicros = dacSettingsTimeSlice * 1000000; // seconds * million
            uint64_t endTimeMicros = ofGetElapsedTimeMicros();
            uint64_t startTimeMicros = endTimeMicros - visibledurationmicros;
            int numvalues = 1000;
            dac->stateRecorder.recording = true;
            dac->stateRecorder.getLatencyValuesForTime(startTimeMicros, endTimeMicros, numvalues);
            label = "Round trip time";
            ImGui::PlotHistogram(label.c_str(), dac->stateRecorder.values, numvalues, 0, "", 0.0f, 1000.0f, ImVec2(0,80));
            
            dac->stateRecorder.getBufferSizeValuesForTime(startTimeMicros, endTimeMicros, numvalues);
            label = "Buffer ";
            ImGui::PlotHistogram(label.c_str(), dac->stateRecorder.values, numvalues, 0, "", 0.0f, dac->getMaxPointBufferSize(), ImVec2(0,80));
       
            
    //            dac->stateRecorder.getDataRateValuesForTime(startTimeMicros, endTimeMicros, numvalues);
    //            label = "Data rate ";
    //            ImGui::PlotHistogram(label.c_str(), dac->stateRecorder.values, numvalues, 0, "", 0.0f, 5000.0f, ImVec2(0,80));

            dac->frameRecorder.getFrameLatencyValuesForTime(startTimeMicros, endTimeMicros, numvalues);
            label = "Frame latency ";
            ImGui::PlotHistogram(label.c_str(), dac->frameRecorder.values, numvalues, 0, "", 0.0f, 200000.0f, ImVec2(0,80));
            
            dac->frameRecorder.getFrameRepeatValuesForTime(startTimeMicros, endTimeMicros, numvalues);
            label = "Frame repeats ";
            ImGui::PlotHistogram(label.c_str(), dac->frameRecorder.values, numvalues, 0, "",0.0f, 5.0f, ImVec2(0,80));
            
            dac->frameRecorder.getFrameSkipValuesForTime(startTimeMicros, endTimeMicros, numvalues);
            label = "Frame skips ";
            ImGui::PlotHistogram(label.c_str(), dac->frameRecorder.values, numvalues, 0, "",0.0f, 1.0f, ImVec2(0,80));

          //  UI::addIntSlider(dac->pointBufferMinParam);
            UI::addFloatSlider(dacSettingsTimeSlice);
            
           
        }
        
        
       // UI::extraLargeItemEnd();
        UI::endWindow();
    }
    
}

//glm::vec2 Manager ::getPreviewOffset() {
//    return previewOffset;
//}
//float Manager :: getPreviewScale() {
//    return previewScale;
//}
ofRectangle Manager :: getPreviewRect() {
    return canvasViewController.getOutputRect();
    // ofRectangle(previewOffset.x, previewOffset.y, canvasTarget.getWidth()*previewScale, canvasTarget.getHeight()*previewScale);
    
}

ofRectangle Manager :: getZonePreviewRect() {
//    if(laserZoneViews.size()>0) {
//        return laserZoneViews[0].getOutputRect();
//    } else {
//        return ofRectangle(0,0,0,0);
//    }
//
    int w = ofGetWidth()*0.6;
    int h = ofGetHeight()*0.6;
    int size = (MIN(w, h));
    
    return ofRectangle(54,100, size, size);
    
}



void Manager :: drawBigNumber(int number) {
    
    ofPushStyle();
    ofPushMatrix();
    //ofTranslate(200,300);
    //ofScale(20,20);
    ofFill();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetColor(30);
    ofTranslate(getZonePreviewRect().getTopLeft());
    ofTranslate(20,170);
    ofScale(0.1,-0.1);
    if((number+1)>9) {
        iconSVGs.numberSVGs[(number+1)/10].draw(false);
        ofTranslate(1100,0);
    }
    iconSVGs.numberSVGs[(number+1)%10].draw(false);
    ofPopMatrix();
    ofPopStyle();
    
}





//
//void Manager::ShowExampleMenuFile() {
//
//    ImGui::MenuItem("(dummy menu)", NULL, false, false);
//    if (ImGui::MenuItem("New")) {}
//    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
//    if (ImGui::BeginMenu("Open Recent"))
//    {
//        ImGui::MenuItem("fish_hat.c");
//        ImGui::MenuItem("fish_hat.inl");
//        ImGui::MenuItem("fish_hat.h");
//        if (ImGui::BeginMenu("More.."))
//        {
//            ImGui::MenuItem("Hello");
//            ImGui::MenuItem("Sailor");
//            if (ImGui::BeginMenu("Recurse.."))
//            {
//                ShowExampleMenuFile();
//                ImGui::EndMenu();
//            }
//            ImGui::EndMenu();
//        }
//        ImGui::EndMenu();
//    }
//    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
//    if (ImGui::MenuItem("Save As..")) {}
//
//    ImGui::Separator();
//    if (ImGui::BeginMenu("Options"))
//    {
//        static bool enabled = true;
//        ImGui::MenuItem("Enabled", "", &enabled);
//        ImGui::BeginChild("child", ImVec2(0, 60), true);
//        for (int i = 0; i < 10; i++)
//            ImGui::Text("Scrolling Text %d", i);
//        ImGui::EndChild();
//        static float f = 0.5f;
//        static int n = 0;
//        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
//        ImGui::InputFloat("Input", &f, 0.1f);
//        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
//        ImGui::EndMenu();
//    }
//
//    if (ImGui::BeginMenu("Colors"))
//    {
//        float sz = ImGui::GetTextLineHeight();
//        for (int i = 0; i < ImGuiCol_COUNT; i++)
//        {
//            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
//            ImVec2 p = ImGui::GetCursorScreenPos();
//            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x+sz, p.y+sz), ImGui::GetColorU32((ImGuiCol)i));
//            ImGui::Dummy(ImVec2(sz, sz));
//            ImGui::SameLine();
//            ImGui::MenuItem(name);
//        }
//        ImGui::EndMenu();
//    }
//
//    // Here we demonstrate appending again to the "Options" menu (which we already created above)
//    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
//    // In a real code-base using it would make senses to use this feature from very different code locations.
//    if (ImGui::BeginMenu("Options")) // <-- Append!
//    {
//        static bool b = true;
//        ImGui::Checkbox("SomeOption", &b);
//        ImGui::EndMenu();
//    }
//
//    if (ImGui::BeginMenu("Disabled", false)) // Disabled
//    {
//        IM_ASSERT(0);
//    }
//    if (ImGui::MenuItem("Checked", NULL, true)) {}
//    if (ImGui::MenuItem("Quit", "Alt+F4")) {
//        ofExit();
//    }
//
//}

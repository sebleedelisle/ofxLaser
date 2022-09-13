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

        if(zones.size()==0) createDefaultZone();
        for(Laser* laser : lasers) {
            laser->addZone(zones[0],800,800);
            laser->setGrid(zoneGridSnap, zoneGridSize);
        }
        //showScannerSettingsWindow = true;
        
    }
    
//    showDacAssignmentWindow = false;
//    copySettingsWindowOpen = false;
//    showCustomParametersWindow = false;
//    showLaserManagementWindow = false;
    
    selectedLaserIndex = 0;
    viewMode  = OFXLASER_VIEW_CANVAS;
    mouseMode = OFXLASER_MOUSE_DEFAULT;
    guiIsVisible = true;
    guiLaserSettingsPanelWidth = 320;
    guiSpacing = 8;
    draggingPreview = false;
    previewScale = 1;
    
    setDefaultPreviewOffsetAndScale();
    
    params.add(showGuideImage.set("Show guide image", false));
    params.add(guideImageColour.set("Guide image colour", ofColor::white));
    
    dacSettingsTimeSlice.set("Magnification", 0.5, 0.1, 20);
    
    // bit of a hack - ideally UI elements should be completely separate from
    // the ManagerBase but I'm not quite there yet.
    if(!loadJson.empty()) {
        if(loadJson.contains("Laser")) {
            //cout << loadJson["Laser"]["Show_guide_image"].dump(3)<< endl;
            //cout << loadJson["Laser"]["Guide_image_colour"].dump(3)<< endl;
            //cout << showGuideImage << " " <<loadJson["Laser"]["Show_guide_image"]<< endl;
            try {
                ofDeserialize(loadJson["Laser"], showGuideImage);
                ofDeserialize(loadJson["Laser"], guideImageColour);


            } catch(...) {
                //cout << showGuideImage << " " <<loadJson["Laser"]["Show_guide_image"]<< endl;
            }
        }
    }
    
    ofAddListener(ofEvents().mousePressed, this, &Manager::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseReleased, this, &Manager::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseDragged, this, &Manager::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().keyPressed, this, &Manager::keyPressed, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().keyReleased, this, &Manager::keyReleased, OF_EVENT_ORDER_BEFORE_APP);
    initSVGs();
    //visFbo.allocate(1000,600, GL_RGBA, 4);
    
}

Manager::~Manager() {
    ofRemoveListener(ofEvents().mousePressed, this, &Manager::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseReleased, this, &Manager::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseDragged, this, &Manager::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(params.parameterChangedE(), this, &Manager::paramChanged);
    ofRemoveListener(ofEvents().keyPressed, this, &Manager::keyPressed, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().keyReleased, this, &Manager::keyReleased, OF_EVENT_ORDER_BEFORE_APP);
 
    
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
    //params.add(customParams);
    
    // is this still used ?
    params.add(zoneEditorShowLaserPath.set("Show path in zone editor", true));
    params.add(zoneEditorShowLaserPoints.set("Show points in zone editor", false));
    params.add(zoneGridSnap.set("Zone snap to grid", true));
    params.add(zoneGridSize.set("Zone grid size", 20,1,50));
    params.add(globalLatency.set("Latency (ms)", 150,0,400));
   
    
    params.add(showDacAssignmentWindow.set("showDacAssignmentWindow", false));
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
    //copyParams.add(copyZonePositions.set("Copy output zone positions", false));

}
void Manager :: paramChanged(ofAbstractParameter& e) {
    for(Laser* laser : lasers) {
        laser->setGrid(zoneGridSnap, zoneGridSize);
        laser->maxLatencyMS = globalLatency;
    }
    ofLogNotice() << "paramChanged " << e.getName();
    saveSettings();
}


void Manager :: update() {
    // go through all the zones and set their editable status
    // dependent on the current view. Needs a refactor.
    for(size_t i= 0; i<zones.size(); i++) {
        zones[i]->setEditable(showInputPreview && (viewMode==OFXLASER_VIEW_CANVAS) && (!lockInputZones) && showInputZones);
    }
    ManagerBase :: update();
}

bool Manager :: mousePressed(ofMouseEventArgs &e){
    //if(ofGetKeyPressed(' ')) {
   // ofLogNotice("Manager :: mousePressed"); 
    if (viewMode  == OFXLASER_VIEW_CANVAS) {
        if(mouseMode == OFXLASER_MOUSE_ZOOM_IN) { //if(ofGetKeyPressed(OF_KEY_COMMAND)) {
            // zoom in
            zoomPreviewAroundPoint(e,1.2);
            return true;
        } else if(mouseMode == OFXLASER_MOUSE_ZOOM_OUT) { //if(ofGetKeyPressed(OF_KEY_ALT)) {
            // zoom out
            zoomPreviewAroundPoint(e,0.8);
            return true;
       // } else if(ofGetKeyPressed(OF_KEY_CONTROL)) {
       //     setDefaultPreviewOffsetAndScale();
            
        } else if(mouseMode == OFXLASER_MOUSE_DRAG){
            // start dragging
            draggingPreview = true;
            dragStartPoint = e - previewOffset;
            return true;
        }
    } else if (viewMode  == OFXLASER_VIEW_OUTPUT) {
        // do the stuff but for the individual laser
        ofxLaser::Laser& currentLaser = *lasers[getSelectedLaserIndex()];
        if(mouseMode == OFXLASER_MOUSE_ZOOM_IN) { //if(ofGetKeyPressed(OF_KEY_COMMAND)) {
            // zoom in
            currentLaser.zoomAroundPoint(e,1.2);
            return true;
        } else if(mouseMode == OFXLASER_MOUSE_ZOOM_OUT) { //} else if(ofGetKeyPressed(OF_KEY_ALT)) {
            // zoom out
            currentLaser.zoomAroundPoint(e,0.8);
            return true;
        //} else if(ofGetKeyPressed(OF_KEY_CONTROL)) {
        //    currentLaser.setOffsetAndScale(glm::vec2(guiSpacing, guiSpacing), 1);
            
        } else if(mouseMode == OFXLASER_MOUSE_DRAG) { //} else {
            // start dragging
            currentLaser.startDrag(e); //  - previewOffset;
            return true;
        }
    } else if(viewMode == OFXLASER_VIEW_3D) {
        visualiser3D.mousePressed(e);
        
    }
    
    return false;
}

void Manager :: setDefaultPreviewOffsetAndScale(){
    previewOffset = glm::vec2(guiSpacing, guiSpacing+iconBarHeight+menuBarHeight);
    previewScale = 1;
    float thirdOfHeight = (ofGetHeight()/3 )- previewOffset.y;
    
    if(height>(thirdOfHeight*2)) {
        previewScale = (float)(thirdOfHeight*2) / (float)height;
    }
    if(width * previewScale> ofGetWidth()-(guiSpacing*3)-guiLaserSettingsPanelWidth) {
        previewScale = (float)(ofGetWidth()-(guiSpacing*3)-guiLaserSettingsPanelWidth)/width;
    }
    
}

bool Manager :: mouseReleased(ofMouseEventArgs &e){
    draggingPreview = false;
    for(ofxLaser::Laser* laser : lasers) {
        laser->stopDrag();
        
    }
    visualiser3D.mouseReleased(e);
    return false;
}
bool Manager :: mouseDragged(ofMouseEventArgs &e){
    visualiser3D.mouseDragged(e);
    if(draggingPreview) {
        previewOffset = e-dragStartPoint;
        return true;
    } else {
        return false;
    }
}
bool Manager :: keyPressed(ofKeyEventArgs &e) {

    if(ofGetKeyPressed(' ')) {
        if(ofGetKeyPressed(OF_KEY_COMMAND))  {
            mouseMode = OFXLASER_MOUSE_ZOOM_IN;
        } else if(ofGetKeyPressed(OF_KEY_ALT)) {
            mouseMode = OFXLASER_MOUSE_ZOOM_OUT;
        } else {
            mouseMode = OFXLASER_MOUSE_DRAG;
        }
    }
    // false means we keep the event bubbling
    return false;
}
       
bool Manager :: keyReleased(ofKeyEventArgs &e){
    if(e.key == ' ' ) {
        
        mouseMode = OFXLASER_MOUSE_DEFAULT;
        
    }
    //false means we keep the event bubbling
    return false;
}


void Manager :: zoomPreviewAroundPoint(glm::vec2 anchor, float zoomMultiplier) {
    glm::vec2 offset = anchor-previewOffset;
    offset-=(offset*zoomMultiplier);
    previewOffset+=offset;
    previewScale*=zoomMultiplier;
    
}

bool Manager :: deleteLaser(Laser* laser) {
    bool success = ManagerBase::deleteLaser(laser);
    if(success) {
        if(selectedLaserIndex>=getNumLasers()) selectedLaserIndex = getNumLasers()-1;
        if(getNumLasers()==0) {
            showLaserOutputSettingsWindow = false;
            viewMode = OFXLASER_VIEW_CANVAS;
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
    
    if(mouseMode!=OFXLASER_MOUSE_DEFAULT) {
        ofHideCursor(); // TODO add so that it only happens over the preview
        ofPushMatrix();
        ofTranslate(ofGetMouseX(), ofGetMouseY());
        ofScale(0.6);
        ofTranslate(-12,-12);
        // ofEnableDepthTest();
        if(mouseMode == OFXLASER_MOUSE_ZOOM_IN) { // ofGetKeyPressed(OF_KEY_COMMAND)) {
            iconMagPlus.draw();
        } else if(mouseMode == OFXLASER_MOUSE_ZOOM_OUT) { // if(ofGetKeyPressed(OF_KEY_ALT)) {
            iconMagMinus.draw();
        } else if(mouseMode == OFXLASER_MOUSE_DRAG) { // } else if(ofGetMousePressed()){
            if(ofGetMousePressed()) iconGrabClosed.draw();
            else iconGrabOpen.draw();
        }
        // ofDisableDepthTest();
        ofPopMatrix();
    } else ofShowCursor();
    
}

void Manager :: drawPreviews() {
    
    if(viewMode == OFXLASER_VIEW_3D) {
        float previewheight = (ofGetHeight()/2)-30;
        ofRectangle rect3D(10,30,previewheight/9*16, previewheight); // 16/9 ratio
        visualiser3D.draw(rect3D, lasers, mouseMode == OFXLASER_MOUSE_DRAG);
        
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
//            if(spaceatbottom>(outputpreviewsize*2)+guiSpacing) {
//                numrows=2;
//                outputpreviewsize = (spaceatbottom/2)-guiSpacing;
//            }
            
            for(size_t i= 0; i<lasers.size(); i++) {
                
                ofRectangle laserOutputPreviewRect(guiSpacing+((outputpreviewsize+guiSpacing)*i),ofGetHeight()-guiSpacing-outputpreviewsize,outputpreviewsize,outputpreviewsize);
               
//                if(numrows>1) {
//                    int numinrow = lasers.size()/numrows;
//                    int rownum = floor(i/numinrow);
//                    laserOutputPreviewRect.y-=(rownum*(outputpreviewsize+guiSpacing));
//                    laserOutputPreviewRect.x =guiSpacing+ ((outputpreviewsize+guiSpacing)*(i%numinrow));
//
//                }
                
                ofFill();
                ofSetColor(0);
                ofDrawRectangle(laserOutputPreviewRect);
                
                lasers[i]->drawTransformAndPath(laserOutputPreviewRect);
                
                // disables the warp interfaces
                lasers[i]->disableTransformGui();
            }
            
           
            
        }
       
        
    } else if(viewMode == OFXLASER_VIEW_CANVAS) {
        if(showInputPreview) {
        
        
            ofPushStyle();
            ofFill();
            ofSetColor(0);
            ofPushMatrix();
            ofTranslate(previewOffset);
            ofScale(previewScale);
            ofDrawRectangle(0,0,width, height);
            ofPopMatrix();
            ofPopStyle();
        

            if(showInputZones) {
                // this renders the input zones in the graphics source space
                for(size_t i= 0; i<zones.size(); i++) {
                    zones[i]->setOffsetAndScale(previewOffset,previewScale);
                   // zones[i]->setEditable(!lockInputZones);
                    zones[i]->setVisible(true);
                    zones[i]->draw();
                }
            } else { 
                // this renders the input zones in the graphics source space
                for(size_t i= 0; i<zones.size(); i++) {
                    
                    zones[i]->setVisible(false);
                   
                }
            }
            
            renderPreview();
            
            if(showBitmapMask) {
                ofPushMatrix();
                laserMask.setOffsetAndScale(previewOffset,previewScale);
                laserMask.draw(showBitmapMask);
                ofTranslate(previewOffset);
                ofScale(previewScale, previewScale);
                ofPopMatrix();
            }
        }
        
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
//            if(spaceatbottom>(outputpreviewsize*2)+guiSpacing) {
//                numrows=2;
//                outputpreviewsize = (spaceatbottom/2)-guiSpacing;
//            }
            
            for(size_t i= 0; i<lasers.size(); i++) {
                
                ofRectangle laserOutputPreviewRect(guiSpacing+((outputpreviewsize+guiSpacing)*i),ofGetHeight()-guiSpacing-outputpreviewsize,outputpreviewsize,outputpreviewsize);
               
//                if(numrows>1) {
//                    int numinrow = lasers.size()/numrows;
//                    int rownum = floor(i/numinrow);
//                    laserOutputPreviewRect.y-=(rownum*(outputpreviewsize+guiSpacing));
//                    laserOutputPreviewRect.x =guiSpacing+ ((outputpreviewsize+guiSpacing)*(i%numinrow));
//
//                }
                
                ofFill();
                ofSetColor(0);
                ofDrawRectangle(laserOutputPreviewRect);
                
                lasers[i]->drawTransformAndPath(laserOutputPreviewRect);
                
                // disables the warp interfaces
                lasers[i]->disableTransformGui();
            }
            
           
            
        }
        
       
        
    }
    
    if(viewMode == OFXLASER_VIEW_OUTPUT){
        
        // hide the input zones
        for(size_t i= 0; i<zones.size(); i++) {
                
            zones[i]->setVisible(false);
               
        }
        
        
        ofxLaser::Laser* selectedlaser = nullptr;
        // draw laser zone adjustment screen
        for(size_t i= 0; i<lasers.size(); i++) {
            if((int)i==selectedLaserIndex) {
                selectedlaser = lasers[i];
               
                selectedlaser->enableTransformGui();
                selectedlaser->drawTransformUI();
                if(zoneEditorShowLaserPath) {
                    // if the laser is paused then show the movement of the laser
                    selectedlaser->drawLaserPath(zoneEditorShowLaserPoints, selectedlaser->paused);
                }
                ofPushStyle();
                ofPushMatrix();
                //ofTranslate(200,300);
                //ofScale(20,20);
                ofFill();
                ofEnableBlendMode(OF_BLENDMODE_ADD);
                ofSetColor(30);
                //string label = ofToString(i+1);
                //ofRectangle rect = numberFont.getStringBoundingBox(label, 0, 0);
                //numberFont.drawStringAsShapes(label, 20, rect.getHeight()+50);
                
                ofTranslate(40,250);
                ofScale(0.12,-0.12);
                if((i+1)>9) {
                    numberSVGs[(i+1)/10].draw(false);
                    ofTranslate(1100,0);
                }
                numberSVGs[(i+1)%10].draw(false);
                ofPopMatrix();
                ofPopStyle();
            } else {
                lasers[i]->disableTransformGui();
            }
            
        }
        
        
        
        
        if(showOutputPreviews) {
            
            
            for(size_t i= 0; i<lasers.size(); i++) {
                float outputpreviewscale = 0.375;
                
                float outputpreviewsize = 800*outputpreviewscale;
                float previewbotton = (selectedlaser->previewScale*800)+selectedlaser->previewOffset.y;
                float spaceatbottom = (ofGetHeight() - previewbotton) -(guiSpacing*2);
                if (spaceatbottom<50) spaceatbottom = 50;
                if(outputpreviewsize>spaceatbottom) outputpreviewsize = spaceatbottom;
                ofRectangle laserOutputPreviewRect(guiSpacing+((outputpreviewsize+guiSpacing)*i),ofGetHeight()-guiSpacing-outputpreviewsize,outputpreviewsize,outputpreviewsize);
                
                ofFill();
                ofSetColor(0);
                ofDrawRectangle(laserOutputPreviewRect);
                
                lasers[i]->drawTransformAndPath(laserOutputPreviewRect);
                if((int)i==selectedLaserIndex) {
                    ofNoFill();
                    ofSetLineWidth(1);
                    ofSetColor(255);
                    ofDrawRectangle(laserOutputPreviewRect);
                    
                }
            }
            
        }
        
        
    } else {
        for(Laser* laser : lasers) {
            laser->disableTransformGui();
        }
        
    }
    
    
}
void Manager :: renderPreview() {
    
    //
    //    ofPushStyle();
    //    ofFill();
    //    ofSetColor(0);
    //    ofPushMatrix();
    //    ofTranslate(previewOffset);
    //    ofScale(previewScale);
    //    ofDrawRectangle(0,0,width, height);
    //    ofPopMatrix();
    //    ofPopStyle();
    //
    
    ofRectangle previewRect = getPreviewRect();
    ofRectangle screenRect(0,0,ofGetWidth(), ofGetHeight());
    ofRectangle visibleRect = previewRect.getIntersection(screenRect);
    
    int fbowidth = visibleRect.getWidth();
    int fboheight = visibleRect.getHeight();
    
    if((canvasPreviewFbo.getWidth()!=fbowidth) || (canvasPreviewFbo.getHeight()!=fboheight)) {
        // previewFbo.clear();
        canvasPreviewFbo.allocate(fbowidth, fboheight, GL_RGB, 3);
    }
    
    canvasPreviewFbo.begin();
    
    ofClear(0,0,0,0);
    ofPushStyle();
    // ofEnableSmoothing();
    ofPushMatrix();
    ofTranslate(previewRect.getTopLeft()-visibleRect.getTopLeft());
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
    
    
    if(showGuideImage && guideImage.isAllocated()) {
        ofSetColor(guideImageColour);
        guideImage.draw(0,0,width, height);
        
        
    }
    ofDisableBlendMode();
    // ofDisableSmoothing();
    
    ofPopMatrix();
    
    ofPopStyle();
    canvasPreviewFbo.end();
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    canvasPreviewFbo.draw(visibleRect.getTopLeft());
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
        if(!loadJson.empty()) {
            if(loadJson.contains("Laser")) {
                if(loadJson["Laser"].contains("CUSTOM_PARAMETERS")) {
                    //     auto value = loadJson["Laser"]["Custom"][param.getName()];
                    try {
                        ofDeserialize(loadJson["Laser"]["CUSTOM_PARAMETERS"], param);
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
    
    glm::vec2 returnpos= pos - previewOffset;
    returnpos/=previewScale;
    return returnpos;
    
}

void Manager::ShowExampleMenuFile() {
    
    ImGui::MenuItem("(dummy menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x+sz, p.y+sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {
        ofExit();
    }
    
}

void Manager::drawLaserGui() {
    
    //ImGui::ShowDemoWindow();
    
    
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
        if(ImGui::BeginMenu("Window") ) {
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
            
//            if (ImGui::MenuItem("Controller assingment", "CMD+D")) {}

            ImGui::EndMenu();
        }
        menuBarHeight = ImGui::GetWindowHeight();
        ImGui::EndMainMenuBar();
    }
    ImGui::PopFont();
    ImGui::PopStyleVar();
    
    drawUIPanelTopBar(menuBarHeight-1);
    drawUIPanelMainLasers();
    drawUIPanelLaserOutputSettings(&getLaser(getSelectedLaserIndex()));
    drawUIPanelCustomParameters();
//
//    int mainpanelwidth = 270;
//    int laserpanelwidth = 280;
//    int spacing = 8;
////
//
//
//    UI::addParameterGroup(laserManager.interfaceParams);
//
//    if(guideImage.isAllocated()) {
//        UI::addParameter(showGuideImage);
//        UI::addParameter(guideImageColour);
//    }
//
//    if(!lockInputZones) {
//
//        ImGui::Separator();
//
//        for(InputZone* zone : zones) {
//
//            string buttonlabel ="DELETE "+zone->zoneLabel;
//            string modallabel ="Delete "+zone->zoneLabel+"?";
//
//            UI::secondaryColourButtonStart();
//            if(ImGui::Button(buttonlabel.c_str())) {
//                ImGui::OpenPopup(modallabel.c_str());
//            }
//            UI::secondaryColourButtonEnd();
//
//            if (ImGui::BeginPopupModal(modallabel.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
//            {
//                ImGui::Text("Are you sure? All the zone settings will be deleted.\n\n");
//                ImGui::Separator();
//
//                UI::secondaryColourButtonStart();
//                if (ImGui::Button("DELETE", ImVec2(120, 0))) {
//                    ImGui::CloseCurrentPopup();
//                    deleteZone(zone);
//
//                }
//                UI::secondaryColourButtonEnd();
//
//                ImGui::SetItemDefaultFocus();
//                ImGui::SameLine();
//                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
//                    ImGui::CloseCurrentPopup();
//
//                }
//                ImGui::EndPopup();
//            }
//
//
//
//
//
//        }
//    }
//
//
//    if(laserManager.customParams.size()>0) {
//        ImGui::Separator();
//        //ImGui::Text("CUSTOM PARAMETERS");
//        UI::addParameterGroup(laserManager.customParams);
//
//    }
//
    
    
//    ImGui::PopItemWidth();
    
 //   UI::endWindow();
    // ImGui::End();
    
    
    // show laser settings :
   
    ofxLaser::Manager& laserManager = *this;
   
   
    
    // Show laser zone settings mute / solo / etc
    if(viewMode == OFXLASER_VIEW_OUTPUT)  {
        
        
        // LASER ZONE SETTINGS
        Laser* laser = lasers[selectedLaserIndex];
        vector<OutputZone*> activeZones = laser->getActiveZones();
        
        glm::vec2 laserZonePos = previewOffset + (previewScale*glm::vec2(width, 0));
        
        UI::startWindow("Laser output zones", ImVec2(800+guiSpacing, guiSpacing+menuBarHeight), ImVec2(380,500));
        
        ImGui::PushFont(UI::largeFont);
        ImGui::Text("%s", laser->getLabel().c_str());
        ImGui::PopFont();
      
        ImGuiTreeNodeFlags collapsingheaderflags = ImGuiTreeNodeFlags_None;
        if(activeZones.size()==0) collapsingheaderflags|=ImGuiTreeNodeFlags_DefaultOpen;
        
        if (ImGui::CollapsingHeader("Add / remove zones and masks",collapsingheaderflags)) {
                ImGui::Columns(3, "Laser zones columns");

            // ADD / REMOVE ZONES
            ImGui::Text("Zones");
            for(InputZone* zone : zones) {
                bool checked = laser->hasZone(zone);

                if(ImGui::Checkbox(zone->displayLabel.c_str(), &checked)) {
                    if(checked) {
                        laser->addZone(zone, width, height);
                    } else {
                        laser->removeZone(zone);
                    }
                }
            }
            ImGui::NextColumn();
            ImGui::Text("Alternate zones");
            for(InputZone* zone : zones) {
                bool checked = laser->hasAltZone(zone);
                string label = zone->displayLabel + "##alt";
                if(ImGui::Checkbox(label.c_str(), &checked)) {
                    if(checked) {
                        laser->addAltZone(zone, width, height);
                    } else {
                        laser->removeAltZone(zone);
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
        
        UI::addIntSlider(laser->testPattern);
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
        
        // Laser Output Masks
        
        for(OutputZone* laserZone : laser->outputZones) {
            
            if(laserZone->getSelected()) {
                
                
                //ImVec2 pos(laserZone->zoneTransform.getRight(),laserZone->zoneTransform.getCentre().y);
                //ImVec2 size(200,0);
                //UI::startWindow(laserZone->getLabel()+"##"+laser->getLabel(),pos, size, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
                
                ImGui::Separator();
                ImGui::PushFont(UI::largeFont);
                ImGui::Text("Zone %d", laserZone->getZoneIndex()+1);
                ImGui::PopFont();
                
                if(laserZone->isSquare()) {
                    UI::startGhosted();
                }
                if(UI::Button("Reset to square")) {
                    laserZone->resetToSquare();
                    laserZone->isDirty = true;
                }
                UI::stopGhosted();
                UI::toolTip("Removes any distortion in the zone and makes all the corners right angles");
               
                
                UI::addParameterGroup(laserZone->transformParams, false);
                ImGui::Text("Edge masks");
                UI::addFloatSlider(laserZone->bottomEdge);
                UI::addFloatSlider(laserZone->topEdge);
                UI::addFloatSlider(laserZone->leftEdge);
                UI::addFloatSlider(laserZone->rightEdge);
               
            }
        }
        UI::endWindow();
        
    }
    drawUIPanelDacAssigner();
    
    visualiser3D.drawUI();

    drawUIPanelLaserCopySettings();
    
}


void Manager :: drawUIPanelMainLasers() {
    
    if(!showLaserManagementWindow) return;
    ofxLaser::Manager& laserManager = *this;
    

    
    // calculate x position of main window
    int x = ofGetWidth() - guiLaserSettingsPanelWidth - guiSpacing;

    if(UI::startWindow("Laser overview", ImVec2(x, guiSpacing+menuBarHeight), ImVec2(guiLaserSettingsPanelWidth, 0),0,false, (bool*)&showLaserManagementWindow.get())){
        
        float buttonwidth = (guiLaserSettingsPanelWidth-(guiSpacing*3))/2;
        
    //
    //    // START BIG BUTTONS
    //    UI::largeItemStart();
    //
    //    // the arm and disarm buttons
    //    bool useRedButton =laserManager.areAllLasersArmed();
    //    if(useRedButton) UI::secondaryColourButtonStart();
    //    // change the colour for the arm all button if we're armed
    //    float buttonwidth = (mainpanelwidth-(spacing*3))/2;
    //    if(ImGui::Button(useRedButton ? "ALL LASERS ARMED" : "ARM ALL LASERS", ImVec2(buttonwidth, 0.0f) )) {
    //        laserManager.armAllLasers();
    //    }
    //    if(useRedButton) UI::secondaryColourButtonEnd();
    //
    //    ImGui::SameLine();
    //    if(ImGui::Button("DISARM ALL LASERS",  ImVec2(buttonwidth, 0.0f))) {
    //        laserManager.disarmAllLasers();
    //    }
    //
    //    // change width of slider vs label
    //    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 120.0f);
    //
    //    // add intensity slide
    //    //UI::addFloatAsIntSlider(laser.masterIntensity, 100);
    //    ImGui::PushItemWidth(mainpanelwidth-(spacing*2));
    //    float multiplier = 100;
    //    int value = laserManager.globalBrightness*multiplier;
    //    if (ImGui::SliderInt("##int", &value, laserManager.globalBrightness.getMin()*multiplier, laserManager.globalBrightness.getMax()*multiplier, "BRIGHTNESS %d")) {
    //        laserManager.globalBrightness.set((float)value/multiplier);
    //
    //    }
    //    ImGui::PopItemWidth();
    //    UI::largeItemEnd();
      
        UI::addIntSlider(laserManager.testPattern);
        
        
       // UI::largeItemStart();
        
    //    if(ImGui::Button("LASER SETTINGS", ImVec2(buttonwidth, 0.0f))) {
    //        showLaserSettings = true;
    //    }
    //    ImGui::SameLine();
    //    if(ImGui::Button("CONTROLLERS", ImVec2(buttonwidth, 0.0f))) {
    //        dacAssignmentWindowOpen = true;
    //    }
    //
    //    if(ImGui::Button("ADD LASER", ImVec2(buttonwidth, 0.0f))) {
    //        createAndAddLaser();
    //        saveSettings();
    //    }
    //    ImGui::SameLine();
    //    if(ImGui::Button("ADD ZONE", ImVec2(buttonwidth, 0.0f))) {
    //        addZone();
    //        lockInputZones = false;
    //        saveSettings();
    //    }
    //
    //    bool useRedButton =laserManager.areAllLasersUsingAlternateZones();
    //    if(useRedButton) UI::secondaryColourButtonStart();
    //    // change the colour for the arm all button if we're armed
    //
    //    if(ImGui::Button("USE ALTERNATE ZONES") ) {
    //        laserManager.useAltZones = !laserManager.useAltZones;
    //    }
    //    if(useRedButton) UI::secondaryColourButtonEnd();
    //
    //
        // END BIG BUTTONS
        //UI::largeItemEnd();
        
        // SHOW LIST OF LASERS
        
        for(int i = 0; i<getNumLasers(); i++) {
            
            string label;
            
            ofxLaser::Laser& laserobject = laserManager.getLaser(i);
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
            if(laserobject.armed) {
                UI::dangerColourStart();
    
            }
            string armlabel = "ARM##"+ofToString(i+1);
            if(ImGui::Button(armlabel.c_str())){
                laserobject.toggleArmed();
            }
            UI::dangerColourEnd();
            
            ImGui::SameLine();
            
            // LASER SETTINGS BUTTONS
            label = ofToString(ICON_FK_BARS)+"##"+ofToString(i);
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
             
            if(laserobject.paused && (((ofGetElapsedTimeMillis()-(i*15))%600)<300)) UI::startGhosted();
            label = ofToString(ICON_FK_PAUSE)+"##"+ofToString(i);
            if(UI::Button( label, false, laserobject.paused)) {
                laserobject.paused = !laserobject.paused;
            }
            UI::stopGhosted();
            
            // FRAME RATES
            
            ImGui::SameLine();
            label = "##framerate"+laserNumberString;
            ImGui::PushItemWidth(60);
            ImGui::PlotHistogram(label.c_str(), laserobject.frameTimeHistory, laserobject.frameTimeHistorySize, laserobject.frameTimeHistoryOffset, "", 0, 0.1f);
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
            ImU32 col = UI::getColourForState(laserobject.getDacConnectedState());
            
            //draw_list->AddCircleFilled(p,radius, col);
            //ImGui::InvisibleButton("##invisible", ImVec2(radius*2, radius*2) - ImVec2(2,2));
            
            draw_list->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), col);
            ImGui::InvisibleButton("##gradient2", size - ImVec2(0,2));
            ImGui::SameLine();
            label = ofToString(ICON_FK_MINUS_CIRCLE)+"##"+ofToString(i);
            if(UI::DangerButton( label, false)) {
                // delete laser
                string label ="Delete Laser?##"+ofToString(i);
                ImGui::OpenPopup(label.c_str());
            }
                
            drawUIPopupDeleteLaser(&laserobject, i);
            
            float framerate = laserobject.getFrameRate();
            if(framerate!=INFINITY) {
                ImGui::SameLine();
                label = ofToString(round(framerate));
                ImGui::Text("%s",label.c_str());
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
            addZone();
            lockInputZones = false;
            saveSettings();
        }
    
        if(UI::Button("CONTROLLERS", false, false)) {
            // add laser
            showDacAssignmentWindow = true;
            ImGui::SetWindowFocus("Controller Assignment");
            
        }
        UI::addIntSlider(globalLatency);
        
        
        UI::endWindow();
    }
   
}

void Manager :: drawUIPanelLaserOutputSettings(ofxLaser::Laser* laser) {
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
        
        
        
        UI::largeItemStart();
        // change width of slider vs label
        ImGui::PushItemWidth(140);
        //UI::secondaryColourButtonStart();
       // UI::addNumberedCheckbox(laser->laserIndex+1, laser->armed);
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
        
        UI::addIntSlider(laser->testPattern);
        //UI::addButton(resetDac);
        
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
        
        // FINE OUTPUT SETTINGS
        bool treevisible = ImGui::TreeNode("Fine position adjustments");
        
        UI::toolTip("These affect all output zones for this laser and can be used to re-align the output if moved slightly since setting them up");
        if (treevisible){
            
            
            ofParameter<float>& param = laser->rotation;
            if(ImGui::DragFloat("Rotation", (float*)&param.get(), 0.01f,-10,10)) { //  param.getMin(), param.getMax())) {
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
        
        UI::addCheckbox(laser->flipX);
        ImGui::SameLine();
        UI::addCheckbox(laser->flipY);
//
        // TODO put this in a menu item
//        if(ImGui::Button("COPY LASER SETTINGS")) {
//            copySettingsWindowOpen = !copySettingsWindowOpen;
//        }
//
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
        
        //    ImGui::SetNextWindowPos,
        ImGui::SetNextWindowSize({760,0});
        // centre popup
        
        ImGui::SetNextWindowPos({(float)ofGetWidth()/2, (float)ofGetHeight()/2}, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        
        // SCANNER PRESET POPUP
        
        if (showEditScannerPreset && ImGui::Begin("Edit Scanner Preset", &showEditScannerPreset, ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoDocking))
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
        
        
        
        
        
        
        
        UI::endWindow();
        
    }
    
    
        
        
        
    
    
}


void Manager :: drawUIPanelTopBar(int ypos) {

    ofxLaser::Manager& laserManager = *this;
    
    UI::startWindow("Icon bar", ImVec2(0,ypos), ImVec2(800,iconBarHeight),ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize, true, nullptr );

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
    
    
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
   //ImGui::PushFont(UI::symbolFont);
    if(UI::Button(ICON_FK_MOUSE_POINTER, false, mouseMode==OFXLASER_MOUSE_DEFAULT)) {
        mouseMode = OFXLASER_MOUSE_DEFAULT;
    }
    ImGui::SameLine();
    if(UI::Button(ICON_FK_HAND_PAPER_O, false, mouseMode==OFXLASER_MOUSE_DRAG)) {
        mouseMode = OFXLASER_MOUSE_DRAG;
    }ImGui::SameLine();

    glm::vec2 centre = glm::vec2(width/2, height/2);
    ofxLaser::Laser& currentLaser = *lasers[getSelectedLaserIndex()];
    
    if(UI::Button(ICON_FK_PLUS, false, false)) {
        if(viewMode == OFXLASER_VIEW_CANVAS) {
            zoomPreviewAroundPoint(centre,1.2);
        } else if(viewMode == OFXLASER_VIEW_OUTPUT) {
            currentLaser.zoomAroundPoint(centre,1.2);
   
        }
    }
    ImGui::SameLine();
    if(UI::Button(ICON_FK_MINUS, false, false)) {
        if(viewMode == OFXLASER_VIEW_CANVAS) {
            zoomPreviewAroundPoint(centre,0.8);
        } else if(viewMode == OFXLASER_VIEW_OUTPUT) {
            currentLaser.zoomAroundPoint(centre,0.8);
   
        }
    }
    ImGui::SameLine();
    if(UI::Button(ICON_FK_ARROWS_ALT, false, false)) {
        // reset display ;// mouseMode = OFXLASER_MOUSE_ZOOM_OUT;
        if(viewMode == OFXLASER_VIEW_CANVAS) setDefaultPreviewOffsetAndScale();
        else if(viewMode == OFXLASER_VIEW_OUTPUT) {
            
            currentLaser.setOffsetAndScale(glm::vec2(guiSpacing, guiSpacing), 1);
        }
    }
    
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX()+10);
   
    ImGui::Text("%s",ofToString(ofToString(round(ofGetFrameRate()))).c_str());
    //ImGui::PopFont();
    UI::endWindow();
    
    
    
}

void Manager :: drawUIPanelCustomParameters() {
    if(showCustomParametersWindow && (customParams.size()>0)) {
        UI::startWindow("Custom Paramenters", ImVec2(ofGetWidth()-guiSpacing -guiLaserSettingsPanelWidth,620), ImVec2(guiLaserSettingsPanelWidth,0),0,false,(bool*)&showCustomParametersWindow.get());
            
        UI::addParameterGroup(customParams);
    
         
        UI::endWindow();
    }
    
}

void Manager :: drawUIPanelDacAssigner() {
    
    if(showDacAssignmentWindow) {
        UI::startWindow("Controller Assignment", ImVec2(100, 100), ImVec2(550,0), ImGuiWindowFlags_None, false, (bool*)&showDacAssignmentWindow.get());


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
                showDacAliasEditButton(laser->dacLabel.get());
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
                
                showDacAliasEditButton(dacdata.getLabel());
                
                
               
            }
            
        }
        
        
        
        UI::endWindow();
    }
}

void Manager :: showDacAliasEditButton(string daclabel) {
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
//
//void Manager :: drawUIPanelScannerSettings(ofxLaser::Laser* laser, float laserpanelwidth, float spacing, float x) {
//   //ImGui::ShowDemoWindow();
//    UI::startWindow("Scanner settings", ImVec2(x, spacing), ImVec2(laserpanelwidth,0), ImGuiWindowFlags_AlwaysAutoResize, false, (bool*)&showScannerSettingsWindow.get());
//
//
//    if(ImGui::Button("COPY LASER SETTINGS")) {
//        copySettingsWindowOpen = !copySettingsWindowOpen;
//    }
//
//
//    UI::largeItemStart();
//    ImGui::PushItemWidth(190);
//    UI::addFloatAsIntPercentage(laser->speedMultiplier);
//    UI::toolTip("Scanner speed adjustment (NB this works mathematically, it doesn't change the point rate)");
//    ImGui::PopItemWidth();
//    ImGui::PushItemWidth(170);
//    UI::addFloatSlider(laser->colourChangeShift);
//    UI::toolTip("Shifts the laser colours to match the scanner position (AKA blank shift)");
//    ImGui::PopItemWidth();
//    UI::largeItemEnd();
//
//
//
//   // PresetManager& presetManager = *PresetManager::instance();
//    // TODO :
//    // check if the settings are different from the preset, if they are
//    // show a "save" button, also save as?
//    //
//    // when an option is selected, update all the params
//
//    ScannerSettings& currentPreset = *scannerPresetManager.getPreset(laser->scannerSettings.getLabel());
//    scannerPresetManager.drawComboBox(laser->scannerSettings);
//
//    ImGui::SameLine();
//
//    if(ImGui::Button("EDIT")) {
//        //ImGui::OpenPopup("Edit Scanner Preset");
//        showEditScannerPreset = true;
//    }
//
//    //    ImGui::SetNextWindowPos,
//    ImGui::SetNextWindowSize({760,0});
//    // centre popup
//
//    ImGui::SetNextWindowPos({(float)ofGetWidth()/2, (float)ofGetHeight()/2}, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
//
//    // SCANNER PRESET POPUP
//
//    if (showEditScannerPreset && ImGui::Begin("Edit Scanner Preset", &showEditScannerPreset, ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoDocking))
//    {
//        ImGui::Text("SCANNER SETTINGS - %s",laser->getLabel().c_str());
//        ImGui::Separator();
//        scannerPresetManager.drawComboBox(laser->scannerSettings);
//
//
//        ImGui::SameLine();
//        scannerPresetManager.drawSaveButtons(laser->scannerSettings);
//
//        UI::addResettableFloatSlider(laser->scannerSettings.moveSpeed, currentPreset.moveSpeed,"How quickly the mirrors move between shapes", "%.1f", 3.0f);
//
//        ImGui::Columns(2);
//        UI::addResettableIntSlider(laser->scannerSettings.shapePreBlank, currentPreset.shapePreBlank, "The length of time that the laser is switched off and held at the beginning of a shape");
//
//        UI::addResettableIntSlider(laser->scannerSettings.shapePreOn, currentPreset.shapePreOn, "The length of time that the laser is switched on and held at the beginning of a shape");
//        ImGui::NextColumn();
//
//        UI::addResettableIntSlider(laser->scannerSettings.shapePostBlank, currentPreset.shapePostBlank,"The length of time that the laser is switched off and held at the end of a shape" );
//        UI::addResettableIntSlider(laser->scannerSettings.shapePostOn, currentPreset.shapePostOn,"The length of time that the laser is switched on and held at the end of a shape" );
//
//        ImGui::Columns(1);
//
//        ImGui::Text("Render profiles");
//        UI::toolTip("Every scanner setting has three profiles for rendering different qualities of laser effects. Unless otherwise specified, the default profile is used. The fast setting is good for long curvy lines, the high detail setting is good for complex pointy shapes.");
//
//        ImGui::Separator();
//        //bool firsttreeopen = true;
//        ImGui::Columns(3);
//        ImGui::SetColumnWidth(0, 250);
//        ImGui::SetColumnWidth(1, 250);
//        ImGui::SetColumnWidth(1, 250);
//
//
//        for (auto & renderProfilePair : laser->scannerSettings.renderProfiles) {
//            ImGui::PushItemWidth(120);
//            string name =renderProfilePair.first;
//            RenderProfile& profile = renderProfilePair.second;
//
//            RenderProfile& resetProfile = currentPreset.renderProfiles.at(name);
//
//            ImGui::Text("%s", name.c_str());
//            UI::addResettableFloatSlider(profile.speed,resetProfile.speed, "",  "%.1f", 3.0f);
//            UI::addResettableFloatSlider(profile.acceleration,resetProfile.acceleration, "",  "%.2f", 3.0f);
//            UI::addResettableIntSlider(profile.dotMaxPoints, resetProfile.dotMaxPoints);
//            UI::addResettableFloatSlider(profile.cornerThreshold, resetProfile.cornerThreshold);
//
//            ImGui::PopItemWidth();
//            ImGui::NextColumn();
//        }
//
//
//        ImGui::End();
//    }
//
//
//
//    // COLOUR SETTINGS
//    ImGui::Separator();
//    ImGui::Text("COLOUR");
//
//    if(ImGui::TreeNode("Colour calibration")){
//        colourPresetManager.drawComboBox(laser->colourSettings);
//        colourPresetManager.drawSaveButtons(laser->colourSettings);
//        UI::addParameterGroup(laser->colourSettings.params);
//
//        ImGui::TreePop();
//    }
//
//
//
//    ImGui::Separator();
//    ImGui::Text("ADVANCED SETTINGS");
//
//    // ADVANCED
//
//
//    if(ImGui::TreeNode("Advanced")){
//        // POINT RATE
//        ImGui::PushItemWidth(laserpanelwidth-60);
//        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 26.0f);
//        int ppsslider = laser->pps;
//        string ppsstring = "Point rate : " + ofToString(ppsslider);
//        if(ImGui::SliderInt("##Point rate", &ppsslider, laser->pps.getMin(), laser->pps.getMax(), ppsstring.c_str())){
//            laser->pps.set(ppsslider/100*100);
//
//        }
//        UI::toolTip("The actual points sent to the laser - YOU DON'T NEED TO ADJUST THIS unless you want to actually change the data rate, or you need better resolution for very fast scanners. The speed of the scanners can be fully adjusted without changing the point rate. ");
//        ImGui::PopItemWidth();
//        ImGui::PopStyleVar(1);
//
//        UI::addParameterGroup(laser->advancedParams);
//        ImGui::TreePop();
//    }
//
//
//
//    // draw a flashing dot during saving
//    if(laser->getSaveStatus() && (ofGetElapsedTimeMillis()%300)<150) {
//        ImDrawList*   draw_list = ImGui::GetWindowDrawList();
//        ImVec2 p = ImGui::GetWindowPos();
//        p.x+=ImGui::GetContentRegionAvailWidth();
//        p.y+=30;// + ImGui::GetScrollY();
//        // if(ImGui::GetScrollY()>0) p.x-=14;
//        //ImGui::GetContentRegionAvailWidth()
//        draw_list->AddCircleFilled(p, 4, ImGui::GetColorU32(ImGuiCol_Border));
//    }
//
//
//    //ImGui::End();
//    UI::endWindow();
//
//    drawUIPanelDacAnalytics();
//
//
//
//}

void Manager :: drawUIPopupDeleteLaser(Laser* laser, int index) {
    
    string label ="Delete Laser?##"+ofToString(index);
    if (ImGui::BeginPopupModal(label.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to delete this laser? All the zone settings will be deleted.\n\n");
        ImGui::Separator();
        
        UI::dangerColourStart();
        
        if (ImGui::Button("DELETE", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            deleteLaser(laser);
            
        }
        UI::dangerColourEnd();
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::EndPopup();
    }
}

void Manager :: drawUIPanelLaserCopySettings() {

    if(!copySettingsWindowOpen) return ;
    
    UI::startWindow("Copy laser settings", ImVec2(100, 100), ImVec2(0,0), ImGuiWindowFlags_None, false, &copySettingsWindowOpen );
    
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
            UI::startGhosted();
        }
        if(UI::addNumberedCheckBox(i+1, "##" + ofToString(i) + "laserToCopyTo", &copyactive, false)) {
            lasersToCopyTo[i] =  copyactive;
            
        }
        UI::stopGhosted();
        
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
            }
            
            if(copyColourSettings) {
                ofJson colourjson;
                ofSerialize(colourjson, sourceLaser.colourSettings.params);
                ofDeserialize(colourjson, targetLaser.colourSettings.params);
            }
            
//            if(copyZonePositions) {
//                 //????????
//
//                for(int j=0; j<sourceLaser.outputZones.size() ; j++) {
//                    if(targetLaser.outputZones.size()>j) {
//                        OutputZone* sourceZone = sourceLaser.outputZones[j];
//                        OutputZone* targetZone = targetLaser.outputZones[j];
//
//                        //targetZone->zone.
//
//                    }
//
//
//                }
//
//            }
            
            
        }
        copySettingsWindowOpen = false;
        
    }
    UI::dangerColourEnd();
    UI::endWindow();
    
}

void Manager::drawUIPanelDacAnalytics() {
    
    if(!showDacAnalytics) return;
    
    string label;
    
    ofxLaser::Laser* laser = &getLaser(getSelectedLaserIndex());
    
    UI::startWindow("Controller Analytics", ImVec2(guiSpacing, ofGetHeight()-guiSpacing-guiSpacing-600), ImVec2(ofGetWidth()-guiSpacing-guiSpacing, 600), ImGuiWindowFlags_None, false, &showDacAnalytics );
    
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

glm::vec2 Manager ::getPreviewOffset() {
    return previewOffset;
}
float Manager :: getPreviewScale() {
    return previewScale;
}
ofRectangle Manager :: getPreviewRect() {
    return ofRectangle(previewOffset.x, previewOffset.y, width*previewScale, height*previewScale);
    
}
void Manager :: fitPreviewInRect(ofRectangle fitrect ) {
    previewScale = fitrect.width/width;
    if(height*previewScale>fitrect.height) previewScale = fitrect.height/height;
    previewOffset = fitrect.getTopLeft();
    
}

void Manager :: initSVGs() {
    
    
    iconGrabOpen.loadFromString("<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" x=\"0px\" y=\"0px\" viewBox=\"0 0 24 30\" style=\"enable-background:new 0 0 24 24;\" xml:space=\"preserve\"><g><g><path fill=\"white\"  d=\"M16.313,23.134H3.384c-0.827,0-1.5-0.673-1.5-1.5c0-1.388,1.036-2.582,2.409-2.778l4.03-0.576l-7.543-7.543 c-0.428-0.428-0.665-0.998-0.665-1.604S0.352,7.958,0.781,7.53c0.325-0.325,0.731-0.539,1.173-0.624    C1.147,6.017,1.173,4.638,2.031,3.78c0.858-0.858,2.239-0.883,3.127-0.076C5.24,3.274,5.448,2.863,5.781,2.53    c0.884-0.885,2.323-0.885,3.207,0l0.171,0.171c0.083-0.429,0.29-0.839,0.622-1.171c0.884-0.885,2.323-0.885,3.207,0l8.982,8.982    c1.234,1.234,1.914,2.875,1.914,4.621s-0.68,3.387-1.914,4.621l-1.768,1.768C19.163,22.561,17.782,23.134,16.313,23.134z     M2.384,7.866c-0.325,0-0.649,0.124-0.896,0.371c-0.494,0.494-0.494,1.299,0,1.793l8.25,8.25c0.134,0.134,0.181,0.332,0.121,0.512    c-0.06,0.18-0.216,0.31-0.403,0.337l-5.02,0.717c-0.884,0.126-1.551,0.895-1.551,1.788c0,0.276,0.224,0.5,0.5,0.5h12.929    c1.202,0,2.332-0.468,3.182-1.318l1.768-1.768c1.045-1.045,1.621-2.436,1.621-3.914s-0.576-2.869-1.621-3.914l-8.982-8.982    c-0.494-0.494-1.299-0.494-1.793,0s-0.494,1.299,0,1.793l4.75,4.75c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354    c-0.195,0.195-0.512,0.195-0.707,0l-6.25-6.25c-0.479-0.479-1.313-0.479-1.793,0c-0.494,0.494-0.494,1.299,0,1.793l6.25,6.25    c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354c-0.195,0.195-0.512,0.195-0.707,0l-7.5-7.5    c-0.494-0.494-1.299-0.494-1.793,0s-0.494,1.299,0,1.793l7.5,7.5c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354    c-0.195,0.195-0.512,0.195-0.707,0l-6.25-6.25C3.033,7.99,2.709,7.866,2.384,7.866z\"/></g></g></svg>");
    
    iconGrabClosed.loadFromString("<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' version='1.1' x='0px' y='0px' viewBox='0 0 24 30' style='enable-background:new 0 0 24 24;' xml:space='preserve'><g><g><path fill='white' d='M14.058,21.129H9.129c-2.481,0-4.5-2.019-4.5-4.5v-1.793l-1.595-1.595c-0.884-0.884-0.884-2.323,0-3.207    c0.336-0.337,0.76-0.555,1.221-0.632C3.663,8.521,3.756,7.313,4.534,6.534C5.238,5.83,6.293,5.688,7.139,6.103    C7.199,5.62,7.415,5.154,7.784,4.784c0.857-0.858,2.237-0.883,3.126-0.077c0.084-0.441,0.298-0.848,0.624-1.173    c0.884-0.885,2.323-0.885,3.207,0l4.974,4.974c1.234,1.234,1.914,2.875,1.914,4.621s-0.68,3.387-1.914,4.621l-1.768,1.768    C16.908,20.557,15.527,21.129,14.058,21.129z M5.629,15.836v0.793c0,1.93,1.57,3.5,3.5,3.5h4.929c1.202,0,2.332-0.468,3.182-1.318    l1.768-1.768c1.045-1.045,1.621-2.436,1.621-3.914s-0.576-2.869-1.621-3.914l-4.974-4.974c-0.479-0.479-1.313-0.479-1.793,0    c-0.494,0.494-0.494,1.299,0,1.793l0.241,0.241c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354    c-0.195,0.195-0.512,0.195-0.707,0l-1.491-1.491c-0.494-0.494-1.299-0.494-1.793,0C8.252,5.731,8.12,6.05,8.12,6.388    s0.132,0.657,0.371,0.896l1.491,1.491c0.098,0.098,0.146,0.226,0.146,0.354S10.08,9.385,9.982,9.483    c-0.195,0.195-0.512,0.195-0.707,0L7.034,7.241c-0.494-0.494-1.299-0.494-1.793,0s-0.494,1.299,0,1.793l2.241,2.241    c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354c-0.195,0.195-0.512,0.195-0.707,0l-1.241-1.241    c-0.479-0.479-1.313-0.479-1.793,0C3.502,10.981,3.37,11.3,3.37,11.638s0.132,0.657,0.371,0.896l3.741,3.741    c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354c-0.195,0.195-0.512,0.195-0.707,0L5.629,15.836z'/></g></g></svg>");
    
    iconMagPlus.loadFromString("<?xml version='1.0' encoding='UTF-8' standalone='no'?> <!-- Created with Vectornator for iOS (http://vectornator.io/) --><!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'><svg height='100%' style='fill-rule:nonzero;clip-rule:evenodd;stroke-linecap:round;stroke-linejoin:round;' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns='http://www.w3.org/2000/svg' xml:space='preserve' width='100%' xmlns:vectornator='http://vectornator.io' version='1.1' viewBox='0 0 25 25'><defs/><g id='Untitled' vectornator:layerName='Untitled'><g opacity='1'><path d='M23.359+20.1039L20.4339+17.1788C19.5811+18.4307+18.4979+19.5139+17.246+20.3666L20.1711+23.2918C21.0517+24.1724+22.4798+24.1724+23.359+23.2918C24.2396+22.4111+24.2396+20.9845+23.359+20.1039Z' opacity='1' fill='#ffffff'/><path d='M21.0134+10.4223C21.0134+4.61158+16.3035-0.098298+10.4928-0.098298C4.68212-0.098298-0.0277553+4.61158-0.0277553+10.4223C-0.0277553+16.233+4.68212+20.9428+10.4928+20.9428C16.3035+20.9428+21.0134+16.2329+21.0134+10.4223ZM10.4928+18.6884C5.93482+18.6884+2.22665+14.9795+2.22665+10.4223C2.22665+5.86428+5.93482+2.15611+10.4928+2.15611C15.05+2.15611+18.759+5.86428+18.759+10.4223C18.759+14.9795+15.05+18.6884+10.4928+18.6884Z' opacity='1' fill='#ffffff'/></g><path d='M14.0491+8.87293L6.96295+8.88761C6.0614+8.88952+5.33189+9.62109+5.33379+10.5226C5.3357+11.4242+6.06874+12.1537+6.97029+12.1518L14.0564+12.1371C14.958+12.1352+15.6875+11.4036+15.6856+10.5021C15.6837+9.60054+14.9506+8.87103+14.0491+8.87293ZM12.1612+14.0485L12.1466+6.96241C12.1447+6.06085+11.4131+5.33134+10.5115+5.33325C9.60997+5.33515+8.88046+6.06819+8.88237+6.96975L8.89704+14.0559C8.89895+14.9574+9.63052+15.6869+10.5321+15.685C11.4336+15.6831+12.1631+14.9501+12.1612+14.0485Z' opacity='1' fill='#ffffff'/></g></svg>");
    
    iconMagMinus.loadFromString("<?xml version='1.0' encoding='UTF-8' standalone='no'?> <!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'><svg height='100%' style='fill-rule:nonzero;clip-rule:evenodd;stroke-linecap:round;stroke-linejoin:round;' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns='http://www.w3.org/2000/svg' xml:space='preserve' width='100%' version='1.1' viewBox='0 0 25 25'><defs/><g id='Untitled' vectornator:layerName='Untitled'><g opacity='1'><path d='M23.359+20.1039L20.4339+17.1788C19.5811+18.4307+18.4979+19.5139+17.246+20.3666L20.1711+23.2918C21.0517+24.1724+22.4798+24.1724+23.359+23.2918C24.2396+22.4111+24.2396+20.9845+23.359+20.1039Z' fill-rule='evenodd' fill='#ffffff' opacity='1'/><path d='M21.0134+10.4223C21.0134+4.61158+16.3035-0.098298+10.4928-0.098298C4.68212-0.098298-0.0277553+4.61158-0.0277553+10.4223C-0.0277553+16.233+4.68212+20.9428+10.4928+20.9428C16.3035+20.9428+21.0134+16.2329+21.0134+10.4223ZM10.4928+18.6884C5.93482+18.6884+2.22665+14.9795+2.22665+10.4223C2.22665+5.86428+5.93482+2.15611+10.4928+2.15611C15.05+2.15611+18.759+5.86428+18.759+10.4223C18.759+14.9795+15.05+18.6884+10.4928+18.6884Z' fill-rule='evenodd' fill='#ffffff' opacity='1'/></g><path d='M14.0491+8.87293L6.96295+8.88761C6.0614+8.88952+5.33189+9.62109+5.33379+10.5226C5.3357+11.4242+6.06874+12.1537+6.97029+12.1518L14.0564+12.1371C14.958+12.1352+15.6875+11.4036+15.6856+10.5021C15.6837+9.60054+14.9506+8.87103+14.0491+8.87293Z' opacity='1' fill='#ffffff'/></g></svg>");
    numberSVGs.resize(10);
    numberSVGs[0].loadFromString("<svg>\
         <path unicode='&#x30;' d='M1100 595Q1100 300 967.50 140Q835-20 594-20Q351-20 218 141Q85 302 85 595L85 861Q85 1156 217.50 1316Q350 1476 592 1476Q834 1476 967 1315Q1100 1154 1100 860L1100 595M763 904Q763 1061 722 1138.50Q681 1216 592 1216Q505 1216 465.50 1144Q426 1072 423 926L423 554Q423 391 464 315.50Q505 240 594 240Q680 240 721 313.50Q762 387 763 544L763 904Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[1].loadFromString("<svg>\
         <path unicode='&#x31;' d='M836 0L498 0L498 1076L166 979L166 1235L805 1456L836 1456L836 0Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[2].loadFromString("<svg>\
         <path unicode='&#x32;' d='M1105 0L89 0L89 220L557 712Q730 909 730 1025Q730 1119 689 1168Q648 1217 570 1217Q493 1217 445 1151.50Q397 1086 397 988L59 988Q59 1122 126 1235.50Q193 1349 312 1413Q431 1477 578 1477Q814 1477 941.50 1368Q1069 1259 1069 1055Q1069 969 1037 887.50Q1005 806 937.50 716.50Q870 627 720 477L532 260L1105 260L1105 0Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[3].loadFromString("<svg>\
         <path unicode='&#x33;' d='M397 869L556 869Q745 869 745 1054Q745 1126 700 1171.50Q655 1217 573 1217Q506 1217 456.50 1178Q407 1139 407 1081L70 1081Q70 1196 134 1286Q198 1376 311.50 1426.50Q425 1477 561 1477Q804 1477 943 1366Q1082 1255 1082 1061Q1082 967 1024.50 883.50Q967 800 857 748Q973 706 1039 620.50Q1105 535 1105 409Q1105 214 955 97Q805-20 561-20Q418-20 295.50 34.50Q173 89 110 185.50Q47 282 47 405L386 405Q386 338 440 289Q494 240 573 240Q662 240 715 289.50Q768 339 768 416Q768 526 713 572Q658 618 561 618L397 618L397 869Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[4].loadFromString("<svg>\
         <path unicode='&#x34;' d='M979 569L1127 569L1127 309L979 309L979 0L642 0L642 309L79 309L59 515L642 1453L642 1456L979 1456L979 569M380 569L642 569L642 1017L621 983L380 569Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[5].loadFromString("<svg>\
         <path unicode='&#x35;' d='M109 712L198 1456L1049 1456L1049 1194L472 1194L439 905Q475 926 533.50 942Q592 958 648 958Q865 958 981.50 829.50Q1098 701 1098 469Q1098 329 1035.50 215.50Q973 102 860 41Q747-20 593-20Q456-20 336 36.50Q216 93 148 191.50Q80 290 81 414L419 414Q424 334 470 287Q516 240 591 240Q761 240 761 492Q761 725 553 725Q435 725 377 649L109 712Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[6].loadFromString("<svg>\
         <path unicode='&#x36;' d='M903 1477L903 1212L888 1212Q699 1212 577.50 1121.50Q456 1031 428 870Q542 982 716 982Q905 982 1017 844Q1129 706 1129 482Q1129 343 1063.50 227Q998 111 880.50 45.50Q763-20 620-20Q465-20 343 50.50Q221 121 153 252Q85 383 83 554L83 689Q83 914 179.50 1093.50Q276 1273 455 1375Q634 1477 852 1477L903 1477M599 724Q532 724 487 693Q442 662 420 617L420 516Q420 240 611 240Q688 240 739.50 309Q791 378 791 482Q791 589 738.50 656.50Q686 724 599 724Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[7].loadFromString("<svg>\
         <path unicode='&#x37;' d='M1101 1276L557 0L201 0L746 1196L52 1196L52 1456L1101 1456L1101 1276Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[8].loadFromString("<svg>\
         <path unicode='&#x38;' d='M1071 1067Q1071 961 1018.50 880Q966 799 874 750Q978 699 1039 610.50Q1100 522 1100 401Q1100 206 965.50 93Q831-20 595-20Q358-20 221 93.50Q84 207 84 401Q84 518 144 608Q204 698 314 750Q221 799 168 880Q115 961 115 1067Q115 1257 244 1367Q373 1477 592 1477Q813 1477 942 1367Q1071 1257 1071 1067M762 428Q762 518 715.50 566.50Q669 615 593 615Q517 615 470 566.50Q423 518 423 428Q423 341 470.50 290.50Q518 240 595 240Q670 240 716 290Q762 340 762 428M592 1217Q524 1217 488.50 1172.50Q453 1128 453 1049Q453 971 489 923Q525 875 595 875Q664 875 698.50 923Q733 971 733 1049Q733 1127 697.50 1172Q662 1217 592 1217Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[9].loadFromString("<svg>\
         <path unicode='&#x39;' d='M753 563Q644 465 509 465Q311 465 195 599Q79 733 79 957Q79 1097 144 1217Q209 1337 326.50 1407Q444 1477 586 1477Q730 1477 846.50 1404.50Q963 1332 1028 1199.50Q1093 1067 1095 894L1095 765Q1095 524 1000.50 347Q906 170 731 75Q556-20 323-20L302-20L302 250L366 251Q714 268 753 563M600 708Q710 708 758 803L758 943Q758 1083 711 1150Q664 1217 584 1217Q513 1217 465 1141Q417 1065 417 957Q417 843 466 775.50Q515 708 600 708Z' fill='#ffffff'/>                  \
                     </svg>");
    
}

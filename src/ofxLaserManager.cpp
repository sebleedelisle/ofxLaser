//
//  ofxLaserManager.cpp
//  ofxLaser 2
//
//  Created by Seb Lee-Delisle on 06/05/2021.
//

#include "ofxLaserManager.h"
#include "ofxLaserCanvasViewController.h"
#include "ofxLaserZoneViewController.h"
#include "libera/avb/AvbManager.hpp"
#ifdef TARGET_OSX
#include <unordered_map>
#endif

using namespace ofxLaser;

namespace {

bool hasEmbeddedAvbSettings(const ofJson& json) {
    if (!json.contains("dacassigner") || !json["dacassigner"].is_object()) {
        return false;
    }
    if (!json["dacassigner"].contains("Libera") || !json["dacassigner"]["Libera"].is_object()) {
        return false;
    }

    const ofJson& liberaJson = json["dacassigner"]["Libera"];
    return liberaJson.contains("avb") && liberaJson["avb"].is_object();
}

ofJson makeLegacyAvbManagerJson(const ofJson& json) {
    ofJson liberaJson;
    liberaJson["avb"] = ofJson::object();
    liberaJson["avb"]["devices"] = json.contains("avb") && json["avb"].is_array()
        ? json["avb"]
        : ofJson::array();
    liberaJson["avb"]["halfXYOutputControllers"] =
        json.contains("avbControllers") && json["avbControllers"].is_array()
            ? json["avbControllers"]
            : ofJson::array();
    return liberaJson;
}

std::string getLaserStatusSummary(const std::shared_ptr<ofxLaser::Laser>& laser) {
    if (!laser || !laser->hasDac()) {
        return "Status: No controller assigned";
    }

    std::shared_ptr<ofxLaser::DacBase> dac = laser->getDac();
    if (!dac) {
        return "Status: No controller assigned";
    }

    return dac->getStatusSummary();
}

void addParameterToGuiGroup(ofxGuiGroup& guiGroup, ofAbstractParameter& parameter) {
    const std::string type = parameter.type();

    if (type == typeid(ofParameterGroup).name()) {
        guiGroup.add(static_cast<ofParameterGroup&>(parameter));
    } else if (type == typeid(ofParameter<bool>).name()) {
        guiGroup.add(static_cast<ofParameter<bool>&>(parameter));
    } else if (type == typeid(ofParameter<int>).name()) {
        guiGroup.add(static_cast<ofParameter<int>&>(parameter));
    } else if (type == typeid(ofParameter<float>).name()) {
        guiGroup.add(static_cast<ofParameter<float>&>(parameter));
    } else if (type == typeid(ofParameter<std::string>).name()) {
        guiGroup.add(static_cast<ofParameter<std::string>&>(parameter));
    } else if (type == typeid(ofParameter<glm::vec2>).name()) {
        guiGroup.add(static_cast<ofParameter<glm::vec2>&>(parameter));
    } else if (type == typeid(ofParameter<glm::vec3>).name()) {
        guiGroup.add(static_cast<ofParameter<glm::vec3>&>(parameter));
    } else if (type == typeid(ofParameter<glm::vec4>).name()) {
        guiGroup.add(static_cast<ofParameter<glm::vec4>&>(parameter));
    } else if (type == typeid(ofParameter<ofColor>).name()) {
        guiGroup.add(static_cast<ofParameter<ofColor>&>(parameter));
    } else if (type == typeid(ofParameter<ofFloatColor>).name()) {
        guiGroup.add(static_cast<ofParameter<ofFloatColor>&>(parameter));
    } else if (type == typeid(ofParameter<ofShortColor>).name()) {
        guiGroup.add(static_cast<ofParameter<ofShortColor>&>(parameter));
    } else if (type == typeid(ofParameter<ofRectangle>).name()) {
        guiGroup.add(static_cast<ofParameter<ofRectangle>&>(parameter));
    } else if (type == typeid(ofParameter<void>).name()) {
        guiGroup.add(static_cast<ofParameter<void>&>(parameter));
    }
}

} // namespace

Manager * Manager :: laserManager = nullptr;

Manager * Manager::instance() {
    if(laserManager == nullptr) {
        laserManager = new Manager();
    }
    return laserManager;
}

Manager :: Manager(bool hidecanvas) {
    canvasViewController = std::make_shared<CanvasViewController>();
    updateDisplayRectangle();
    
    if(laserManager == nullptr) {
        laserManager = this;
    } else {
        ofLog(OF_LOG_ERROR, "Multiple ofxLaser::Manager instances created");
    }

    initAndLoadSettings();
    
    // if no lasers are loaded make one and add a zone
    if(lasers.size()==0) {
        createAndAddLaser();
        createDefaultZone();
        std::shared_ptr<InputZone> zone = canvasTarget->getInputZoneForZoneIndex(0);
        addZoneToLaser(zone->zoneId, 0);
    }
    
    selectedLaserIndex = 0;
    viewMode  = OFXLASER_VIEW_CANVAS;
    
    guiIsVisible = true;
    guiIsMouseDisabled = false; 
    setDefaultPreviewOffsetAndScale();
    setupOfxGuiPanels();
        
    params.add(customParams);
    ofAddListener(ofEvents().mouseEntered, this, &Manager::mouseEntered, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseExited, this, &Manager::mouseExited, OF_EVENT_ORDER_BEFORE_APP);
    
    ofAddListener(ofEvents().mouseMoved, this, &Manager::mouseMoved, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mousePressed, this, &Manager::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseReleased, this, &Manager::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseDragged, this, &Manager::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseScrolled, this, &Manager::mouseScrolled, OF_EVENT_ORDER_BEFORE_APP);

    ofAddListener(ofEvents().keyPressed, this, &Manager::keyPressed, OF_EVENT_ORDER_BEFORE_APP);

    // Manager itself is a LaserBaseController — route messages to own receiveLaserMessage
    setLaserMessageCallback(this, &Manager::receiveLaserMessage);

    // Register canvas view controller as a LaserBaseView and set message callback
    addLaserView(canvasViewController.get());
    canvasViewController->setLaserMessageCallback(this, &Manager::receiveLaserMessage);

    // Zone views are already registered inside createAndAddLaser() during initAndLoadSettings.
    // Just set the message callbacks on any that were created before this point.
    for(auto& lzv : laserZoneViews) {
        lzv->setLaserMessageCallback(this, &Manager::receiveLaserMessage);
    }
    
    refreshOfxGuiPanels();

}

Manager::~Manager() {

    // Unregister all views from signal system
    for(auto& lzv : laserZoneViews) {
        removeLaserView(lzv.get());
    }
    if(canvasViewController) {
        removeLaserView(canvasViewController.get());
    }

    ofRemoveListener(ofEvents().mouseEntered, this, &Manager::mouseEntered, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseExited, this, &Manager::mouseExited, OF_EVENT_ORDER_BEFORE_APP);

    ofRemoveListener(ofEvents().mouseMoved, this, &Manager::mouseMoved, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseScrolled, this, &Manager::mouseScrolled, OF_EVENT_ORDER_BEFORE_APP);

    ofRemoveListener(ofEvents().mousePressed, this, &Manager::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseReleased, this, &Manager::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseDragged, this, &Manager::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(params.parameterChangedE(), this, &Manager::paramChanged);
    
    ofRemoveListener(ofEvents().keyPressed, this, &Manager::keyPressed, OF_EVENT_ORDER_BEFORE_APP);
    laserZoneViews.clear();
    canvasViewController = nullptr; 

}

void Manager::receiveLaserMessage(LaserMsgEnvelope& env) {
    ManagerBase::receiveLaserMessage(env);
}


void Manager :: resetAllLasersToDefault() {
    ManagerBase::resetAllLasersToDefault();
    viewMode = OFXLASER_VIEW_CANVAS;
    setSelectedLaserIndex(0);
    createDefaultZone();
    resetViews();
    
}

void Manager :: resetViews() {

    canvasViewController->setOutputRect(getCanvasPreviewRect());
    canvasViewController->autoFitToOutput();
    
    for(std::shared_ptr<LaserZoneViewController>& zoneView : laserZoneViews) {
        zoneView->autoFitToOutput();
    }
    
}

void Manager :: createAndAddLaser()  {
    int laserindex = lasers.size();
    ManagerBase:: createAndAddLaser();
    setLaserDefaultPreviewOffsetAndScale(laserindex);
    laserZoneViews.emplace_back(std::make_shared<LaserZoneViewController>(lasers.back()));
    auto& newView = laserZoneViews.back();
    newView->setOutputRect(getZonePreviewRect());
    newView->autoFitToOutput();
    newView->setGrid(zoneGridSnap, zoneGridSize, zoneGridVisible);

    // Register new zone view controller with the signal/message system
    addLaserView(newView.get());
    newView->setLaserMessageCallback(this, &Manager::receiveLaserMessage);

    setSelectedLaserIndex(laserindex);
    refreshOfxGuiPanels();
}



void Manager :: setLaserDefaultPreviewOffsetAndScale(int lasernum) {
    std::shared_ptr<LaserZoneViewController> laserview = getLaserViewControllerByIndex(lasernum);
    if(laserview!=nullptr) {
        laserview->autoFitToOutput();
    }
}

void Manager :: initAndLoadSettings() {
    
    if(initialised) {
        ofLogError("ofxLaser::Manager::initAndLoadSettings() called twice - NB you no longer need to call this in your code, it happens automatically");
        return ;
    }
    
    interfaceParams.setName("Interface");

    params.add(interfaceParams);
    
    customParams.setName("CUSTOM PARAMETERS");
    
    // is this still used ?
    //params.add(zoneEditorShowLaserPath.set("Show path in zone editor", true));
    //params.add(zoneEditorShowLaserPoints.set("Show points in zone editor", false));
    params.add(zoneGridSnap.set("Zone snap to grid", true));
    params.add(zoneGridSize.set("Zone grid size", 16,1,64));
    params.add(zoneGridVisible.set("Zone grid visible", true));
    
    params.add(canvasGridSnap.set("Canvas snap to grid", true));
    params.add(canvasGridSize.set("Canvas grid size", 20,1,50));
    params.add(canvasGridVisible.set("Canvas grid visible", true));
    
    params.add(globalLatency.set("Latency (ms)", 100,30,400));

    loadSettings();
    
    //updates zone settings and global latency on all
    updateLatencyToLasers();
    updateGridSettings();

    ofAddListener(params.parameterChangedE(), this, &Manager::paramChanged);
    
}


// I don't think this function is currently being called!
// ah except for when importing and exporting laser settings
// At the moment, loading and saving is done via the load/save functions.

// Currently uses files :
// ofxLaser/laserSettings.json - deserializes the params list, beam zones, canvas target
// It reads the numLasers value from laserSettings.json, then attempts to load lasers 
// for each one
//
// The deserialize function does the same thing, except I think it also saves all the laser
// settings within the same file. I believe the zones are saved as different files though,
// probably not great?
//
// ok so each laser also has a load/save settings function that looks for files.
// Will need to refactor that so it is serialize / deserialize. Lasers
// should not take ownership of their own load and save!
//
// How to untangle!!!
// Step one :
// Investigate Laser load save
//      laser.loadSettings only seems to ever be called by the laser manager
//      laser.saveSettings is used throughout Laser!

// Move zone load and save to Laser itself, and include in laser settings
//      Note that laser serialize / deserialize does actually include the zone
//      settings.
// At the moment zone load/save seems to have to manage whether the zone has
// changed itself - I'm guessing at some point it got slow and needed optimizing.
// This suggests that laser.save is called often. Research!
// Laser.saveSettings is called :
//      - when a zone is added
//      - when a zone is removed
//      - on updateZones, if a zone is in the changed list.
//          - *** WHEN is this called?
//          - updateZones is called by the laserManager when a beam zone or canvas
//            zone is deleted. Clumsy!
//          - I don't fully understand this system. I think it's something to do with
//            the connection between the zones in the laser and their interface. Honestly
//            it seems like a total mess. But that might be because I have forgotten
//            how it works.
//      - on Laser::update() if any of the zones return true to their "update" function
//      - when any of the parameters are changed (unless ignoreParamChange is set - ugh)



// Investigate how and when those functions are called and offset them up a level.
//
// Create serialize / deserialize for Laser that includes all zone settings!
// Rewrite laser load / save so that it uses the serialize / deserialize function
// Get rid of laser load / save, move it up to the laser manager.
//      Rewrite laser.saveSettings calls so that they mark the laser as needing save.

// WAIT... is bundling all the laser settings into one file sensible?
// PRO  - code is more consistent
//      - only one set of code for exporting ofxlaser setting / saving
// CON  - saving could be slow if you have a complex set up
//      - if save is interupted, whole set up could be corrupted

// SOLUTION :
// A hybrid approach where the lasers themselves no longer know about saving and loading
// but they do serialize and deserialize their data
// I need to move away from separate files for each zone data though.

void Manager :: serialize(ofJson& json) {
    ManagerBase::serialize(json);
}

bool Manager :: deserialize(ofJson& json) {
    bool success = ManagerBase::deserialize(json);
    loadAdditionalSettings(json);
    //cout << json.dump(3) << endl; 
    
    selectedLaserIndex = 0;

    resetViews();
    
    return success;
    
}

void Manager::loadAdditionalSettings(const ofJson& json) {
    // Compatibility strategy:
    // AVB settings now live under dacassigner/Libera so they are persisted as
    // DAC-manager state with the rest of the app. Keep this legacy importer so
    // older projects that saved top-level "avb" keys still migrate cleanly.
    if (hasEmbeddedAvbSettings(json)) {
        return;
    }

    auto liberaManager = dacAssigner.getManagerForType("Libera");
    if (!liberaManager) {
        return;
    }

    if (!json.contains("avb") && !json.contains("avbControllers")) {
        ofJson emptyLiberaJson;
        liberaManager->deserialize(emptyLiberaJson);
        return;
    }

    ofJson legacyLiberaJson = makeLegacyAvbManagerJson(json);
    liberaManager->deserialize(legacyLiberaJson);
}


void Manager :: paramChanged(ofAbstractParameter& e) {
    updateLatencyToLasers();
    updateGridSettings();

    scheduleSaveSettings();
}
void Manager :: updateLatencyToLasers() {
    for(std::shared_ptr<Laser>& laser : lasers) {
        laser->maxLatencyMS = globalLatency;
    }
    
}
void Manager :: updateGridSettings() {
    for(std::shared_ptr<LaserZoneViewController>& laserview : laserZoneViews) {
        laserview->setGrid(zoneGridSnap, zoneGridSize, zoneGridVisible);
        
    }
    canvasViewController->setGrid(canvasGridSnap, canvasGridSize, canvasGridVisible);
    
}


void Manager :: update() {

    ManagerBase :: update();

    updateDisplayRectangle();

    // Canvas UI↔model: when user drags zones, push to model then pull back
    // to enforce rectangular constraint (setFromRect snaps all 4 corners)
    if(canvasViewController->update()) {
        canvasViewController->updateZonesFromUI(canvasTarget);
        canvasViewController->updateUIFromZones(canvasTarget);
        scheduleSaveSettings();
    }

}

std::shared_ptr<LaserZoneViewController>   Manager ::getCurrentLaserViewController() {
    return getLaserViewControllerByIndex(selectedLaserIndex);
}
std::shared_ptr<LaserZoneViewController>   Manager ::getLaserViewControllerByIndex(int index) {
    if((index>=0) && (index<laserZoneViews.size())) {
        return laserZoneViews[index];
    } else {
        return nullptr;
    }

}

void Manager :: setDefaultPreviewOffsetAndScale(){

    canvasViewController->autoFitToOutput();
    
}

bool Manager :: mousePressed(ofMouseEventArgs &e){
    
    if(!windowActive) {
        windowActive = true;
        return false;
    }
    
    if(isMouseOverOfxGuiPanels(e.x, e.y)) return false;
    if(!isGuiVisible()) return false;
    if(isGuiMouseDisabled()) return false;
    
    if (viewMode  == OFXLASER_VIEW_CANVAS) {
        canvasViewController->mousePressed(e);

    } else if (viewMode  == OFXLASER_VIEW_OUTPUT) {
       
        std::shared_ptr<LaserZoneViewController> currentLaserView = getCurrentLaserViewController();
        if(currentLaserView) {
            currentLaserView->mousePressed(e);
        } else {
            ofLogError("ERROR - missing view for laser");
        }
        
    }
    return false;
}

bool Manager :: mouseReleased(ofMouseEventArgs &e){
    if(!windowActive) return false;

    if(isMouseOverOfxGuiPanels(e.x, e.y)) return false;
    for(std::shared_ptr<LaserZoneViewController>& zoneView : laserZoneViews) {
        zoneView->mouseReleased(e);
    }

    canvasViewController->mouseReleased(e);
    return false;
}


void Manager :: mouseMoved(ofMouseEventArgs &e) {
    if(!windowActive) return;

    if(isMouseOverOfxGuiPanels(e.x, e.y)) return;
    for(std::shared_ptr<LaserZoneViewController>& zoneView : laserZoneViews) {
        zoneView->mouseMoved(e);
    }
    canvasViewController->mouseMoved(e);

}


bool Manager :: mouseDragged(ofMouseEventArgs &e){
    if(!windowActive) return false;

    if(isMouseOverOfxGuiPanels(e.x, e.y)) return false;
    for(std::shared_ptr<LaserZoneViewController>& zoneView : laserZoneViews) {
        zoneView->mouseDragged(e);
    }
    canvasViewController->mouseDragged(e);

    return false;
}

void Manager :: mouseScrolled(ofMouseEventArgs &e){
    
    if(!windowActive) return;
    if(isMouseOverOfxGuiPanels(e.x, e.y)) return;
    if(!isGuiVisible()) return;
    if(isGuiMouseDisabled()) return;

    for(std::shared_ptr<LaserZoneViewController>& zoneView : laserZoneViews) {
        zoneView->mouseScrolled(e);
    }
    canvasViewController->mouseScrolled(e); 
   // return false;
    
}


void Manager :: mouseEntered(ofMouseEventArgs &e) {
    
    windowActive = true;
    
}
void Manager :: mouseExited(ofMouseEventArgs &e) {
    windowActive = false;
}
                   

bool Manager :: keyPressed(ofKeyEventArgs &e) {
    bool capturekey = false;
    for(std::shared_ptr<LaserZoneViewController>& laserZoneView : laserZoneViews) {
        if(laserZoneView->getSelected()) {
            bool changed = laserZoneView->keyPressed(e);
            capturekey|=changed;
            
        }
    }
    bool changed = canvasViewController->keyPressed(e);
    capturekey|=changed;
    
    // false means we keep the event bubbling
    return capturekey;
}
       
bool Manager :: keyReleased(ofKeyEventArgs &e){
    //false means we keep the event bubbling
    return false;
}


bool Manager :: deleteLaser(std::shared_ptr<Laser>& laser) {
    
    int laserindex = getLaserIndex(laser);
    
    bool success = ManagerBase::deleteLaser(laser);
    if(success) {
        if(selectedLaserIndex>=getNumLasers()) selectedLaserIndex = getNumLasers()-1;
        if(getNumLasers()==0) {
            viewMode = OFXLASER_VIEW_CANVAS;
        }
        
        if(laserindex!=-1) {
            // Unregister from signal system before erasing
            removeLaserView(laserZoneViews[laserindex].get());
            // laserZoneViews is not a pointer so memory should be freed
            laserZoneViews.erase(laserZoneViews.begin() + laserindex);
        }
        refreshOfxGuiPanels();
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
        ofxLaserViewMode previousViewMode = viewMode;
        if(viewMode == OFXLASER_VIEW_OUTPUT) {
            viewMode = OFXLASER_VIEW_CANVAS;
        } else {
            viewMode = OFXLASER_VIEW_OUTPUT;
        }
        if(viewMode != previousViewMode) {
            refreshPanelsForViewModeChange();
        }
    }
}

void Manager::selectPreviousLaser() {
    int prev = selectedLaserIndex-1;
    if(prev<0) prev=(int)lasers.size()-1;
    setSelectedLaserIndex(prev);
    
}

bool Manager::setSelectedLaserIndex(int i){
    if((selectedLaserIndex!=i) && (i<getNumLasers())) {
        selectedLaserIndex = i;
        refreshOfxGuiPanels();
        return true;
    } else {
        return false;
    }
}

bool Manager::selectAndShowLaser(int i) {
    ofxLaserViewMode previousViewMode = viewMode;
    bool changed = setSelectedLaserIndex(i);
    if(changed || (viewMode == OFXLASER_VIEW_CANVAS)) {
        viewMode = OFXLASER_VIEW_OUTPUT;
    } else {
        viewMode = OFXLASER_VIEW_CANVAS;
    }
    if(viewMode != previousViewMode) {
        refreshPanelsForViewModeChange();
    }
    return changed;
    
}

int Manager::getSelectedLaserIndex(){
    return selectedLaserIndex;
}
std::shared_ptr<Laser> Manager::getSelectedLaser() {
    
    if((selectedLaserIndex>=0) && (selectedLaserIndex<lasers.size())) {
        return lasers.at(selectedLaserIndex);
    } else {
        return nullptr;
    }
}
    
int Manager::getLaserIndex(std::shared_ptr<Laser>& laser) {
    for(int i = 0; i<lasers.size(); i++) {
        if(lasers[i] == laser) {
            return i;
        }
    }
    return -1;

}

void Manager::setCanvasSize(int width, int height) {
    const ofRectangle previousBounds = canvasTarget->getBounds();
    ManagerBase::setCanvasSize(width, height);

    if(canvasTarget->getBounds() == previousBounds) {
        return;
    }

    if(canvasViewController) {
        canvasViewController->updateUIFromZones(canvasTarget);
        canvasViewController->setOutputRect(getCanvasPreviewRect());
        canvasViewController->update();
    }

    setDefaultPreviewOffsetAndScale();
    fireCanvasChanged();
    scheduleSaveSettings();
}


bool Manager :: deleteZone(std::shared_ptr<InputZone> inputZone) {
    ZoneId zoneid = inputZone->zoneId;
    if(ManagerBase::deleteZone(inputZone) ) {
        for(std::shared_ptr<Laser>& laser : lasers) {
            laser->removeZone(zoneid);
            
        }
        canvasViewController->deselectAll();
        scheduleSaveSettings();
        return true;
    } else {
        return false;
    }
    
}


bool Manager ::toggleGui(){
    guiIsVisible = !guiIsVisible;
    return guiIsVisible;
}
void Manager ::setGuiVisible(bool visible){
    guiIsVisible = visible;
}
bool Manager::isGuiVisible() const {
    return guiIsVisible;
}

bool Manager::isGuiMouseDisabled() {
    return guiIsMouseDisabled;
}
void Manager ::setGuiMouseDisabled(bool state){
    guiIsMouseDisabled = state;
}

void Manager::addCustomParameter(ofAbstractParameter& param, bool loadFromSettings){
    customParams.add(param);
    refreshGlobalPanel();
    if(loadFromSettings){
        
        if(!loadedJson.empty()) {
            if(loadedJson.contains("Laser")) {
                if(loadedJson["Laser"].contains("CUSTOM_PARAMETERS")) {
                    try {
                        ofDeserialize(loadedJson["Laser"]["CUSTOM_PARAMETERS"], param);
                    } catch(...) {
                        
                    }
                    
                }
            }
        }
        
    }
}

glm::vec2 Manager::screenToLaserInput(glm::vec2& pos){
    // todo make into canvas view conversion
    glm::vec2 returnpos= pos ;//
    return returnpos;
    
}

void Manager::setupOfxGuiPanels() {
    ofxGuiSetFillColor(ofColor::fromHsb(144, 100, 112));
    ofxGuiSetDefaultWidth(220);
    ofxGuiSetDefaultHeight(20);

    globalPanel.setup("Laser", "", 10, 10);
    viewPanel.setup("View", "", 10, 10);
    laserSettingsPanel.setup("Laser Settings", "", 360, 10);
    selectedZonePanel.setup("Selected Zone", "", 810, 10);
    editorPanel.setup("Editor", "", 1160, 10);
    selectedCanvasItemPanel.setup("Selected Canvas Item", "", 810, 10);

    armAllButton.setup("ARM ALL", 220, 40);
    disarmAllButton.setup("DISARM ALL", 220, 40);
    canvasViewToggle.setup("Canvas", viewMode == OFXLASER_VIEW_CANVAS, 220, 40);
    outputViewToggle.setup("Output", viewMode == OFXLASER_VIEW_OUTPUT, 220, 40);
    previousLaserButton.setup("Previous Laser");
    nextLaserButton.setup("Next Laser");
    addLaserButton.setup("Add Laser");
    deleteLaserButton.setup("Delete Selected Laser");
    selectedLaserDisplay.set("Current Laser", 1, 1, 1);
    selectedZoneMuteToggle.set("Muted", false);
    selectedZoneLockToggle.set("Locked", false);
    selectedZoneQuadButton.setup("Quad");
    selectedZoneSegmentedButton.setup("Segmented");
    selectedZoneResetButton.setup("Reset To Default");
    selectedZoneDeleteButton.setup("Delete Zone");
    selectedZoneRemoveDistortionButton.setup("Remove Distortion");
    selectedZoneSubdivisionDownButton.setup("Less Detail");
    selectedZoneSubdivisionUpButton.setup("More Detail");
    selectedZoneManualBezier.set("Manual bezier editing", false);
    selectedZoneSmoothLevel.set("Smooth level", 0.3f, 0.0f, 0.5f);
    outputAddZoneButton.setup("Add Existing Zone");
    outputAddMaskButton.setup("Add Mask");
    outputTestPatternToggle.set("Test pattern", false);
    outputTestPatternIndex.set("Pattern", 1, 1, TestPatternGenerator::getNumTestPatterns());
    outputGridSizeGui.set("Zone grid size", zoneGridSize.get(), 1, 256);
    canvasAddZoneButton.setup("Add Canvas Zone");
    canvasAddGuideImageButton.setup("Add Guide Image");
    canvasGridSizeGui.set("Canvas grid size", canvasGridSize.get(), 1, 256);
    selectedCanvasMoveToBackButton.setup("Move To Back");
    selectedCanvasLockToggle.set("Locked", false);
    selectedCanvasTint.set("Tint", ofFloatColor(1, 1, 1, 1), ofFloatColor(0, 0, 0, 0), ofFloatColor(1, 1, 1, 1));
    selectedCanvasDeleteButton.setup("Delete");
    globalTestPatternToggle.set("Global test pattern", false);
    globalTestPatternIndex.set("Test pattern", 1, 1, TestPatternGenerator::getNumTestPatterns());
    laserArmedToggle.set("Armed", false);

    armAllButton.addListener(this, &Manager::onArmAllPressed);
    disarmAllButton.addListener(this, &Manager::onDisarmAllPressed);
    canvasViewToggle.addListener(this, &Manager::onCanvasViewToggled);
    outputViewToggle.addListener(this, &Manager::onOutputViewToggled);
    previousLaserButton.addListener(this, &Manager::onPreviousLaserPressed);
    nextLaserButton.addListener(this, &Manager::onNextLaserPressed);
    addLaserButton.addListener(this, &Manager::onAddLaserPressed);
    deleteLaserButton.addListener(this, &Manager::onDeleteLaserPressed);
    selectedZoneMuteToggle.addListener(this, &Manager::onSelectedZoneMuteChanged);
    selectedZoneLockToggle.addListener(this, &Manager::onSelectedZoneLockChanged);
    selectedZoneQuadButton.addListener(this, &Manager::onSelectedZoneQuadPressed);
    selectedZoneSegmentedButton.addListener(this, &Manager::onSelectedZoneSegmentedPressed);
    selectedZoneResetButton.addListener(this, &Manager::onSelectedZoneResetPressed);
    selectedZoneDeleteButton.addListener(this, &Manager::onSelectedZoneDeletePressed);
    selectedZoneRemoveDistortionButton.addListener(this, &Manager::onSelectedZoneRemoveDistortionPressed);
    selectedZoneSubdivisionDownButton.addListener(this, &Manager::onSelectedZoneSubdivisionDownPressed);
    selectedZoneSubdivisionUpButton.addListener(this, &Manager::onSelectedZoneSubdivisionUpPressed);
    selectedZoneManualBezier.addListener(this, &Manager::onSelectedZoneManualBezierChanged);
    selectedZoneSmoothLevel.addListener(this, &Manager::onSelectedZoneSmoothLevelChanged);
    outputAddZoneButton.addListener(this, &Manager::onOutputAddZonePressed);
    outputAddMaskButton.addListener(this, &Manager::onOutputAddMaskPressed);
    outputTestPatternToggle.addListener(this, &Manager::onOutputTestPatternChanged);
    outputTestPatternIndex.addListener(this, &Manager::onOutputTestPatternIndexChanged);
    outputGridSizeGui.addListener(this, &Manager::onOutputGridSizeChanged);
    canvasAddZoneButton.addListener(this, &Manager::onCanvasAddZonePressed);
    canvasAddGuideImageButton.addListener(this, &Manager::onCanvasAddGuideImagePressed);
    canvasGridSizeGui.addListener(this, &Manager::onCanvasGridSizeChanged);
    selectedCanvasMoveToBackButton.addListener(this, &Manager::onSelectedCanvasMoveToBackPressed);
    selectedCanvasLockToggle.addListener(this, &Manager::onSelectedCanvasLockChanged);
    selectedCanvasTint.addListener(this, &Manager::onSelectedCanvasTintChanged);
    selectedCanvasDeleteButton.addListener(this, &Manager::onSelectedCanvasDeletePressed);
    globalTestPatternToggle.addListener(this, &Manager::onGlobalTestPatternChanged);
    globalTestPatternIndex.addListener(this, &Manager::onGlobalTestPatternIndexChanged);
    laserArmedToggle.addListener(this, &Manager::onLaserArmedChanged);
    hideContentDuringTestPattern.addListener(static_cast<ManagerBase*>(this), &ManagerBase::hideContentDuringTestPatternChanged);
}

void Manager::refreshGlobalPanel() {
    glm::vec3 position = globalPanel.getPosition();
    float panelX = position.x;
    float panelY = position.y;
    if (panelX == 0 && panelY == 0) {
        panelX = 10;
        panelY = 10;
    }

    globalPanel.clear();
    globalPanel.setup("Laser", "", panelX, panelY);
    globalPanel.setWidthElements(220);

    globalPanel.setDefaultHeight(40);
    globalPanel.add(&armAllButton);
    globalPanel.add(&disarmAllButton);
    globalPanel.add(globalBrightness);

    globalPanel.setDefaultHeight(20);
    globalPanel.add(globalLatency);

    selectedLaserDisplay.setMax(std::max(1, getNumLasers()));
    selectedLaserDisplay.set(getNumLasers() > 0 ? getSelectedLaserIndex() + 1 : 1);
    globalPanel.add(selectedLaserDisplay);
    globalPanel.add(&addLaserButton);
    if (getNumLasers() > 1) {
        globalPanel.add(&deleteLaserButton);
    }

    globalTestPatternToggle.setWithoutEventNotifications(testPatternGlobalActive);
    globalTestPatternIndex.setWithoutEventNotifications(testPatternGlobal);
    globalPanel.add(globalTestPatternToggle);
    globalPanel.add(globalTestPatternIndex);
    globalPanel.add(hideContentDuringTestPattern);

    if (customParams.size() > 0) {
        globalPanel.add(customParams);
    }
    guiPanelLaserCount = getNumLasers();
}

void Manager::refreshViewPanel() {
    float panelX = globalPanel.getPosition().x;
    float panelY = globalPanel.getPosition().y + globalPanel.getHeight() + 8;

    viewPanel.clear();
    viewPanel.setup("View", "", panelX, panelY);
    viewPanel.setWidthElements(220);

    static_cast<ofParameter<bool>&>(canvasViewToggle.getParameter())
        .setWithoutEventNotifications(viewMode == OFXLASER_VIEW_CANVAS);
    static_cast<ofParameter<bool>&>(outputViewToggle.getParameter())
        .setWithoutEventNotifications(viewMode == OFXLASER_VIEW_OUTPUT);
    viewPanel.add(&canvasViewToggle);
    viewPanel.add(&outputViewToggle);
}

void Manager::refreshLaserSettingsPanel() {
    glm::vec3 position = laserSettingsPanel.getPosition();
    float panelX = position.x;
    float panelY = position.y;
    if (panelX == 0 && panelY == 0) {
        panelX = 360;
        panelY = 10;
    }

    std::string laserTitle = "Laser " + ofToString(getSelectedLaserIndex() + 1);
    laserSettingsPanel.clear();
    laserSettingsPanel.setup(laserTitle, "", panelX, panelY);
    laserSettingsPanel.setWidthElements(320);
    laserSettingsOwnedGroups.clear();
    laserSettingsOwnedButtons.clear();
    laserSettingsOwnedLabels.clear();
    laserSettingsButtonListeners.clear();

    std::shared_ptr<Laser> laser = getSelectedLaser();
    if (laser) {
        ofxGuiSetDefaultHeight(40);
        laserArmedToggle.setWithoutEventNotifications(laser->armed.get());
        laserSettingsPanel.add(laserArmedToggle);
        laserSettingsPanel.add(laser->intensity);

        outputTestPatternToggle.setWithoutEventNotifications(laser->testPatternActive);
        outputTestPatternIndex.setWithoutEventNotifications(laser->testPattern);
        laserSettingsPanel.add(outputTestPatternToggle);
        if (laser->testPatternActive) {
            laserSettingsPanel.add(outputTestPatternIndex);
        }

        // Speed and scanner sync are prominent controls
        laserSettingsPanel.add(laser->speed);
        laserSettingsPanel.add(laser->scannerSync);
        addLaserControllerControls(laserSettingsPanel, laser);

        ofxGuiSetDefaultHeight(20);

        for (std::size_t i = 0; i < laser->params.size(); ++i) {
            ofAbstractParameter& param = laser->params.get(i);
            // Skip internal fields and params already shown above
            if (param.getName() == "dacId" || param.getName() == "Brightness"
                || param.getName() == "Speed" || param.getName() == "Scanner sync") continue;
            addLaserSettingParameter(laserSettingsPanel, param, true);
        }
        laserSettingsPanel.minimizeAll();
        if (laserSettingsPanel.getControl("Controller") != nullptr) {
            laserSettingsPanel.getGroup("Controller").maximize();
        }
        if (laserSettingsPanel.getControl("Laser settings") != nullptr) {
            laserSettingsPanel.getGroup("Laser settings").maximize();
        }
        laserSettingsPanel.setWidthElements(320);
    }
    guiPanelSelectedLaserIndex = getSelectedLaserIndex();
    guiPanelDacSignature = getDacUiSignature();
    guiPanelSelectedLaserDacLabel = laser ? laser->dacLabel.get() : "";
    guiPanelSelectedLaserHasDac = laser && laser->hasDac();
    guiPanelSelectedLaserDacStatus = laser ? getLaserStatusSummary(laser) : "";
}

std::shared_ptr<OutputZone> Manager::getSelectedOutputZone() {
    std::shared_ptr<LaserZoneViewController> laserView = getCurrentLaserViewController();
    if (!laserView) {
        return nullptr;
    }
    return laserView->getSelectedOutputZone();
}

std::shared_ptr<MoveablePoly> Manager::getSelectedCanvasUiElement() {
    if (!canvasViewController) {
        return nullptr;
    }
    return canvasViewController->getSelectedUiElement();
}

void Manager::refreshSelectedZonePanel() {
    const ofRectangle previewRect = getZonePreviewRect();
    const float panelX = previewRect.getRight() + (16.0f * GlobalScale::getScale());
    const float panelY = previewRect.getTop();

    selectedZoneOwnedGroups.clear();

    std::shared_ptr<OutputZone> outputZone = getSelectedOutputZone();
    std::string panelTitle = "Selected Zone";
    if (outputZone) {
        panelTitle += ": " + outputZone->getLabel();
    }

    selectedZonePanel.clear();
    selectedZonePanel.setup(panelTitle, "", panelX, panelY);
    selectedZonePanel.setWidthElements(340);

    if (!outputZone) {
        guiPanelSelectedZoneUid.clear();
        guiPanelSelectedZoneType = -1;
        return;
    }

    static_cast<ofParameter<bool>&>(selectedZoneMuteToggle).setWithoutEventNotifications(outputZone->muted.get());
    static_cast<ofParameter<bool>&>(selectedZoneLockToggle).setWithoutEventNotifications(outputZone->locked.get());
    selectedZonePanel.add(selectedZoneMuteToggle);
    selectedZonePanel.add(selectedZoneLockToggle);
    selectedZonePanel.add(&selectedZoneQuadButton);
    selectedZonePanel.add(&selectedZoneSegmentedButton);
    selectedZonePanel.add(&selectedZoneResetButton);

    ZoneTransformBase* zoneTransform = &outputZone->getZoneTransform();
    if (auto* zoneTransformQuad = dynamic_cast<ZoneTransformQuadData*>(zoneTransform)) {
        selectedZonePanel.add(&selectedZoneRemoveDistortionButton);
        addParameterToGuiGroup(selectedZonePanel, zoneTransformQuad->useHomography);
    } else if (auto* zoneTransformQuadComplex = dynamic_cast<ZoneTransformQuadComplexData*>(zoneTransform)) {
        selectedZonePanel.add(&selectedZoneSubdivisionDownButton);
        selectedZonePanel.add(&selectedZoneSubdivisionUpButton);

        auto group = std::make_unique<ofxGuiGroup>();
        group->setup("Transform");
        group->setWidthElements(320);
        addParameterToGuiGroup(*group, zoneTransformQuadComplex->transformParams);
        group->maximize();
        selectedZonePanel.add(group.get());
        selectedZoneOwnedGroups.push_back(std::move(group));
    }

    selectedZonePanel.add(&selectedZoneDeleteButton);
    guiPanelSelectedZoneUid = ofToString(getSelectedLaserIndex()) + ":" + outputZone->getZoneId().getUid();
    guiPanelSelectedZoneType = outputZone->transformType;
}

void Manager::refreshEditorPanel() {
    glm::vec3 position = editorPanel.getPosition();
    float panelX = position.x;
    float panelY = position.y;
    if (panelX == 0 && panelY == 0) {
        panelX = 1160;
        panelY = 10;
    }

    editorPanel.clear();
    editorPanel.setup("Editor", "", panelX, panelY);
    editorPanel.setWidthElements(300);

    if (viewMode == OFXLASER_VIEW_OUTPUT) {
        std::shared_ptr<Laser> laser = getSelectedLaser();
        if (!laser) {
            return;
        }

        editorPanel.add(&outputAddZoneButton);
        editorPanel.add(&outputAddMaskButton);

        outputTestPatternToggle.setWithoutEventNotifications(laser->testPatternActive);
        outputTestPatternIndex.setWithoutEventNotifications(laser->testPattern);
        outputGridSizeGui.setWithoutEventNotifications(zoneGridSize.get());

        editorPanel.add(outputTestPatternToggle);
        if (laser->testPatternActive) {
            editorPanel.add(outputTestPatternIndex);
        }
        editorPanel.add(zoneGridSnap);
        editorPanel.add(outputGridSizeGui);
    } else if (viewMode == OFXLASER_VIEW_CANVAS) {
        canvasGridSizeGui.setWithoutEventNotifications(canvasGridSize.get());
        editorPanel.add(&canvasAddZoneButton);
        editorPanel.add(&canvasAddGuideImageButton);
        editorPanel.add(canvasGridSnap);
        editorPanel.add(canvasGridSizeGui);
    }
}

void Manager::refreshSelectedCanvasItemPanel() {
    const ofRectangle previewRect = getCanvasPreviewRect();
    const float panelX = previewRect.getRight() + (16.0f * GlobalScale::getScale());
    const float panelY = previewRect.getTop();

    selectedCanvasLaserButtons.clear();
    selectedCanvasLaserButtonListeners.clear();

    std::shared_ptr<MoveablePoly> uiElement = getSelectedCanvasUiElement();
    std::string panelTitle = "Selected Canvas Item";
    if (uiElement) {
        panelTitle += ": " + uiElement->getLabel();
    }

    selectedCanvasItemPanel.clear();
    selectedCanvasItemPanel.setup(panelTitle, "", panelX, panelY);
    selectedCanvasItemPanel.setWidthElements(340);

    if (!uiElement) {
        guiPanelSelectedCanvasUid.clear();
        return;
    }

    static_cast<ofParameter<bool>&>(selectedCanvasLockToggle).setWithoutEventNotifications(uiElement->getDisabled());
    selectedCanvasItemPanel.add(&selectedCanvasMoveToBackButton);
    selectedCanvasItemPanel.add(selectedCanvasLockToggle);

    std::shared_ptr<InputZone> inputZone = canvasTarget->getInputZoneForZoneIdUid(uiElement->getUid());
    std::shared_ptr<GuideImage> guideImage = canvasTarget->getGuideImageForUid(uiElement->getUid());

    if (guideImage) {
        selectedCanvasTint.setWithoutEventNotifications(guideImage->colour);
        selectedCanvasItemPanel.add(selectedCanvasTint);
    }

    if (inputZone) {
        ZoneId zoneId = inputZone->getZoneId();
        for (int i = 0; i < getNumLasers(); ++i) {
            bool assigned = lasers[i]->hasZone(zoneId);
            auto button = std::make_unique<ofxButton>();
            std::string label = assigned
                ? "Remove from Laser " + ofToString(i + 1)
                : "Add to Laser " + ofToString(i + 1);
            button->setup(label);
            auto token = static_cast<ofParameter<void>&>(button->getParameter()).newListener([this, zoneId, i, assigned]() {
                if (assigned) {
                    sendLaserMessage(LaserMsg::RemoveZoneFromLaser{zoneId.getUid(), i});
                } else {
                    sendLaserMessage(LaserMsg::AddZoneToLaser{zoneId.getUid(), i});
                }
                guiPanelSelectedCanvasUid = "__refresh__";
            });
            selectedCanvasItemPanel.add(button.get());
            selectedCanvasLaserButtonListeners.push_back(std::move(token));
            selectedCanvasLaserButtons.push_back(std::move(button));
        }
    }

    selectedCanvasItemPanel.add(&selectedCanvasDeleteButton);
    guiPanelSelectedCanvasUid = uiElement->getUid();
}

std::string Manager::getDacUiSignature() {
    std::string signature;
    const vector<std::shared_ptr<DacData>>& dacList = dacAssigner.getAvailableDacList();
    for (const std::shared_ptr<DacData>& dacdata : dacList) {
        if (!dacdata) continue;
        signature += dacdata->getLabel();
        signature += "|";
        signature += dacdata->alias;
        signature += "|";
        signature += dacdata->address;
        signature += "|";
        signature += ofToString(dacdata->available);
        signature += "|";
        signature += ofToString(dacdata->unavailable);
        signature += "|";
        if (dacdata->assignedLaser) {
            signature += ofToString(dacdata->assignedLaser->laserIndex);
        }
        signature += ";";
    }
    return signature;
}

void Manager::addLaserSettingsLabel(ofxGuiGroup& guiGroup, const std::string& name, const std::string& value) {
    auto label = std::make_unique<ofxLabel>();
    label->setup(name, value, 300, 20);
    guiGroup.add(label.get());
    laserSettingsOwnedLabels.push_back(std::move(label));
}

void Manager::addLaserSettingsButton(ofxGuiGroup& guiGroup, const std::string& label, std::function<void()> onPressed) {
    auto button = std::make_unique<ofxButton>();
    ofParameter<void> buttonParameter;
    buttonParameter.set(label);
    button->setup(buttonParameter);

    auto token = static_cast<ofParameter<void>&>(button->getParameter()).newListener([onPressed]() {
        if (onPressed) {
            onPressed();
        }
    });

    guiGroup.add(button.get());
    laserSettingsButtonListeners.push_back(std::move(token));
    laserSettingsOwnedButtons.push_back(std::move(button));
}

void Manager::addLaserControllerControls(ofxGuiGroup& guiGroup, std::shared_ptr<Laser> laser) {
    if (!laser) {
        return;
    }

    auto controllerGroup = std::make_unique<ofxGuiGroup>();
    controllerGroup->setup("Controller");
    controllerGroup->setWidthElements(300);

    const std::string savedDacLabel = laser->dacLabel.get();
    std::string controllerLabel = "None";
    if (laser->hasDac()) {
        controllerLabel = dacAssigner.getDisplayLabelForLabel(laser->getDacLabel());
    } else if (!savedDacLabel.empty()) {
        controllerLabel = dacAssigner.getDisplayLabelForLabel(savedDacLabel) + " (saved)";
    }

    addLaserSettingsLabel(*controllerGroup, "Assigned", controllerLabel);
    addLaserSettingsLabel(*controllerGroup, "Status", getLaserStatusSummary(laser));

    if (!savedDacLabel.empty()) {
        addLaserSettingsButton(*controllerGroup, "Reconnect", [this, savedDacLabel, laser]() {
            sendLaserMessage(LaserMsg::AssignDac{savedDacLabel, laser->laserIndex});
            guiPanelDacSignature = "__refresh__";
            guiPanelSelectedLaserDacLabel = "__refresh__";
            guiPanelSelectedLaserDacStatus = "__refresh__";
        });
    }

    if (laser->hasDac() || !savedDacLabel.empty()) {
        addLaserSettingsButton(*controllerGroup, "Disconnect", [this, laser]() {
            sendLaserMessage(LaserMsg::DisconnectDac{laser->laserIndex});
            guiPanelDacSignature = "__refresh__";
            guiPanelSelectedLaserDacLabel = "__refresh__";
            guiPanelSelectedLaserDacStatus = "__refresh__";
        });
    }

    addLaserSettingsButton(*controllerGroup, "Refresh Controllers", [this]() {
        dacAssigner.updateDacList();
        fireDacStatusChanged();
        guiPanelDacSignature = "__refresh__";
        guiPanelSelectedLaserDacLabel = "__refresh__";
        guiPanelSelectedLaserDacStatus = "__refresh__";
    });

    const vector<std::shared_ptr<DacData>>& dacList = dacAssigner.getAvailableDacList();
    if (dacList.empty()) {
        addLaserSettingsLabel(*controllerGroup, "Available", "None found");
    } else {
        addLaserSettingsLabel(*controllerGroup, "Available", ofToString(dacList.size()) + " found");
        for (const std::shared_ptr<DacData>& dacdata : dacList) {
            if (!dacdata) continue;

            const std::string dacLabel = dacdata->getLabel();
            const std::string displayLabel = dacAssigner.getDisplayLabelForLabel(dacLabel);
            const bool assignedToThisLaser = dacdata->assignedLaser && dacdata->assignedLaser.get() == laser.get();
            const bool assignedToAnotherLaser = dacdata->assignedLaser && dacdata->assignedLaser.get() != laser.get();

            if (assignedToThisLaser) {
                addLaserSettingsLabel(*controllerGroup, displayLabel, "Assigned here");
                continue;
            }

            if (dacdata->unavailable) {
                addLaserSettingsLabel(*controllerGroup, displayLabel, "In use");
                continue;
            }

            std::string buttonLabel = "Assign " + displayLabel;
            if (assignedToAnotherLaser) {
                buttonLabel = "Move " + displayLabel + " from Laser " + ofToString(dacdata->assignedLaser->laserIndex + 1);
            }

            addLaserSettingsButton(*controllerGroup, buttonLabel, [this, dacLabel, laser]() {
                sendLaserMessage(LaserMsg::AssignDac{dacLabel, laser->laserIndex});
                guiPanelDacSignature = "__refresh__";
                guiPanelSelectedLaserDacLabel = "__refresh__";
                guiPanelSelectedLaserDacStatus = "__refresh__";
            });
        }
    }

    controllerGroup->maximize();
    guiGroup.add(controllerGroup.get());
    laserSettingsOwnedGroups.push_back(std::move(controllerGroup));
}

void Manager::addLaserSettingParameter(ofxGuiGroup& guiGroup, ofAbstractParameter& parameter, bool topLevel) {
    const std::string type = parameter.type();

    if (type == typeid(ofParameterGroup).name()) {
        auto group = std::make_unique<ofxGuiGroup>();
        group->setup(parameter.getName());
        group->setWidthElements(300);

        auto& parameterGroup = static_cast<ofParameterGroup&>(parameter);
        for (std::size_t i = 0; i < parameterGroup.size(); ++i) {
            addLaserSettingParameter(*group, parameterGroup.get(i), false);
        }

        group->minimizeAll();
        if (topLevel && parameter.getName() == "Laser settings") {
            group->maximize();
        } else {
            group->minimize();
        }

        guiGroup.add(group.get());
        laserSettingsOwnedGroups.push_back(std::move(group));
        return;
    }

    if (type == typeid(ofParameter<bool>).name()) {
        addParameterToGuiGroup(guiGroup, parameter);
        return;
    }

    addParameterToGuiGroup(guiGroup, parameter);
}

void Manager::refreshOfxGuiPanels() {
    refreshGlobalPanel();
    refreshViewPanel();
    refreshLaserSettingsPanel();
    refreshSelectedZonePanel();
    refreshEditorPanel();
    refreshSelectedCanvasItemPanel();
}

void Manager::refreshPanelsForViewModeChange() {
    refreshViewPanel();
    refreshEditorPanel();
    refreshSelectedZonePanel();
    refreshSelectedCanvasItemPanel();
}

bool Manager::isMouseOverOfxGuiPanels(float x, float y) const {
    if (!isGuiVisible()) {
        return false;
    }

    const glm::vec2 point(x, y);
    if (globalPanel.getShape().inside(point)) return true;
    if (viewPanel.getShape().inside(point)) return true;
    if (laserSettingsPanel.getShape().inside(point)) return true;
    if (viewMode == OFXLASER_VIEW_OUTPUT && !guiPanelSelectedZoneUid.empty() && selectedZonePanel.getShape().inside(point)) return true;
    if (editorPanel.getShape().inside(point)) return true;
    if (viewMode == OFXLASER_VIEW_CANVAS && !guiPanelSelectedCanvasUid.empty() && selectedCanvasItemPanel.getShape().inside(point)) return true;
    return false;
}

void Manager::drawOfxGuiPanels() {
    if (!isGuiVisible()) {
        return;
    }

    if (guiPanelLaserCount != getNumLasers()) {
        refreshGlobalPanel();
    }
    if (guiPanelSelectedLaserIndex != getSelectedLaserIndex()) {
        refreshLaserSettingsPanel();
        refreshEditorPanel();
    }
    std::shared_ptr<Laser> selectedLaserForDac = getSelectedLaser();
    std::string dacSignature = getDacUiSignature();
    std::string selectedLaserDacLabel = selectedLaserForDac ? selectedLaserForDac->dacLabel.get() : "";
    bool selectedLaserHasDac = selectedLaserForDac && selectedLaserForDac->hasDac();
    std::string selectedLaserDacStatus = selectedLaserForDac ? getLaserStatusSummary(selectedLaserForDac) : "";
    if (dacSignature != guiPanelDacSignature
        || selectedLaserDacLabel != guiPanelSelectedLaserDacLabel
        || selectedLaserHasDac != guiPanelSelectedLaserHasDac
        || selectedLaserDacStatus != guiPanelSelectedLaserDacStatus) {
        refreshLaserSettingsPanel();
    }
    std::shared_ptr<OutputZone> selectedZone = getSelectedOutputZone();
    std::string selectedZoneUid = selectedZone
        ? ofToString(getSelectedLaserIndex()) + ":" + selectedZone->getZoneId().getUid()
        : "";
    int selectedZoneType = selectedZone ? selectedZone->transformType.get() : -1;
    if (selectedZoneUid != guiPanelSelectedZoneUid || selectedZoneType != guiPanelSelectedZoneType) {
        refreshSelectedZonePanel();
    }
    std::shared_ptr<MoveablePoly> selectedCanvasItem = getSelectedCanvasUiElement();
    std::string selectedCanvasUid = selectedCanvasItem ? selectedCanvasItem->getUid() : "";
    if (selectedCanvasUid != guiPanelSelectedCanvasUid) {
        refreshSelectedCanvasItemPanel();
    }

    globalPanel.draw();
    refreshViewPanel();
    viewPanel.draw();
    laserSettingsPanel.draw();
    editorPanel.draw();
    if (viewMode == OFXLASER_VIEW_OUTPUT && !guiPanelSelectedZoneUid.empty()) {
        selectedZonePanel.draw();
    }
    if (viewMode == OFXLASER_VIEW_CANVAS && !guiPanelSelectedCanvasUid.empty()) {
        selectedCanvasItemPanel.draw();
    }
}

void Manager::onArmAllPressed() {
    armAllLasers();
    refreshLaserSettingsPanel();
    bool state = true;
    ofNotifyEvent(armEvent, state);
}

void Manager::onDisarmAllPressed() {
    disarmAllLasers();
    refreshLaserSettingsPanel();
    bool state = false;
    ofNotifyEvent(armEvent, state);
}

void Manager::onCanvasViewToggled(bool& value) {
    if (!value) {
        if (viewMode == OFXLASER_VIEW_CANVAS) {
            static_cast<ofParameter<bool>&>(canvasViewToggle.getParameter()).setWithoutEventNotifications(true);
        }
        return;
    }

    if (viewMode != OFXLASER_VIEW_CANVAS) {
        viewMode = OFXLASER_VIEW_CANVAS;
    }
    static_cast<ofParameter<bool>&>(outputViewToggle.getParameter()).setWithoutEventNotifications(false);
    refreshGlobalPanel();
    refreshEditorPanel();
    refreshSelectedCanvasItemPanel();
}

void Manager::onOutputViewToggled(bool& value) {
    if (!value) {
        if (viewMode == OFXLASER_VIEW_OUTPUT) {
            static_cast<ofParameter<bool>&>(outputViewToggle.getParameter()).setWithoutEventNotifications(true);
        }
        return;
    }

    if (viewMode != OFXLASER_VIEW_OUTPUT) {
        viewMode = OFXLASER_VIEW_OUTPUT;
    }
    static_cast<ofParameter<bool>&>(canvasViewToggle.getParameter()).setWithoutEventNotifications(false);
    refreshGlobalPanel();
    refreshEditorPanel();
    refreshSelectedZonePanel();
}

void Manager::onPreviousLaserPressed() {
    selectPreviousLaser();
}

void Manager::onNextLaserPressed() {
    selectNextLaser();
}

void Manager::onAddLaserPressed() {
    sendLaserMessage(LaserMsg::AddLaser{});
    sendLaserMessage(LaserMsg::SaveSettings{});
    refreshOfxGuiPanels();
}

void Manager::onDeleteLaserPressed() {
    std::shared_ptr<Laser> laser = getSelectedLaser();
    if (laser && getNumLasers() > 1) {
        deleteLaser(laser);
        scheduleSaveSettings();
        refreshOfxGuiPanels();
    }
}

void Manager::onSelectedZoneMuteChanged(bool& value) {
    std::shared_ptr<OutputZone> outputZone = getSelectedOutputZone();
    if (!outputZone) {
        return;
    }
    sendLaserMessage(LaserMsg::ZoneMuteChanged{
        outputZone->getZoneId().getUid(),
        value});
}

void Manager::onSelectedZoneLockChanged(bool& value) {
    std::shared_ptr<OutputZone> outputZone = getSelectedOutputZone();
    if (!outputZone) {
        return;
    }

    sendLaserMessage(LaserMsg::ZoneLockChanged{
        outputZone->getZoneId().getUid(),
        value});
}

void Manager::onSelectedZoneQuadPressed() {
    std::shared_ptr<OutputZone> outputZone = getSelectedOutputZone();
    if (!outputZone) return;
    sendLaserMessage(LaserMsg::ZoneTypeChanged{outputZone->getZoneId().getUid(), 0});
}

void Manager::onSelectedZoneSegmentedPressed() {
    std::shared_ptr<OutputZone> outputZone = getSelectedOutputZone();
    if (!outputZone) return;
    sendLaserMessage(LaserMsg::ZoneTypeChanged{outputZone->getZoneId().getUid(), 2});
}

void Manager::onSelectedZoneResetPressed() {
    std::shared_ptr<OutputZone> outputZone = getSelectedOutputZone();
    if (!outputZone) return;
    sendLaserMessage(LaserMsg::ZoneResetTransform{outputZone->getZoneId().getUid()});
}

void Manager::onSelectedZoneDeletePressed() {
    std::shared_ptr<LaserZoneViewController> laserView = getCurrentLaserViewController();
    std::shared_ptr<OutputZone> outputZone = getSelectedOutputZone();
    if (!outputZone) return;
    if (laserView) {
        laserView->deselectAll();
    }
    sendLaserMessage(LaserMsg::DeleteOutputZone{
        outputZone->getZoneId().getUid()});
    guiPanelSelectedZoneUid = "__refresh__";
    guiPanelSelectedZoneType = -2;
}

void Manager::onSelectedZoneRemoveDistortionPressed() {
    std::shared_ptr<OutputZone> outputZone = getSelectedOutputZone();
    if (!outputZone) return;
    if (auto* zoneTransformQuad = dynamic_cast<ZoneTransformQuadData*>(&outputZone->getZoneTransform())) {
        zoneTransformQuad->resetToSquare();
    }
}

void Manager::onSelectedZoneSubdivisionDownPressed() {
    std::shared_ptr<OutputZone> outputZone = getSelectedOutputZone();
    if (!outputZone) return;
    if (auto* zoneTransformQuadComplex = dynamic_cast<ZoneTransformQuadComplexData*>(&outputZone->getZoneTransform())) {
        zoneTransformQuadComplex->decSubdivisionLevel();
    }
}

void Manager::onSelectedZoneSubdivisionUpPressed() {
    std::shared_ptr<OutputZone> outputZone = getSelectedOutputZone();
    if (!outputZone) return;
    if (auto* zoneTransformQuadComplex = dynamic_cast<ZoneTransformQuadComplexData*>(&outputZone->getZoneTransform())) {
        zoneTransformQuadComplex->incSubdivisionLevel();
    }
}

void Manager::onSelectedZoneManualBezierChanged(bool& value) {
}

void Manager::onSelectedZoneSmoothLevelChanged(float& value) {
}

void Manager::onOutputAddZonePressed() {
    std::shared_ptr<Laser> laser = getSelectedLaser();
    if (!laser) return;

    for (std::shared_ptr<InputZone>& zone : canvasTarget->getInputZones()) {
        ZoneId zoneId = zone->getZoneId();
        if (!laser->hasZone(zoneId)) {
            sendLaserMessage(LaserMsg::AddZoneToLaser{zoneId.getUid(), getSelectedLaserIndex()});
            return;
        }
    }
}

void Manager::onOutputAddMaskPressed() {
    std::shared_ptr<Laser> laser = getSelectedLaser();
    if (!laser) return;
    laser->maskManager.addQuadMask();
}

void Manager::onOutputTestPatternChanged(bool& value) {
    std::shared_ptr<Laser> laser = getSelectedLaser();
    if (!laser) return;
    sendLaserMessage(LaserMsg::SetLaserTestPattern{
        getSelectedLaserIndex(), outputTestPatternIndex.get(), value});
    refreshEditorPanel();
}

void Manager::onOutputTestPatternIndexChanged(int& value) {
    std::shared_ptr<Laser> laser = getSelectedLaser();
    if (!laser || !laser->testPatternActive) return;
    sendLaserMessage(LaserMsg::SetLaserTestPattern{
        getSelectedLaserIndex(), value, true});
}

void Manager::onOutputGridSizeChanged(int& value) {
    zoneGridSize.set(value);
}

void Manager::onCanvasAddZonePressed() {
    int numZones = canvasTarget->getNumZoneIds();
    sendLaserMessage(LaserMsg::AddZone{
        (float)(numZones * 20), (float)(numZones * 20), 100, 100});
}

void Manager::onCanvasAddGuideImagePressed() {
    ofFileDialogResult result = ofSystemLoadDialog("Choose guide image file");
    if(result.bSuccess) {
        canvasTarget->addGuideImage(result.filePath);
        scheduleSaveSettings();
        fireCanvasChanged();
    }
}

void Manager::onCanvasGridSizeChanged(int& value) {
    canvasGridSize.set(value);
}

void Manager::onSelectedCanvasMoveToBackPressed() {
    std::shared_ptr<MoveablePoly> uiElement = getSelectedCanvasUiElement();
    if (!uiElement) return;

    vector<std::shared_ptr<MoveablePoly>>& elements = canvasViewController->uiElementsSorted;
    auto it = find(elements.begin(), elements.end(), uiElement);
    if (it != elements.end()) {
        elements.erase(it);
        elements.insert(elements.begin(), uiElement);
        uiElement->setSelected(false);
        guiPanelSelectedCanvasUid = "__refresh__";
    }
}

void Manager::onSelectedCanvasLockChanged(bool& value) {
    std::shared_ptr<MoveablePoly> uiElement = getSelectedCanvasUiElement();
    if (!uiElement) return;

    uiElement->setDisabled(value);
    canvasViewController->zonesChangedFlag = true;
}

void Manager::onSelectedCanvasDeletePressed() {
    std::shared_ptr<MoveablePoly> uiElement = getSelectedCanvasUiElement();
    if (!uiElement) return;

    std::shared_ptr<InputZone> inputZone = canvasTarget->getInputZoneForZoneIdUid(uiElement->getUid());
    std::shared_ptr<GuideImage> guideImage = canvasTarget->getGuideImageForUid(uiElement->getUid());

    if (inputZone) {
        sendLaserMessage(LaserMsg::DeleteZone{inputZone->getZoneId().getUid()});
        sendLaserMessage(LaserMsg::SaveSettings{});
    } else if (guideImage) {
        canvasTarget->deleteGuideImage(guideImage);
        scheduleSaveSettings();
        fireCanvasChanged();
    }

    canvasViewController->deselectAll();
    guiPanelSelectedCanvasUid = "__refresh__";
}

void Manager::onSelectedCanvasTintChanged(ofFloatColor& value) {
    std::shared_ptr<MoveablePoly> uiElement = getSelectedCanvasUiElement();
    if (!uiElement) return;

    std::shared_ptr<GuideImage> guideImage = canvasTarget->getGuideImageForUid(uiElement->getUid());
    if (guideImage) {
        guideImage->colour = value;
        canvasViewController->zonesChangedFlag = true;
    }
}


void Manager::onGlobalTestPatternChanged(bool& value) {
    sendLaserMessage(LaserMsg::SetTestPattern{testPatternGlobal, value});
}

void Manager::onGlobalTestPatternIndexChanged(int& value) {
    sendLaserMessage(LaserMsg::SetTestPattern{value, testPatternGlobalActive});
}

void Manager::onLaserArmedChanged(bool& value) {
    std::shared_ptr<Laser> laser = getSelectedLaser();
    if (laser) {
        sendLaserMessage(LaserMsg::SetLaserArmed{laser->laserIndex, value});
    }
}

void Manager:: drawUI(){
    
    drawPreviews();
    drawOfxGuiPanels();
}

void Manager :: drawPreviews() {
    
    for(std::shared_ptr<LaserZoneViewController>& laserZoneView : laserZoneViews) {
        laserZoneView->setIsVisible((viewMode==OFXLASER_VIEW_OUTPUT) &&(laserZoneView->getLaser().get()==getSelectedLaser().get()));
        laserZoneView->setOutputRect(getZonePreviewRect(), true);
    }
    
    canvasViewController->setIsVisible(viewMode==OFXLASER_VIEW_CANVAS);

    if(viewMode == OFXLASER_VIEW_CANVAS) {
        canvasViewController->setOutputRect(getCanvasPreviewRect(), true);
       
        if(isGuiVisible()) {
            canvasViewController->draw();
            canvasViewController->beginViewPort(false);
            renderPreview();
            canvasViewController->endViewPort();
        }
    
        
    } else if(viewMode == OFXLASER_VIEW_OUTPUT){
        
        // make the selected laser visible
        if(isGuiVisible()) {
            for(std::shared_ptr<LaserZoneViewController>& laserZoneView : laserZoneViews) {
                laserZoneView->update();
                laserZoneView->draw();
                if(laserZoneView->getIsVisible()) drawBigNumber(laserZoneView->getLaserIndex());
            }
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
    
    std::vector <std::shared_ptr<Shape>>& shapes = canvasTarget->shapes;
    // Draw the preview laser graphics, with zones overlaid
    for(std::shared_ptr<Shape>& shape : shapes) {
        shape->addPreviewToMesh(mesh);
    }
    
    float scale = GlobalScale::getScale();
    ofSetLineWidth(1.5*scale);
    mesh.draw();
    
    std::vector<ofFloatColor>& colours = mesh.getColors();
    
    for(size_t i= 0; i<colours.size(); i++) {
        colours[i].r*=0.4;
        colours[i].g*=0.4;
        colours[i].b*=0.4;
    }
    
    ofSetLineWidth(5*scale);
    mesh.draw();

    ofDisableBlendMode();

    ofPopStyle();
}


bool Manager::setOverlayIconsVisible(bool state) {
    
    if(overlayIconsVisible!=state) {
        overlayIconsVisible = state;
        return true;
    } else {
        return false;
    }
    
}

ofRectangle Manager :: getPreviewRect() {
    return canvasViewController->getOutputRect();
    
}

glm::vec2 Manager :: getPreviewOffset() {
    return canvasViewController->getOffset();
}

float Manager :: getPreviewScale() {
    return canvasViewController->getScale();
}

ofRectangle Manager :: getZonePreviewRect() {

    int w = windowRectangle.getWidth()*0.6;
    int h = windowRectangle.getHeight()*0.6;
    int size = (MIN(w, h));
    float scale = GlobalScale::getScale();
    return ofRectangle(54*scale, 10*scale, size, size);
}

ofRectangle Manager :: getCanvasPreviewRect() {

    float ratio = (float)canvasTarget->getWidth() / (float)canvasTarget->getHeight();
    
    int h = windowRectangle.getHeight()*0.5;
    int w = h*ratio;
    if(w>windowRectangle.getWidth()*0.8) {
        w = windowRectangle.getWidth()*0.8;
        h = w/ratio;
    }

    float scale = GlobalScale::getScale();
    //int scale = GlobalScale::getScale();
    //cout << scale << endl;
    return ofRectangle(54*scale, 10*scale, w, h);
}



void Manager :: drawBigNumber(int number) {
    ofPushStyle();
    const ofRectangle previewRect = getZonePreviewRect();
    float scale = GlobalScale::getScale();
    const glm::vec2 labelPos = previewRect.getTopLeft() + glm::vec2(8, 16) * scale;
    ofDrawBitmapStringHighlight("Laser " + ofToString(number + 1), labelPos, ofColor(0, 180), ofColor(255));
    ofPopStyle();
}

bool  Manager :: updateDisplayRectangle() {
    
    if(autoSizeToScreen) {
        
        return setWindowRectangle(0,0,ofGetWidth(), ofGetHeight());
    } else {
    
        return false;
    }
    
}

bool Manager :: setWindowRectangle(int x, int y, int w, int h) {
    
    ofRectangle newrectangle(x, y, w, h);
    if(newrectangle!=windowRectangle) {
        windowRectangle = newrectangle;
        return true;
    } else {
        return false;
    }
    
}

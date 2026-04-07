//
//  ofxLaserManager.h
//
//  Created by Seb Lee-Delisle on 06/05/2021.
//

#pragma once
#include "ofxLaserManagerBase.h"
#include "ofxLaserUIState.h"
#include "ofxLaserBaseController.h"
#include "ofxButton.h"
#include "GlobalScale.h"
#include "ofxGui.h"
#include <memory>

#ifdef OFXLASER_USE_OFXNATIVE
#include "ofxNative.h"
#endif

namespace ofxLaser {

class CanvasViewController;
class LaserZoneViewController;
class MoveablePoly;

class Manager : public ManagerBase, public LaserBaseController {
    
    public :
    
    // it's a Singleton so shouldn't ever have more than one.
    static Manager * instance();
    static Manager * laserManager;
    
    Manager(bool hidecanvas = false);
    ~Manager();
    
    virtual void resetAllLasersToDefault() override;
    void resetViews(); 
    virtual void initAndLoadSettings();
    virtual void update() override;
    virtual void createAndAddLaser() override;
    void receiveLaserMessage(LaserMsgEnvelope& env);
    
    void paramChanged(ofAbstractParameter& e) ;
    void updateLatencyToLasers();
    void updateGridSettings();
    virtual bool deleteLaser(std::shared_ptr<Laser>& laser) override;

    void addCustomParameter(ofAbstractParameter& param, bool loadFromSettings = true);
    
    virtual void serialize(ofJson& json) override;
    virtual bool deserialize(ofJson& json) override;

    ofRectangle getPreviewRect();
    float getPreviewScale();
    glm::vec2 getPreviewOffset();

    ofRectangle getZonePreviewRect();
    ofRectangle getCanvasPreviewRect();
    
    bool updateDisplayRectangle(); 
    bool setWindowRectangle(int x, int y, int w, int h);

    bool setOverlayIconsVisible(bool state); 
    
    virtual void setCanvasSize(int width, int height) override;
    virtual bool deleteZone(std::shared_ptr<InputZone> inputZone) override;
   
    void selectNextLaser();
    void selectPreviousLaser();
    bool selectAndShowLaser(int i);
    bool setSelectedLaserIndex(int i);
    
    int getLaserIndex(std::shared_ptr<Laser>& laser);
    int getSelectedLaserIndex();

    std::shared_ptr<Laser> getSelectedLaser();
    
    std::shared_ptr<LaserZoneViewController>  getCurrentLaserViewController();
    std::shared_ptr<LaserZoneViewController>  getLaserViewControllerByIndex(int index);
   
    void drawUI();
    void drawPreviews();
    
    void renderPreview();
   
    glm::vec2 screenToLaserInput(glm::vec2& pos);
    
    void drawOfxGuiPanels();
    void setupOfxGuiPanels();
    void refreshOfxGuiPanels();
    void refreshGlobalPanel();
    void refreshLaserSettingsPanel();
    void refreshSelectedZonePanel();
    void refreshEditorPanel();
    void refreshSelectedCanvasItemPanel();
    void addLaserSettingParameter(ofxGuiGroup& guiGroup, ofAbstractParameter& parameter, bool topLevel);
    bool isMouseOverOfxGuiPanels(float x, float y) const;
    std::shared_ptr<OutputZone> getSelectedOutputZone();
    std::shared_ptr<MoveablePoly> getSelectedCanvasUiElement();
    void onArmAllPressed();
    void onDisarmAllPressed();
    void onCanvasViewToggled(bool& value);
    void onOutputViewToggled(bool& value);
    void onPreviousLaserPressed();
    void onNextLaserPressed();
    void onAddLaserPressed();
    void onDeleteLaserPressed();
    void onSelectedZoneMuteChanged(bool& value);
    void onSelectedZoneLockChanged(bool& value);
    void onSelectedZoneQuadPressed();
    void onSelectedZoneLinePressed();
    void onSelectedZoneSegmentedPressed();
    void onSelectedZoneResetPressed();
    void onSelectedZoneDeletePressed();
    void onSelectedZoneRemoveDistortionPressed();
    void onSelectedZoneSubdivisionDownPressed();
    void onSelectedZoneSubdivisionUpPressed();
    void onSelectedZoneManualBezierChanged(bool& value);
    void onSelectedZoneSmoothLevelChanged(float& value);
    void onOutputAddZonePressed();
    void onOutputAddMaskPressed();
    void onOutputTestPatternChanged(bool& value);
    void onOutputTestPatternIndexChanged(int& value);
    void onOutputGridSizeChanged(int& value);
    void onGlobalTestPatternChanged(bool& value);
    void onGlobalTestPatternIndexChanged(int& value);
    void onLaserArmedChanged(bool& value);
    void onCanvasAddZonePressed();
    void onCanvasAddGuideImagePressed();
    void onCanvasGridSizeChanged(int& value);
    void onSelectedCanvasMoveToBackPressed();
    void onSelectedCanvasLockChanged(bool& value);
    void onSelectedCanvasDeletePressed();
    void onSelectedCanvasTintChanged(ofFloatColor& value);

    bool toggleGui();
    void setGuiVisible(bool visible);
    bool isGuiVisible() const;
    
    bool isGuiMouseDisabled();
    void setGuiMouseDisabled(bool state);
    
    void drawBigNumber(int number); 
    
    void mouseEntered(ofMouseEventArgs &e);
    void mouseExited(ofMouseEventArgs &e);
    
    void mouseMoved(ofMouseEventArgs &e);
    bool mousePressed(ofMouseEventArgs &e);
    bool mouseReleased(ofMouseEventArgs &e);
    bool mouseDragged(ofMouseEventArgs &e);
    void mouseScrolled(ofMouseEventArgs &e);
  
    bool keyPressed(ofKeyEventArgs &e);
    bool keyReleased(ofKeyEventArgs &e);

    void setDefaultPreviewOffsetAndScale();
    void setLaserDefaultPreviewOffsetAndScale(int lasernum);
     
    //----------- DEPRECATED ------------------------
    
    OF_DEPRECATED_MSG("ofxLaser::Manager::nextProjector - use selectNextLaser() ", void nextProjector());
    OF_DEPRECATED_MSG("ofxLaser::Manager::previousProjector - use selectPreviousLaser() ", void previousProjector());

    bool overlayIconsVisible = true;

    ofParameterGroup interfaceParams;
    ofParameterGroup customParams;
    ofParameter<bool> zoneGridSnap;
    ofParameter<int> zoneGridSize;
    ofParameter<bool> zoneGridVisible;
    
    ofParameter<bool> canvasGridSnap;
    ofParameter<int> canvasGridSize;
    ofParameter<bool> canvasGridVisible;
    
    // TODO are these used?
    ofParameter<bool> zoneEditorShowLaserPath;
    ofParameter<bool> zoneEditorShowLaserPoints;
    
    ofParameter<int> globalLatency; 

    ofxLaserViewMode viewMode;
   
    ofEvent<bool> armEvent;

    bool autoSizeToScreen = true;

    vector<std::shared_ptr<LaserZoneViewController>> laserZoneViews;
    std::shared_ptr<CanvasViewController> canvasViewController;
    
    ofxPanel globalPanel;
    ofxPanel laserSettingsPanel;
    ofxPanel selectedZonePanel;
    ofxPanel editorPanel;
    ofxPanel selectedCanvasItemPanel;
    ofParameter<int> selectedLaserDisplay;
    ofxButton armAllButton;
    ofxButton disarmAllButton;
    ofxToggle canvasViewToggle;
    ofxToggle outputViewToggle;
    ofxButton previousLaserButton;
    ofxButton nextLaserButton;
    ofxButton addLaserButton;
    ofxButton deleteLaserButton;
    ofParameter<bool> selectedZoneMuteToggle;
    ofParameter<bool> selectedZoneLockToggle;
    ofxButton selectedZoneQuadButton;
    ofxButton selectedZoneLineButton;
    ofxButton selectedZoneSegmentedButton;
    ofxButton selectedZoneResetButton;
    ofxButton selectedZoneDeleteButton;
    ofxButton selectedZoneRemoveDistortionButton;
    ofxButton selectedZoneSubdivisionDownButton;
    ofxButton selectedZoneSubdivisionUpButton;
    ofParameter<bool> selectedZoneManualBezier;
    ofParameter<float> selectedZoneSmoothLevel;
    ofxButton outputAddZoneButton;
    ofxButton outputAddMaskButton;
    ofParameter<bool> outputTestPatternToggle;
    ofParameter<int> outputTestPatternIndex;
    ofParameter<int> outputGridSizeGui;
    ofxButton canvasAddZoneButton;
    ofxButton canvasAddGuideImageButton;
    ofParameter<int> canvasGridSizeGui;
    ofxButton selectedCanvasMoveToBackButton;
    ofParameter<bool> selectedCanvasLockToggle;
    ofParameter<ofFloatColor> selectedCanvasTint;
    std::vector<std::unique_ptr<ofxButton>> selectedCanvasLaserButtons;
    std::vector<std::unique_ptr<of::priv::AbstractEventToken>> selectedCanvasLaserButtonListeners;
    ofxButton selectedCanvasDeleteButton;
    ofParameter<bool> globalTestPatternToggle;
    ofParameter<int> globalTestPatternIndex;
    ofParameter<bool> laserArmedToggle;
    int guiPanelLaserCount = -1;
    int guiPanelSelectedLaserIndex = -1;
    std::string guiPanelSelectedZoneUid;
    int guiPanelSelectedZoneType = -1;
    std::string guiPanelSelectedCanvasUid;
    std::vector<std::unique_ptr<ofxGuiGroup>> laserSettingsOwnedGroups;
    std::vector<std::unique_ptr<ofxButton>> laserSettingsOwnedButtons;
    std::vector<std::unique_ptr<of::priv::AbstractEventToken>> laserSettingsButtonListeners;
    std::vector<std::unique_ptr<ofxGuiGroup>> selectedZoneOwnedGroups;

    protected :

    bool initialised = false;
    bool windowActive = true;

    int selectedLaserIndex;

    bool guiIsVisible;
    bool guiIsMouseDisabled;

    ofRectangle windowRectangle;
    
    
    void loadAdditionalSettings(const ofJson& json) override;
   

};
}

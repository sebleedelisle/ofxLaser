//
//  ofxLaserManager.h
//
//  Created by Seb Lee-Delisle on 06/05/2021.
//

#pragma once
#include "ofxLaserManagerBase.h"
#include "ofxLaserUI.h"
#include "ofxLaserZoneViewController.h"
#include "ofxLaserCanvasViewController.h"
#include "ofxLaserIconSVGs.h"
#include "GlobalScale.h"
#include "ofxLaserPresetManager.h"

#ifdef OFXLASER_USE_OFXNATIVE
#include "ofxNative.h"
#endif

enum ofxLaserViewMode {
    OFXLASER_VIEW_CANVAS, // show default canvas view
    OFXLASER_VIEW_OUTPUT, // show laser output system
    
};

namespace ofxLaser {

class Manager : public ManagerBase {
    
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
    virtual bool deleteCanvasZone(std::shared_ptr<InputZone> inputZone) override;
   
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
    
    void drawLaserGui();
    void startLaserUI() ;
    void finishLaserUI() ;
    
    void guiMenuBar();
    void guiTopBar(int ypos);
    void guiDacAssignment();
    void guiLaserOverview();
    void guiLaserSettings(std::shared_ptr<Laser>& laser);
    //void guiZoneSettings();
    
    void guiCopyLaserSettings();
    void drawGuiAllDacAnalytics(); 
    void guiDacAnalytics(int index);
    void guiCustomParameters();
    void guiShowLaserZoneContextMenu(); 
    
    // pop ups
    bool guiDeleteLaserButtonAndPopup(std::shared_ptr<Laser>& laser, int index);
    void guiEditDacAliasButtonAndPopup(string daclabel);

    bool guiRenameZonePopup(std::shared_ptr<ObjectWithZoneId> zoneIdContainer);

    bool renderMainMenu = true; 
    bool renderIconBar = true; 
    ofParameter<bool> showCustomParametersWindow;
    ofParameter<bool> showLaserOverviewWindow;
    ofParameter<bool> showLaserOutputSettingsWindow;
   // bool showCanvasSettingsWindow;
    bool showCopySettingsWindow;
    bool showDacAssignmentWindow;
    bool showAVBWindow;
   // bool showBeamZoneSortWindow;
    int canvasWidthInterface = -1; // only used for the canvas settings interface.
    int canvasHeightInterface = -1; // there must be a better way but i can't think of it right now
    
    
    bool toggleGui();
    void setGuiVisible(bool visible);
    bool isGuiVisible();
    
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

    int guiLaserSettingsPanelWidth;
    int guiSpacing;
    int menuBarHeight = 20;
    int iconBarHeight = 32;

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

    vector<bool> showDacDiagnostics;
    ofParameter<float> dacSettingsTimeSlice;

    ofxLaserViewMode viewMode;
   
    ofEvent<bool> armEvent;
    
    bool autoSizeToScreen = true;
  
    vector<std::shared_ptr<LaserZoneViewController>> laserZoneViews;
    std::shared_ptr<CanvasViewController> canvasViewController;
   
    protected :
    
    bool initialised = false;
    bool windowActive = true; 
   
    int selectedLaserIndex;

    bool guiIsVisible;
    bool guiIsMouseDisabled;
    bool showEditScannerPreset = false;
    
    DacData dacToAssign;
    std::shared_ptr<Laser> laserToAssign = nullptr;
    std::shared_ptr<Laser> laserToDelete = nullptr;
    
    //ofImage guideImage;
  
    PresetManager<ScannerSettings> scannerPresetManager;
    PresetManager<ColourSettings> colourPresetManager;
    
    // copy settings system :
    // TODO Break into its own object
    // is actually an array of booleans
    deque<bool> lasersToCopyTo;
    ofParameterGroup copyParams; 
    ofParameter<bool> copyScannerSettings;
    ofParameter<bool> copyAdvancedSettings;
    ofParameter<bool> copyColourSettings;
    ofParameter<bool> copyZonePositions;
    
    ofRectangle windowRectangle;
    
    
    IconSVGs iconSVGs; 
   

};
}




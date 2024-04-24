//
//  ofxLaserManager.h
//
//  Created by Seb Lee-Delisle on 06/05/2021.
//

#pragma once
#include "ofxLaserManagerBase.h"
#include "ofxLaserUI.h"
#include "ofxLaserVisualiser3D.h"
#include "ofxLaserZoneViewController.h"
#include "ofxLaserCanvasViewController.h"
#include "ofxLaserIconSVGs.h"
#include "GlobalScale.h"

#define OFX_LASER_HIDE_CANVAS false

enum ofxLaserViewMode {
    OFXLASER_VIEW_3D, // show 3D view
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
    virtual void initAndLoadSettings();
    virtual void update() override;
    virtual void createAndAddLaser() override;
    void paramChanged(ofAbstractParameter& e) ;
    
    bool deleteLaser(Laser* laser) override;
    
    void addCustomParameter(ofAbstractParameter& param, bool loadFromSettings = true);
    
    virtual void serialize(ofJson& json) override;
    virtual bool deserialize(ofJson& json) override;

    ofRectangle getPreviewRect();
    float getPreviewScale();
    glm::vec2 getPreviewOffset();

    ofRectangle getZonePreviewRect();
    ofRectangle getCanvasPreviewRect();
    ofRectangle getVisualiserPreviewRect();
    
    bool updateDisplayRectangle(); 
    bool setDisplayRectangle(int x, int y, int w, int h);

    bool setOverlayIconsVisible(bool state); 
    
    virtual void setCanvasSize(int width, int height) override;
    virtual bool deleteCanvasZone(InputZone* inputZone) override;
   
    void selectNextLaser();
    void selectPreviousLaser();
    int getLaserIndex(Laser* laser); 
    int getSelectedLaserIndex();
    void setSelectedLaserIndex(int i);
    Laser* getSelectedLaser();
    

    
    LaserZoneViewController*  getCurrentLaserViewController();
    LaserZoneViewController*  getLaserViewControllerByIndex(int index);
   
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
    void guiLaserSettings(ofxLaser::Laser* laser);
    //void guiLaserOutputSettings();
    void guiZoneSettings();
    
    void guiCopyLaserSettings();
    void guiDacAnalytics();
    void guiCustomParameters();
    void guiShowLaserZoneContextMenu(); 
    
    // pop ups
    bool guiDeleteLaserButtonAndPopup(Laser* laser, int index);
    void guiEditDacAliasButtonAndPopup(string daclabel);

    ofParameter<bool> showCustomParametersWindow;
    ofParameter<bool> showLaserOverviewWindow;
    ofParameter<bool> showLaserOutputSettingsWindow;
    bool showCanvasSettingsWindow;
    bool showCopySettingsWindow;
    bool showDacAssignmentWindow;
    bool showBeamZoneSortWindow;
    
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
    bool enableCanvasFunctionality = true;
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

    bool showDacAnalytics;
    ofParameter<float> dacSettingsTimeSlice;

    ofxLaserViewMode viewMode;
   
    ofEvent<bool> armEvent;
    
    bool autoSizeToScreen = true;
  
    vector<LaserZoneViewController> laserZoneViews;
    CanvasViewController canvasViewController;
   
    
    protected :
    
    bool initialised = false;
    bool windowActive = true; 
   
    int selectedLaserIndex;

    bool guiIsVisible;
    bool guiIsMouseDisabled;
    bool showEditScannerPreset = false;
    
    Visualiser3D visualiser3D;
    ofRectangle visualiser3DRect; 
    
    DacData dacToAssign;
    ofxLaser::Laser* laserToAssign = nullptr;
    
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
    
    ofRectangle displayRectangle;
    
    
    IconSVGs iconSVGs; 
   

};
}




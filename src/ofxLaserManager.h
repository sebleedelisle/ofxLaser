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
    
    Manager();
    ~Manager();
    
    virtual void initAndLoadSettings();
    virtual void update() override;
    virtual void createAndAddLaser() override;
    void paramChanged(ofAbstractParameter& e) ;
    
    bool deleteLaser(Laser* laser) override;
    
    void addCustomParameter(ofAbstractParameter& param, bool loadFromSettings = true);

    ofRectangle getPreviewRect();
    ofRectangle getZonePreviewRect();
    
    virtual void setCanvasSize(int width, int height) override; 
    bool setGuideImage(string filename);

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
    void guiLaserOutputSettings(); 

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
    bool copySettingsWindowOpen;
    bool showDacAssignmentWindow;
    
    bool toggleGui();
    void setGuiVisible(bool visible);
    bool isGuiVisible();
    
    void drawBigNumber(int number); 
    
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
    int iconBarHeight = 44;
    
    ofParameterGroup interfaceParams;
    ofParameterGroup customParams;
    ofParameter<bool> zoneGridSnap;
    ofParameter<int> zoneGridSize;
        
    // TODO are these used?
    ofParameter<bool> zoneEditorShowLaserPath;
    ofParameter<bool> zoneEditorShowLaserPoints;
    
    ofParameter<int> globalLatency; 

    bool showDacAnalytics;
    ofParameter<float> dacSettingsTimeSlice;

    ofxLaserViewMode viewMode;
   
    protected :
    
     
    bool initialised = false;
   
    int selectedLaserIndex;

    
    bool guiIsVisible;
    bool showEditScannerPreset = false;
    
    Visualiser3D visualiser3D; 
    
    DacData dacToAssign;
    ofxLaser::Laser* laserToAssign = nullptr;
    
    ofImage guideImage;
  
    vector<LaserZoneViewController> laserZoneViews;
    bool showCanvas = false; 
    CanvasViewController canvasViewController;
     
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
    
    
    IconSVGs iconSVGs; 
   

};
}




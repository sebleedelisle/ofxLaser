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


enum ofxLaserViewMode {
    OFXLASER_VIEW_3D, // show 3D view
    OFXLASER_VIEW_CANVAS, // show default canvas view
    OFXLASER_VIEW_OUTPUT, // show laser output system
    
};
enum ofxLaserMouseMode {
    OFXLASER_MOUSE_DEFAULT,
    OFXLASER_MOUSE_DRAG, //
    OFXLASER_MOUSE_ZOOM_IN, //
    OFXLASER_MOUSE_ZOOM_OUT, //
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
    
    void initSVGs(); 
    
    bool deleteLaser(Laser* laser) override;
    
    void addCustomParameter(ofAbstractParameter& param, bool loadFromSettings = true);
    glm::vec2 getPreviewOffset();
    float getPreviewScale();
    ofRectangle getPreviewRect();
    void fitPreviewInRect(ofRectangle fitrect);
    
    virtual void setCanvasSize(int width, int height) override; 
    bool setGuideImage(string filename);

    void selectNextLaser();
    void selectPreviousLaser();
    int getLaserIndex(Laser* laser); 
    int getSelectedLaserIndex();
    void setSelectedLaserIndex(int i);
    
    LaserZoneViewController*  getCurrentLaserViewController();
    LaserZoneViewController*  getLaserViewControllerByIndex(int index);
   

    void drawUI();
    void drawPreviews();
    
    void renderPreview();
   
    glm::vec2 screenToLaserInput(glm::vec2& pos);
    

    void drawLaserGui();
    void startLaserUI() ;
    void finishLaserUI() ;
    void renderCustomCursors();
    
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
    void guiDeleteLaserButtonAndPopup(Laser* laser, int index);
    void guiEditDacAliasButtonAndPopup(string daclabel);

    ofParameter<bool> showCustomParametersWindow;
    ofParameter<bool> showLaserManagementWindow;
    ofParameter<bool> showLaserOutputSettingsWindow;
    bool copySettingsWindowOpen;
    bool showDacAssignmentWindow;
    
    bool togglePreview();
    // TODO I think this functionality is broken
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
    void zoomPreviewAroundPoint(glm::vec2 anchor, float zoomMultiplier);
    void setLaserDefaultPreviewOffsetAndScale(int lasernum);
    
    bool draggingPreview;
    glm::vec2 dragStartPoint;
    
    //----------- DEPRECATED ------------------------
    
    OF_DEPRECATED_MSG("ofxLaser::Manager::nextProjector - use selectNextLaser() ", void nextProjector());
    OF_DEPRECATED_MSG("ofxLaser::Manager::previousProjector - use selectPreviousLaser() ", void previousProjector());

    int guiLaserSettingsPanelWidth;
    int guiSpacing;
    int menuBarHeight = 20;
    int iconBarHeight = 44;
    
    ofParameterGroup interfaceParams;
    ofParameterGroup customParams;
    
    ofParameter<bool> lockInputZones;
    ofParameter<bool> showInputZones;
    ofParameter<bool> showInputPreview;
    ofParameter<bool> showOutputPreviews;
    ofParameter<bool> zoneGridSnap;
    ofParameter<int> zoneGridSize;
        
    ofParameter<bool> zoneEditorShowLaserPath;
    ofParameter<bool> zoneEditorShowLaserPoints;
    
    ofParameter<int> globalLatency; 
    
    ofParameter<bool> showGuideImage;
    ofParameter<ofColor> guideImageColour;
    ofParameter<string> guideImageFilename; 

    bool showDacAnalytics;
    ofParameter<float> dacSettingsTimeSlice;

    ofxLaserViewMode viewMode;
   
    protected :
    
     
    bool initialised = false;
   
    int selectedLaserIndex;
    //bool showOutputInterface; // the zone editing interface
    ofxLaserMouseMode mouseMode;
    
    bool guiIsVisible;
    bool showEditScannerPreset = false;
    
    Visualiser3D visualiser3D; 
    
    DacData dacToAssign;
    ofxLaser::Laser* laserToAssign = nullptr;
    
    ofImage guideImage;
  
    vector<LaserZoneViewController> laserZoneViews;
    
    glm::vec2 previewOffset;
    float previewScale;
    
    ofxSVGExtra iconGrabOpen;
    ofxSVGExtra iconGrabClosed;
    ofxSVGExtra iconMagPlus;
    ofxSVGExtra iconMagMinus;
    vector<ofxSVGExtra> numberSVGs;
    
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
    
   // bool firstUpdate = true;
   

};
}




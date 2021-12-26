//
//  ofxLaserManager.h
//
//  Created by Seb Lee-Delisle on 06/05/2021.
//

#pragma once
#include "ofxLaserManagerBase.h"
#include "ofxLaserUI.h"

namespace ofxLaser {
class Manager : public ManagerBase {
    
    public : 
    Manager();
    ~Manager();
    
    bool deleteLaser(Laser* laser) override;
    
    void addCustomParameter(ofAbstractParameter& param, bool loadFromSettings = true);
    glm::vec2 getPreviewOffset() {
        return previewOffset;
    }
    float getPreviewScale() {
        return previewScale;
    }
    ofRectangle getPreviewRect() {
        return ofRectangle(previewOffset.x, previewOffset.y, width*previewScale, height*previewScale); 
        
    }
    void fitPreviewInRect(ofRectangle fitrect ) {
        previewScale = fitrect.width/width;
        if(height*previewScale>fitrect.height) previewScale = fitrect.height/height;
        previewOffset = fitrect.getTopLeft();
        
    }
    
    virtual void setCanvasSize(int width, int height) override; 
    bool setGuideImage(string filename);

    void selectNextLaser();
    void selectPreviousLaser();
    int getSelectedLaser();
    void setSelectedLaser(int i);
    bool isAnyLaserSelected();

    void drawUI();
    void drawPreviews();
    
    void renderPreview();
   
    glm::vec2 screenToLaserInput(glm::vec2& pos); 
    void setDefaultHandleSize(float size);
    
    void drawLaserSettingsPanel(ofxLaser::Laser* laser, float laserpanelwidth, float spacing, float x);
    void drawLaserGui();
    void startLaserUI() ;
    void finishLaserUI() ;
    void renderCustomCursors(); 
   
    bool togglePreview();
    // TODO I think this functionality is broken
    bool toggleGui();
    void setGuiVisible(bool visible);
    bool isGuiVisible();
    
    
    bool mousePressed(ofMouseEventArgs &e);
    bool mouseReleased(ofMouseEventArgs &e);
    bool mouseDragged(ofMouseEventArgs &e);

    void setDefaultPreviewOffsetAndScale(); 
    void zoomPreviewAroundPoint(glm::vec2 anchor, float zoomMultiplier);
    
    bool draggingPreview;
    glm::vec2 dragStartPoint;
    
    //----------- DEPRECATED ------------------------
    
    OF_DEPRECATED_MSG("ofxLaser::Manager::nextProjector - use selectNextLaser() ", void nextProjector());
    OF_DEPRECATED_MSG("ofxLaser::Manager::previousProjector - use selectPreviousLaser() ", void previousProjector());

    int guiLaserSettingsPanelWidth;
    int guiSpacing;

    protected :
    
    ofParameter<bool> showGuideImage;
    ofParameter<ofColor> guideImageColour;
    ofParameter<bool> previewNavigationEnabled;
    
    int selectedLaser;
   
    bool guiIsVisible;
    bool showEditScannerPreset = false;
    
    
    ofImage guideImage;
  
    glm::vec2 previewOffset;
    float previewScale;
    
    ofxSVGExtra iconGrabOpen;
    ofxSVGExtra iconGrabClosed;
    ofxSVGExtra iconMagPlus;
    ofxSVGExtra iconMagMinus; 

};
}




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
    
    bool deleteLaser(Laser* laser) override;
    
    void addCustomParameter(ofAbstractParameter& param);

    
    void selectNextLaser();
    void selectPreviousLaser();
    int getSelectedLaser();
    void setSelectedLaser(int i);
    bool isAnyLaserSelected();

    void drawUI(bool fullscreen = false);
    void drawPreviews(bool fullscreen = false);
    
    
    void renderPreview();
   
    glm::vec2 screenToLaserInput(glm::vec2& pos); 
    void setDefaultHandleSize(float size);
    
    void drawLaserSettingsPanel(ofxLaser::Laser* laser, float laserpanelwidth, float spacing, float x);
    void drawLaserGui();
    bool togglePreview();
    bool toggleGui();
    void setGuiVisible(bool visible);
    bool isGuiVisible();
    
    protected :
    int selectedLaser;
   
    bool guiIsVisible;
    bool showEditScannerPreset = false;
    
    int guiLaserSettingsPanelWidth;
    int dacStatusBoxSmallWidth;
    int dacStatusBoxHeight;
    int guiSpacing;
    
  
    glm::vec2 previewOffset;
    float previewScale;

};
}




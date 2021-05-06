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
    
    bool deleteProjector(Projector* projector) override;
    
    void addCustomParameter(ofAbstractParameter& param);

    
    void nextProjector();
    void previousProjector();
    int getCurrentProjector();
    void setCurrentProjector(int i);
    bool isProjectorSelected();

    void drawUI(bool fullscreen = false);
    void drawPreviews(bool fullscreen = false);
    
    
    void renderPreview();
   
    
    void setDefaultHandleSize(float size);
    
    void drawProjectorPanel(ofxLaser::Projector* projector, float projectorpanelwidth, float spacing, float x);
    void drawLaserGui();
    bool togglePreview();
    bool toggleGui();
    void setGuiVisible(bool visible);
    bool isGuiVisible();
    
    protected :
    int currentProjector;
   
    bool guiIsVisible;
    bool showEditScannerPreset = false;
    
    

};
}




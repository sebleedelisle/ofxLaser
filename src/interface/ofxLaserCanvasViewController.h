//
//  ofxLaserCanvasViewController.h
//
//  Created by Seb Lee-Delisle on 11/02/2023.
//
//

#pragma once

#include "ofxLaserViewWithMoveables.h"
#include "ofxLaserUI.h"
#include "ofxLaserShapeTargetCanvas.h"
#include "ofxLaserGuideImageUiQuad.h"

namespace ofxLaser {

class CanvasInputZoneQuad : public MoveablePoly  {
    
    public :
    
    CanvasInputZoneQuad() {
        setHue(0);
        setBrightness(180);
        
    }
    
};

class CanvasViewController : public ViewWithMoveables {
    
    public :
    void drawImGui();
    bool updateZonesFromUI(ShapeTargetCanvas& canvasTarget);
    void updateUIFromZones(ShapeTargetCanvas& canvasTarget);
    
    void setOutputZonesBrightness(float brightness);
    
    bool update() override;
    
    GuideImage* getGuideImageForUiElement(ShapeTargetCanvas& canvasTarget, MoveablePoly* poly) {
        for(int i = 0; i<canvasTarget.guideImages.size(); i++) {
            string uid = "guide"+ofToString(i);
            if(poly->getUid()==uid) {
                return &canvasTarget.guideImages.at(i);
            }
        }
        return nullptr;
    }
    
    GuideImageUiQuad* getUiElementForGuideImageIndex(int guideIndex) {
        string uid = "guide"+ofToString(guideIndex);
        for(MoveablePoly * uiElement : uiElementsSorted) {
            if(uiElement->getUid() == uid) {
                return dynamic_cast<GuideImageUiQuad*>(uiElement);
            }
        }
        return nullptr;
    }
    
    virtual void drawMoveables() override;
    
    bool zonesChangedFlag = false; 
    
    
};
}

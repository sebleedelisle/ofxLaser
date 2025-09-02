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
#include "SebUtils.h"

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
    bool updateZonesFromUI(std::shared_ptr<ShapeTargetCanvas> canvasTarget);
    void updateUIFromZones(std::shared_ptr<ShapeTargetCanvas> canvasTarget);
    
    virtual void setOutputRect(ofRectangle rect, bool updatescaleandoffset = false) override;
    
    void setOutputZonesBrightness(float brightness);
    
    bool update() override;
    
    std::shared_ptr<GuideImage> getGuideImageForUiElement(std::shared_ptr<ShapeTargetCanvas> canvasTarget, MoveablePoly* poly) {
        for(int i = 0; i<canvasTarget->guideImages.size(); i++) {
            string uid = "guide"+ofToString(i);
            if(poly->getUid()==uid) {
                return canvasTarget->guideImages.at(i);
            }
        }
        return nullptr;
    }
    
    std::shared_ptr<GuideImageUiQuad> getUiElementForGuideImageIndex(int guideIndex) {
        string uid = "guide"+ofToString(guideIndex);
        for(std::shared_ptr<MoveablePoly>& uiElement : uiElementsSorted) {
            if(uiElement->getUid() == uid) {
                return std::dynamic_pointer_cast<GuideImageUiQuad>(uiElement);
            }
        }
        return nullptr;
    }
    
    virtual void drawMoveables() override;
    
    bool zonesChangedFlag = false; 
    
    
};
}

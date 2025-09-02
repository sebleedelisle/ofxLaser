//
//  ofxLaserMaskUiQuad.h
//
//
//  Created by Seb Lee-Delisle on 07/02/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserQuadMask.h"

namespace ofxLaser {

class MaskUiQuad : public MoveablePoly {
    
    public :
    
    MaskUiQuad() {
        setHue(0);
        setBrightness(180);
        
    }
    
    void updateDataFromUI(std::shared_ptr<QuadMask>& mask) {
        //bool changed = false;
        //if(maskLevel!=mask->maskLevel){
            mask->maskLevel = maskLevel;
        //    changed = true;
        //}
        mask->setFromPoints(getPoints());
        
        
    }
    
    bool updateFromData(std::shared_ptr<QuadMask>& mask){
        bool changed = false;
        
       
        
        if(maskLevel!=mask->maskLevel){
            maskLevel = mask->maskLevel;
            
            changed = true;
        }
        std::vector<glm::vec2>* basePtr = mask.get();
        changed |= setFromPoints(basePtr);
        
        if(maskLevel==100) {
            setHue(0);
            setBrightness(180);
        } else {
            setHue(30);
            setBrightness(180);
        }
        
        return changed;
        
        
    }

  //  bool showContextMenu = false; // bit of a hack to show the right click menu
    int maskLevel = 100;
    
    
};
}


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
    
    void updateDataFromUI(QuadMask* mask) {
        //bool changed = false;
        //if(maskLevel!=mask->maskLevel){
            mask->maskLevel = maskLevel;
        //    changed = true;
        //}
        mask->setFromPoints(getPoints());
        
        
    }
    
    bool updateFromData(QuadMask* mask){
        bool changed = false;
        
       
        
        if(maskLevel!=mask->maskLevel){
            maskLevel = mask->maskLevel;
            
            changed = true;
        }
        changed |= setFromPoints(dynamic_cast<vector<glm::vec2>*>(mask));
        
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


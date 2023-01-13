//
//  ofxLaserViewPort.h
//
//  Created by Seb Lee-Delisle on 13/01/2023.
//
//

#pragma once

#include "ofMain.h"

namespace ofxLaser {
class ViewPort {
    public :
    
    ViewPort() {
        scale =1;
        offset = glm::vec2(0,0);
    
    }
    
    // base class for anything that is a window that can be zoomed and scrolled
    
    glm::vec2 offset;
    float scale;
    
    
    
    
};
}

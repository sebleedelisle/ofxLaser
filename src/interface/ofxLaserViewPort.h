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
  
    // base class for anything that is a window that can be zoomed and scrolled
    
    ViewPort() {
        scale =1;
        offset = glm::vec2(0,0);
    }
    
    void zoom(glm::vec2 anchor, float zoomMultiplier);
    void setOffsetAndScale(glm::vec2 newoffset =glm::vec2(0,0), float newscale = 1);
    
    
    virtual void mouseMoved(ofMouseEventArgs &e){};
    virtual bool mousePressed(ofMouseEventArgs &e){return false;};
    virtual void mouseDragged(ofMouseEventArgs &e){};
    virtual void mouseReleased(ofMouseEventArgs &e){};
    
    //void startDrag(glm::vec2 p);
    //void stopDrag();
    
    glm::vec2 offset;
    float scale;
    ofRectangle sourceRect;
    ofRectangle outputRect;
    
};
}

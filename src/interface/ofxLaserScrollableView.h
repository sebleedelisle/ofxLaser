//
//  ofxLaserViewPort.h
//
//  Created by Seb Lee-Delisle on 13/01/2023.
//
//

#pragma once

#include "ofMain.h"

namespace ofxLaser {
class ScrollableView {
    public :
  
    // base class for anything that is a window that can be zoomed and scrolled
  
    // TODO add source and target rectangle
    
    ScrollableView() {
        scale =1;
        offset = glm::vec2(0,0);
        outputRect.set(0,0,800,800);
        sourceRect.set(0,0,800,800);
    }
    
    void zoom(glm::vec2 anchor, float zoomMultiplier);
    void setOffsetAndScale(glm::vec2 newoffset =glm::vec2(0,0), float newscale = 1);
    
    
    virtual void mouseMoved(ofMouseEventArgs &e);
    virtual bool mousePressed(ofMouseEventArgs &e);
    virtual void mouseDragged(ofMouseEventArgs &e);
    virtual void mouseReleased(ofMouseEventArgs &e);
    virtual void mouseScrolled(ofMouseEventArgs &e);
    
    void setSourceRect(ofRectangle rect);
    void setOutputRect(ofRectangle rect);
    void autoFitToOutput();
    
    
    bool hitTest(glm::vec2 screenpos); 
    
    
    
    glm::vec2 screenPosToLocalPos(glm::vec2 pos);
    ofMouseEventArgs screenPosToLocalPos(ofMouseEventArgs pos);

    bool startDrag(glm::vec2 mousepos);
    bool updateDrag(glm::vec2 mousepos);
    bool stopDrag();
    
    
    
    //void startDrag(glm::vec2 p);
    //void stopDrag();
    
    glm::vec2 offset;
    float scale;
    //glm::vec2 mousePosLocal;
    
    ofRectangle sourceRect;
    ofRectangle outputRect;
    
    bool isDragging = false;
    glm::vec2 dragOffset;
    
    float zoomSpeed = 0.02f; 
    
    
};
}

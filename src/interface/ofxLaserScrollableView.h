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
    
    ScrollableView() ;
    
    virtual bool update();
    virtual bool checkEdges();

    
    void zoom(glm::vec2 anchor, float zoomMultiplier);
    void setOffsetAndScale(glm::vec2 newoffset =glm::vec2(0,0), float newscale = 1);
    float getScale() { return scale; };
    glm::vec2 getOffset() { return offset; } ; 
    
    void beginViewPort(bool clearScreen = true);
    void drawFrame();
    void drawEdges(); 
    void endViewPort(bool dontDrawFbo = false);
    
    virtual void mouseMoved(ofMouseEventArgs &e);
    virtual bool mousePressed(ofMouseEventArgs &e);
    virtual bool mouseDoubleClicked(ofMouseEventArgs &e);
    virtual void mouseDragged(ofMouseEventArgs &e);
    virtual void mouseReleased(ofMouseEventArgs &e);
    virtual void mouseScrolled(ofMouseEventArgs &e);
    
    bool setIsVisible(bool visible);
    bool getIsVisible(); 
    
    void setSourceRect(ofRectangle rect);
    void setOutputRect(ofRectangle rect, bool updatescaleandoffset = false);
    ofRectangle getOutputRect();
    
    void autoFitToOutput();
    
    bool hitTest(glm::vec2 screenpos); 
    
    glm::vec2 screenPosToLocalPos(glm::vec2 pos);
    ofMouseEventArgs screenPosToLocalPos(ofMouseEventArgs pos);

    bool startDrag(glm::vec2 mousepos);
    bool updateDrag(glm::vec2 mousepos);
    bool stopDrag();
    bool cancelDrag();

    bool doesUseFbo();
    bool setUseFbo(bool state);
    ofFbo& getFbo(){ return fbo; };
    
    bool initialiseFbo();
    
    protected :
    bool isVisible;
    
    glm::vec2 offset;
    float scale;
    
    ofRectangle sourceRect;
    ofRectangle outputRect;
    ofRectangle boundingRect;
    
    float lastClickTime = 0;
    float doubleClickMaxInterval = 0.2f; 
    bool isDragging = false;
    glm::vec2 dragOffset;
    glm::vec2 dragStartPosition; 
    
    float zoomSpeed = 0.02f;
    
    ofFbo fbo;
    bool useFbo = false;
    
    
};
}

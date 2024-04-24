//
//  ofxLaserMoveablePoly.h
//
//
//  Created by Seb Lee-Delisle on 03/02/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserDragHandle.h"
#include "ofxLaserPolygonBase.h"
#ifdef USE_FONT_MANAGER
#include "ofxFontManager.h"
#endif
namespace ofxLaser {

class MoveablePoly {
    
    public :
    
    MoveablePoly(string _label = "");
    
    virtual bool update();
    virtual void draw();
    
    virtual void drawHandlesIfSelectedAndNotDisabled();
    virtual void drawLabel();
    
    glm::vec2 getCentre();
    
    void setScale(float _scale) ;
    
    bool setFromPoints(vector<glm::vec2>& points);
    bool setFromPoints(vector<glm::vec2>* points);
    
    bool setFromRect(ofRectangle rect);
    
    void setLabel(string newlabel);
    
    ofRectangle getBoundingBox(); 
    
    vector<glm::vec2*> getPoints(); 
  
    virtual void updatePoly();
    virtual bool hitTest(glm::vec2& p) ;
    virtual bool hitTest(float x, float y) ;
    virtual bool hitTest(ofVec3f p) {
        return hitTest(p.x, p.y);
    }
    virtual void startDraggingHandleByIndex(int index); 

    void resetColours(); 
    void setHue(int hue) ;
    void setBrightness(int brightness) ;
    void setSaturationFloat(float saturationmultiplier) ;
    void updateHandleColours();
    
    bool getSelected();
    bool setSelected(bool v);
    bool getDisabled();
    bool setDisabled(bool v);
    
    void setDirty(){
        isDirty = true;
    }
    
    string& getLabel() {
        return label;
    }
    string& getUid() {
        return uid;
    }
    bool setShowLabel(bool state) {
        if(state!=showLabel) {
            showLabel = state;
            return true;
        } else {
            return false;
        } 
    }
    
    bool isQuad();
    
    bool setGrid(bool snapstate, int gridsize);
    
    virtual void mouseMoved(ofMouseEventArgs &e);
    virtual bool mousePressed(ofMouseEventArgs &e);
    virtual void mouseDragged(ofMouseEventArgs &e);
    virtual void mouseReleased(ofMouseEventArgs &e);
    
    DragHandle* getMainDragHandle();
    int getMainDragHandleIndex();
    
    bool getRightClickPressed(bool reset = true);
    void setRightClickPressed(bool value = true);
    
    int mainDragHandleIndex = -1;
    
    ofColor fillColour;
    ofColor fillColourSelected;
    ofColor strokeColour;
    ofColor strokeColourSelected;
    ofColor handleColour;
    ofColor handleColourOver;
    
    vector<DragHandle> handles; // all handles for all points
    
    
    
    
    protected :
    
    virtual void drawShape();
    void setNumHandles(int numhandles); 
    
    glm::vec2 centre;
    
    PolygonBase outlinePoly;

    glm::vec2 mousePos;
    glm::vec2 dragOffset;
    
    float handleSize = 10; 
    
    bool selected;

    string label;
    string uid; // unique identifier to associate it with data objects
    
    string type = "default"; // a way to differentiate between different types of elements
    
     
    // only used to change the size of handles, all
    // other scale / offset stuff should happen higher up
    float scale = 1;
    bool isDirty = true;
    bool dimmed = false;
    bool showLabel = true;
    
    bool snapToGrid;
    int gridSize;
    
    bool isDragging = false;
    bool isDisabled = false;
    
    bool constrainedToSquare = false;
    
    bool rightClickPressed = false; // bit of a hack to show the right click menu
 
    
};
}


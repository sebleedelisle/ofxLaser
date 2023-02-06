//
//  ofxLaserMoveablePoly.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 03/02/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserDragHandle.h"
#include "ofxLaserPolygonBase.h"

namespace ofxLaser {

class MoveablePoly {
    
    public :
    
    MoveablePoly();
    
    virtual bool update();
    virtual void draw();
    
    virtual void drawHandlesIfSelectedAndNotDisabled();
    virtual void drawLabel();
    
    glm::vec2 getCentre();
    
    void setScale(float _scale) ;
    
    bool setFromPoints(vector<glm::vec2>& points);
    bool setFromPoints(vector<glm::vec2>* points);
    
    vector<glm::vec2*> getPoints(); 
  
    virtual void updateMeshAndPoly();
    virtual bool hitTest(glm::vec2& p) ;
    virtual bool hitTest(float x, float y) ;
    virtual bool hitTest(ofVec3f p) {
        return hitTest(p.x, p.y);
    }
    virtual void startDraggingHandleByIndex(int index); 

    void setHue(int hue) ;
    void updateHandleColours();
    
    bool getSelected();
    bool setSelected(bool v);
    bool getDisabled();
    bool setDisabled(bool v);
    
    bool isQuad();
    
    bool setGrid(bool snapstate, int gridsize);
    
    virtual void mouseMoved(ofMouseEventArgs &e);
    virtual bool mousePressed(ofMouseEventArgs &e);
    virtual void mouseDragged(ofMouseEventArgs &e);
    virtual void mouseReleased(ofMouseEventArgs &e);
    
    DragHandle* getMainDragHandle();
    int getMainDragHandleIndex();
    
    int mainDragHandleIndex = -1;
    
    ofColor fillColour;
    ofColor fillColourSelected;
    ofColor strokeColour;
    ofColor strokeColourSelected;
    ofColor handleColour;
    ofColor handleColourOver;
    
    vector<DragHandle> handles; // all handles for all points
    
    protected :
    
    void drawShape(); 
    
    glm::vec2 centre;
    
    PolygonBase poly;

    glm::vec2 mousePos;
    
    bool selected;

    // only used to change the size of handles, all
    // other scale / offset stuff should happen higher up
    float scale = 1;
    bool isDirty = true;
    
    bool snapToGrid;
    int gridSize;
    
    bool isDragging = false;
    bool isDisabled = false;
    
    bool constrainedToSquare = false; 
    
    
};
}


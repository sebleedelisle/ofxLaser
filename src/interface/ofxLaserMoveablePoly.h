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
  
    virtual void updateMeshAndPoly();
    virtual bool hitTest(ofPoint mousePoint) ;
    
    virtual void startDraggingHandleByIndex(int index); 

    void setHue(int hue) ;
    void updateHandleColours();
    
    bool getSelected();
    bool setSelected(bool v);
    bool getDisabled();
    bool setDisabled(bool v);
    
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
    glm::vec2 centre;
    
    ofMesh zoneMesh;
    ofPolyline zonePoly;
  
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
    
    
};
}


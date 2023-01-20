//
//  ofxLaserZoneUIBase.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#pragma once

#include "ofMain.h"

namespace ofxLaser {

class ZoneUIBase {
  
    public :
    
    void draw();
    
    virtual bool hitTest(ofPoint mousePoint) {return false;};
     
    bool getSelected();
    bool setSelected(bool v);
    
    void setEditable(bool warpvisible);
    void setVisible(bool warpvisible);
    void setScale(float _scale) ;
    void setOffset(ofPoint _offset);
    bool setGrid(bool snapstate, int gridsize);
    
    virtual void mouseMoved(ofMouseEventArgs &e){};
    virtual bool mousePressed(ofMouseEventArgs &e){return false;};
    virtual void mouseDragged(ofMouseEventArgs &e){};
    virtual void mouseReleased(ofMouseEventArgs &e){};
    
    
    ofColor uiZoneFillColour;
    ofColor uiZoneFillColourSelected;
    ofColor uiZoneStrokeColour;
    ofColor uiZoneStrokeColourSelected;
    ofColor uiZoneStrokeSubdivisionColour;
    ofColor uiZoneStrokeSubdivisionColourSelected;
    ofColor uiZoneHandleColour;
    ofColor uiZoneHandleColourOver;
    
    protected :
    bool selected;
    bool visible;
    bool editable;
    
    ofPoint offset;
    float scale = 1;
    bool isDirty = true;
    
    bool snapToGrid;
    int gridSize;
 
    glm::vec2 mousePos;
    
   
};
}

//
//  ofxLaserZoneUIBase.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserOutputZone.h"

namespace ofxLaser {

class ZoneUiBase {
  
    public :
    
    ZoneUiBase();
    virtual ~ZoneUiBase() = default; 
    virtual bool update();
    virtual void draw() = 0;
    
    virtual void drawHandlesIfSelected();
    virtual void drawLabel(); 
    
    virtual bool updateFromOutputZone(OutputZone* outputZone) = 0;
    
    virtual bool hitTest(ofPoint mousePoint) {return false;};
    
    bool getSelected();
    bool setSelected(bool v);
    
    void setLocked(bool _locked);
    bool getLocked();
//    void setVisible(bool warpvisible);
//    bool getVisible();
    void setScale(float _scale) ;
    
    virtual void updateMeshAndPoly() {} ;

    bool setGrid(bool snapstate, int gridsize);
    
    virtual void mouseMoved(ofMouseEventArgs &e);
    virtual bool mousePressed(ofMouseEventArgs &e){return false;};
    virtual void mouseDragged(ofMouseEventArgs &e){};
    virtual void mouseReleased(ofMouseEventArgs &e){};
    
    
    
    void setHue(int hue) ;
    void updateHandleColours();
    
    
    ofColor uiZoneFillColour;
    ofColor uiZoneFillColourSelected;
    ofColor uiZoneStrokeColour;
    ofColor uiZoneStrokeColourSelected;
    ofColor uiZoneStrokeSubdivisionColour;
    ofColor uiZoneStrokeSubdivisionColourSelected;
    ofColor uiZoneHandleColour;
    ofColor uiZoneHandleColourOver;
    
    int inputZoneIndex = -1;
    bool inputZoneAlt = false;
 
    vector<DragHandle> handles; // all handles for all points
  
    protected :
    bool selected;

    // only used to change the size of handles, all
    // other scale / offset stuff should happen higher up
    float scale = 1;
    bool isDirty = true;
    
    bool snapToGrid;
    int gridSize;
    
     glm::vec2 centre;
    
    ofMesh zoneMesh;
    ofPolyline zonePoly;
  
    glm::vec2 mousePos;
    
    bool locked;
    
   
};
}

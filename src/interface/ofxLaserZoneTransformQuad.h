//
//  ofxLaserZoneTransformQuad.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#pragma once

#include "ofxLaserDragHandle.h"

namespace ofxLaser {
	
class ZoneTransformQuad {
	
	public :
	
	ZoneTransformQuad();
	~ZoneTransformQuad();
    
    virtual bool update();
    virtual void draw();
    
	virtual void init(ofRectangle& srcRect);

    virtual void setHue(int hue);
    virtual bool setGrid(bool snapstate, int gridsize);
    
    bool hitTest(ofPoint mousePoint) ;
     
    virtual bool getSelected();
    virtual bool setSelected(bool v);
    
    void setEditable(bool warpvisible);
    void setVisible(bool warpvisible);
	
	protected :
    
    // TO FIX!!!
    bool isCorner(int i) {
        return true;
    }
    
    void initListeners();
    void removeListeners();
    
    void mouseMoved(ofMouseEventArgs &e);
    bool mousePressed(ofMouseEventArgs &e);
    void mouseDragged(ofMouseEventArgs &e);
    void mouseReleased(ofMouseEventArgs &e);
    void paramChanged(ofAbstractParameter& e);
    
    // scale and offset are only for the visual interface
    void setScale(float _scale) ;
    void setOffset(ofPoint _offset);
    
    void updateHandleColours();
    
    void setDivisions(int xdivisions, int ydivisions);
    void updateDivisions();

    void updateQuads();
    
    void divisionsChanged(int& e);
        
    vector<DragHandle> dstHandles; // all handles for all points

    // used to record hover position
    ofPoint mousePos;
	
    ofColor uiZoneFillColour;
    ofColor uiZoneFillColourSelected;
    ofColor uiZoneStrokeColour;
    ofColor uiZoneStrokeColourSelected;
    ofColor uiZoneStrokeSubdivisionColour;
    ofColor uiZoneStrokeSubdivisionColourSelected;
    ofColor uiZoneHandleColour;
    ofColor uiZoneHandleColourOver;
    
    bool selected; // highlighted and ready to edit
    bool editable;  // visible and editable
    bool visible;
  
    bool snapToGrid;
    int gridSize;
    
    
    ofPoint offset;
    float scale = 1;
    bool isDirty = true; 
    
    
	
};
	
}

//
//  ofxLaserViewWithMovables.h
//
//  Created by Seb Lee-Delisle on 11/02/2023.
//
//

#pragma once

#include "ofxLaserScrollableView.h"
#include "ofxLaserMoveablePoly.h"

namespace ofxLaser {
class ViewWithMoveables : public ScrollableView  {
    
    public :
    ViewWithMoveables();
    
    bool update() override;
    
    virtual void draw();
    void drawMoveables(); 
    
    bool setUiElementsEnabled(bool enabled);
    
    virtual void deselectAllButThis(MoveablePoly* uielement);
    virtual void deselectAll(); 

    virtual void setGrid(bool snaptogrid, int gridsize);
    virtual void updateGrid();
    
    virtual void mouseMoved(ofMouseEventArgs &e) override;
    virtual bool mousePressed(ofMouseEventArgs &e) override;
    virtual void mouseDragged(ofMouseEventArgs &e) override;
    virtual void mouseReleased(ofMouseEventArgs &e) override;
    
    virtual void setLockedAll(bool lockstate);
    
    virtual MoveablePoly* getUiElementByUid(string _uid); 
    
   // vector<MoveablePoly*> uiElements;
    // This is used for the visual order of the elements.
    vector<MoveablePoly*> uiElementsSorted;
    
    protected :
    void drawGrid();
    
    ofMesh gridMesh;
    int gridSize = 1;
    bool snapToGrid = false;
    
    bool uiElementsEnabled;
    
    
    
    
};
}

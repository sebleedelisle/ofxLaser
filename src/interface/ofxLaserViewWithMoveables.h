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
    virtual void drawMoveables();
    
    bool setUiElementsEnabled(bool enabled);
    
    virtual void deselectAllButThis(std::shared_ptr<MoveablePoly>& uielement);
    virtual void deselectAll(); 

    virtual void setGrid(bool snaptogrid, int gridsize, bool visible);
    virtual void updateGrid();
    
    virtual void mouseMoved(ofMouseEventArgs &e) override;
    virtual bool mousePressed(ofMouseEventArgs &e) override;
    virtual void mouseDragged(ofMouseEventArgs &e) override;
    virtual void mouseReleased(ofMouseEventArgs &e) override;
    
    virtual bool keyPressed(ofKeyEventArgs &e) override; 
    
    virtual void setLockedAll(bool lockstate);
    
    // maybe a reference?
    virtual std::shared_ptr<MoveablePoly> getUiElementByUid(string _uid);
    
   // vector<MoveablePoly*> uiElements;
    // This is used for the visual order of the elements.
    vector<std::shared_ptr<MoveablePoly>> uiElementsSorted;
    
    protected :
    void drawGrid();
    
    ofMesh gridMesh;
    int gridSize = 1;
    bool snapToGrid = false;
    bool gridVisible = true; 
    
    bool uiElementsEnabled;
    
    
    
    
};
}

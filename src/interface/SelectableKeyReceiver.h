//
//  SelectableKeyReceiver.h
//  Liberation
//
//  Created by Seb Lee-Delisle on 16/07/2024.
//

//
// Base class for a ViewController with a distinct area
// When clicked on, it becomes selected. When selected it will
// receive key presses.
//
// Mouse and key presses should be handled at the ofApp level
//
#pragma once
#include "ofMain.h"

class SelectableKeyReceiver {
    
    
    public :
    SelectableKeyReceiver() {
        
        
    }
    ~SelectableKeyReceiver() {
    
        
    }
    
    bool selectIfHit(glm::vec2 pos) {
        if(enabled && hitTest(pos)) {
            setSelected(true);
        } else {
            setSelected(false);
        }
        return selected;
        
    }
    
    bool setSelected(bool state) {
        if(state == selected) return false;
        else {
            selected = state;
            return true;
        }
    }
    bool setEnabled(bool state) {
        if(state == enabled) return false;
        else {
            enabled = state;
            return true;
        }
    }
    bool getSelected() {
        return selected;
    }
    bool getEnabled() {
        return enabled;
    }
    

    void setClickArea(ofRectangle rect) {
        clickArea = rect;
    }
    void setClickArea(int x, int y, int w, int h) {
        clickArea.set(x, y, w,h);
    }
    ofRectangle& getClickArea() {
        return clickArea;
    }

    // returns true if it found a key and did something
    virtual bool keyPressed(ofKeyEventArgs & e){
        return false;
    }
    
    bool hitTest(glm::vec2 pos) {
        return clickArea.inside(pos);
    }
    
    protected :
    bool enabled = true;
    bool selected = false;
    
    ofRectangle clickArea;
    
};


//
//  ofxLaserZoneUIBase.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUIBase.h"
using namespace ofxLaser;


ZoneUiBase :: ZoneUiBase() {
    locked = false;
    selected = false;
    snapToGrid = false;
    gridSize = 10;
    mousePos = {0,0};
    
    
    uiZoneFillColour  = ofColor::fromHex(0x001123, 128);
    uiZoneFillColourSelected = ofColor::fromHex(0x001123);
    uiZoneStrokeColour  = ofColor::fromHex(0x0E87E7);
    uiZoneStrokeColourSelected = ofColor::fromHex(0x0E87E7);
    uiZoneHandleColour = ofColor::fromHex(0x0E87E7);
    uiZoneHandleColourOver = ofColor :: fromHex(0xffffff);
    uiZoneStrokeSubdivisionColour = ofColor :: fromHex(0x00386D);;
    uiZoneStrokeSubdivisionColourSelected = ofColor :: fromHex(0x006ADB);
    
    
}
bool ZoneUiBase :: update() {
    
    if(isDirty) {
        updateMeshAndPoly();
    }
    bool wasDirty = isDirty;
    isDirty = false;
    return wasDirty;
}

void ZoneUiBase :: setScale(float _scale) {
    if(scale!=_scale) {
        scale = _scale;
        for(DragHandle& handle : handles) {
            handle.setSize(10/scale);
        }
    }
}
void ZoneUiBase :: drawHandlesIfSelected() {
    
    if(getSelected() && !locked) {
        for(DragHandle& handle : handles) {
            handle.draw(mousePos);
        }
    }
}
void ZoneUiBase :: drawLabel() {
    ofPushStyle();
   
    ofSetColor(uiZoneStrokeColour);
    string label = getLabel();
    if(getLocked()) label = label + " (locked)";
    ofDrawBitmapString(label, centre - glm::vec3(4.0f*label.size()/scale,-4.0f/scale, 0));
    
    ofPopStyle();
    
}

string ZoneUiBase::getLabel(){
    string label = ofToString(inputZoneIndex+1);
    if(inputZoneAlt) {
        label = label + " ALT";
    }
    return label;
    
}

bool ZoneUiBase::getSelected() {
    return selected;
    
};

bool ZoneUiBase::setSelected(bool v) {
    if(selected!=v)  {
        selected = v;
        
        return true;
    } else {
        return false;
    }
};

glm::vec2 ZoneUiBase :: getCentre() {
    return centre;
}

void ZoneUiBase::setLocked(bool _locked){
    locked = _locked;
}
bool ZoneUiBase::getLocked(){
    return locked;
}

//
//void ZoneUiBase :: setEditable(bool editable){
//    editable = editable;
//}
//void ZoneUiBase :: setVisible(bool warpvisible){
//    visible = warpvisible;
//}
//bool ZoneUiBase :: getVisible(){
//    return visible;
//}

bool ZoneUiBase :: setGrid(bool snapstate, int gridsize) {
    if((snapstate!=snapToGrid) || (gridSize!=gridsize)) {
        snapToGrid = snapstate;
        gridSize = gridsize;
        
        for(DragHandle& handle : handles) {
            handle.setGrid(snapstate, gridsize);
        }
        
        return true;
    } else {
        return false;
    }
}

void ZoneUiBase :: mouseMoved(ofMouseEventArgs &e){
    mousePos = e; 
}

void ZoneUiBase::setHue(int hue) {

    uiZoneFillColour.setHue(hue);
    uiZoneFillColourSelected.setHue(hue);
    uiZoneStrokeColour.setHue(hue);
    uiZoneStrokeColourSelected.setHue(hue);
    uiZoneHandleColour.setHue(hue);
    uiZoneHandleColourOver.setHue(hue);
    uiZoneStrokeSubdivisionColour.setHue(hue);
    uiZoneStrokeSubdivisionColourSelected.setHue(hue);

    updateHandleColours();
  
}

void ZoneUiBase ::updateHandleColours() {
    for(DragHandle& handle : handles) {
        
        handle.setColour(uiZoneHandleColour, uiZoneHandleColourOver);
        
    }
    
}

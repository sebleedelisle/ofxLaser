//
//  ofxLaserZoneTransformQuad.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransformBase.h"


using namespace ofxLaser;



ZoneTransformBase:: ZoneTransformBase(){
    
    scale = 1;
    offset.set(0,0);
    
    editable = true;
    isDirty = true;
    selected = false;
    visible = false;
    
    uiZoneFillColour  = ofColor::fromHex(0x001123, 128);
    uiZoneFillColourSelected = ofColor::fromHex(0x001123);
    uiZoneStrokeColour  = ofColor::fromHex(0x0E87E7);
    uiZoneStrokeColourSelected = ofColor::fromHex(0x0E87E7);
    uiZoneHandleColour = ofColor::fromHex(0x0E87E7);
    uiZoneHandleColourOver = ofColor :: fromHex(0xffffff);
    uiZoneStrokeSubdivisionColour = ofColor :: fromHex(0x00386D);;
    uiZoneStrokeSubdivisionColourSelected = ofColor :: fromHex(0x006ADB);;;
    snapToGrid = false;
    gridSize  = 0;
    
    transformParams.setName("ZoneTransformBaseParams");
    
    transformParams.add(locked.set("locked", false));
 
    
    
}
ZoneTransformBase:: ~ZoneTransformBase(){
    
    
}



void ZoneTransformBase:: setSrc(const ofRectangle& rect) {
    
    if(srcRect != rect) {
        //ofLogNotice("ZoneTransformBase:: setSrc ") << rect;
        srcRect = rect;
        isDirty = true;
    }
};

void ZoneTransformBase::setHue(int hue) {
    uiZoneFillColour.setHue(hue);
    uiZoneFillColourSelected.setHue(hue);
    uiZoneStrokeColour.setHue(hue);
    uiZoneStrokeColourSelected.setHue(hue);
    uiZoneHandleColour.setHue(hue);
    uiZoneHandleColourOver.setHue(hue);
    uiZoneStrokeSubdivisionColour.setHue(hue);
    uiZoneStrokeSubdivisionColourSelected.setHue(hue);
  
  
}



bool ZoneTransformBase :: setGrid(bool snapstate, int gridsize) {
    if((snapstate!=snapToGrid) || (gridSize!=gridsize)) {
        snapToGrid = snapstate;
        gridSize = gridsize;
        return true; 
    } else {
        return false;
    }
}
    
void ZoneTransformBase :: setScale(float _scale) {
    scale = _scale;
    
}
void ZoneTransformBase :: setOffset(ofPoint _offset) {
    offset = _offset;
}


bool ZoneTransformBase::getSelected() {
    return selected;
    
};

bool ZoneTransformBase::setSelected(bool v) {
    if(selected!=v)  {
        selected = v;
       
        return true;
    } else {
        return false;
    }
};



void ZoneTransformBase :: setEditable(bool warpvisible){
    editable = warpvisible;
}
void ZoneTransformBase :: setVisible(bool warpvisible){
    visible = warpvisible;
}

void ZoneTransformBase :: setDirty(bool state) {
    isDirty = state;
    
};

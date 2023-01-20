//
//  ofxLaserZoneUIBase.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUIBase.h"
using namespace ofxLaser;

void ZoneUIBase :: setScale(float _scale) {
    scale = _scale;
    
}
void ZoneUIBase :: setOffset(ofPoint _offset) {
    offset = _offset;
}


bool ZoneUIBase::getSelected() {
    return selected;
    
};

bool ZoneUIBase::setSelected(bool v) {
    if(selected!=v)  {
        selected = v;
        
        return true;
    } else {
        return false;
    }
};


void ZoneUIBase :: setEditable(bool warpvisible){
    editable = warpvisible;
}
void ZoneUIBase :: setVisible(bool warpvisible){
    visible = warpvisible;
}


bool ZoneUIBase :: setGrid(bool snapstate, int gridsize) {
    if((snapstate!=snapToGrid) || (gridSize!=gridsize)) {
        snapToGrid = snapstate;
        gridSize = gridsize;
        return true;
    } else {
        return false;
    }
}

//
//  ofxLaserBezierNode
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 15/12/2022.
//
//


#include "ofxLaserBezierNode.h"
using namespace ofxLaser;

void BezierNode :: reset(float x, float y){
    handles.resize(3);
    handles[0].set(x,y);
    handles[1].set(x-20,y);
    handles[2].set(x+20,y);
    handles[1].isCircular = handles[2].isCircular = true;
    mode = 0;
    start = end = false;
    
}

void BezierNode :: draw(glm::vec2 mousePos, float scale) {
    
    handles[0].draw(mousePos, scale);
    if((!start)&&(mode>0)) {
        
        ofDrawLine(handles[0], handles[1]);
        handles[1].draw(mousePos, scale);
    }
    if((!end)&&(mode>0)) {
        
        ofDrawLine(handles[0], handles[2]);
        handles[2].draw(mousePos, scale);
    }
    
    
}

bool BezierNode :: hitTest(glm::vec2 mousePos, float scale) {
    if(mode ==0) {
        return handles[0].hitTest(mousePos, scale);
    } else {
        for(DragHandle& handle :handles) {
            if(handle.hitTest(mousePos, scale)) {
                return true;
            }
        }
        return false;
    }
}

bool BezierNode :: startDrag(glm::vec2 mousePos, float scale) {
    // if centre one hit, then drag them all
    if(handles[0].hitTest(mousePos, scale)) {
        handles[0].startDrag(mousePos);
        handles[1].startDrag(mousePos, &handles[0]);
        handles[2].startDrag(mousePos, &handles[0]);
        
        
    } else if(handles[1].hitTest(mousePos, scale)) {
        handles[1].startDrag(mousePos);
        if(mode==2) handles[2].startDragSymmetrical(mousePos, handles[0], handles[1], false);
    } else if(handles[2].hitTest(mousePos, scale)) {
        handles[2].startDrag(mousePos);
        if(mode==2) handles[1].startDragSymmetrical(mousePos, handles[0], handles[2], false);
    }
    
    return true;
    
}

bool BezierNode :: startDragAll(glm::vec2 mousePos, DragHandle* relativeToHandle) {
    if(relativeToHandle==nullptr) relativeToHandle = &handles[0];
    
    for(DragHandle& handle :handles) {
        handle.startDrag(mousePos, relativeToHandle);
    }
    return true;
    
}


glm::vec2 BezierNode :: getPosition() {
    return handles[0];
}
glm::vec2 BezierNode :: getControlPoint1() {
    if(mode==0) {
        return getPosition();
    } else {
        return handles[1];
    }
}
glm::vec2 BezierNode :: getControlPoint2() {
    if(mode==0) {
        return getPosition();
    } else {
        return handles[2];
    }
}

void BezierNode :: setGrid(bool snaptogrid, int gridsize) {
    //for(DragHandle& handle : handles) {
    handles[0].setGrid(snaptogrid, gridsize);
    
    //}
}
void BezierNode :: setColour(ofColor colour1, ofColor colour2) {
    for(DragHandle& handle : handles) {
        handle.setColour(colour1, colour2);
        
    }
    
}

bool BezierNode :: updateDrag(glm::vec2 mousePoint) {
    bool isDragging = false;
    for(DragHandle& handle : handles) {
        if(handle.updateDrag(mousePoint)) {
            isDragging = true;
        }
        
    }
    return isDragging;
}
bool BezierNode :: stopDrag() {
    bool wasDragging = false;
    for(DragHandle& handle : handles) {
        if( handle.stopDrag()) {
            wasDragging = true;
        }
        
    }
    return wasDragging;
}

bool BezierNode :: serialize(ofJson& json) {
    
    json["node"] = {
        {"mode", mode},
        {"start", start},
        {"end", end},
        {"handles", {
            {handles[0].x, handles[0].y},
            {handles[1].x, handles[1].y},
            {handles[2].x, handles[2].y}
        }}
    };
    
    
    //cout << json.dump(3) << endl;
    
    return true;
}

bool BezierNode :: deserialize(ofJson& jsonGroup) {
    //cout << jsonGroup.dump(3) << endl;
    ofJson& json = jsonGroup["node"];
    mode = json["mode"];
    start = json["start"];
    end = json["end"];
    handles.resize(3);
    ofJson handlesjson = json["handles"];
    for(int i = 0; i<handlesjson.size(); i++) {
        handles[i].set(handlesjson[i][0], handlesjson[i][1]);
    }
    
    return true;
}

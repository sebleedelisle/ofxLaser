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

class BezierNode {
    
    public :
    BezierNode() {
        
        
    }
    ~BezierNode() {
        
        
    }
    void reset(float x, float y);
    
    void draw(glm::vec3 mousePos, float scale);
    
    bool hitTest(glm::vec3 mousePos, float scale) ;
    
    bool startDrag(glm::vec3 mousePos, float scale) ;
    
    bool startDragAll(glm::vec3 mousePos, DragHandle* relativeToHandle = nullptr);
    
    
    glm::vec3 getPosition() ;
    glm::vec3 getControlPoint1();
    glm::vec3 getControlPoint2();
    
    void setGrid(bool snaptogrid, int gridsize) ;
    void setColour(ofColor colour1, ofColor colour2) ;
    
    bool updateDrag(ofPoint mousePoint);
    bool stopDrag();
    
    bool serialize(ofJson& json);
    
    bool deserialize(ofJson& jsonGroup);
    
    vector<DragHandle> handles; // 0 is centre, 1 and 2 are controls
    int mode = 0; // 0 corner, 1 curve corner, 2 mirrored
    
    bool start, end; // denotes whether node is at the start or end of the spline
    
    
    
    
};
}

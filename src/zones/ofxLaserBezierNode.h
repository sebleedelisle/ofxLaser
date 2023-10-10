//
//  ofxLaserZoneTransformQuad.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#pragma once
#include "ofMain.h"


namespace ofxLaser {

class BezierNode {
    
    public :
    BezierNode() {
        
        
    }
    ~BezierNode() {
        
        
    }
    void reset(float x, float y);
    bool setFromAnchorAndControlPoints(glm::vec2 anchor, glm::vec2 cp1, glm::vec2 cp2);
    bool setControlPoints(glm::vec2 cp1, glm::vec2 cp2);
    
   // void draw(glm::vec2 mousePos, float scale);
    
    
    
    
//    bool hitTest(glm::vec2 mousePos, float scale) ;
//
//    bool startDrag(glm::vec2 mousePos, float scale) ;
//
//    bool startDragAll(glm::vec2 mousePos, DragHandle* relativeToHandle = nullptr);
    
    
    glm::vec2 getPosition() ;
    glm::vec2 getControlPoint1();
    glm::vec2 getControlPoint2();
    
//    void setGrid(bool snaptogrid, int gridsize) ;
//    void setColour(ofColor colour1, ofColor colour2) ;
//    
//    bool updateDrag(glm::vec2 mousePoint);
//    bool stopDrag();
//    
    void serialize(ofJson& json) const;
    bool deserialize(ofJson& jsonGroup);
    
    vector<glm::vec2> handles; // 0 is centre, 1 and 2 are controls
   //int mode = 0; // 0 corner, 1 curve corner, 2 mirrored
    
    bool start, end; // denotes whether node is at the start or end of the spline
    
    
    
    
};
}

//
//  ofxLaserPolygon.h
//
//
//  Created by Seb Lee-Delisle on 04/02/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserPoint.h"
#include "ofxLaserGeomUtils.h"

namespace ofxLaser{
class PolygonBase :public vector<glm::vec2> {
    public :
    
    //PolygonBase();
    void setRectangle(float x, float y, float w, float h);
    void setFromPoints(const vector<glm::vec2*>& newpoints);
    void setFromPoints(const vector<glm::vec2>& newpoints);
    
    virtual void serialize(ofJson&json) const;
    virtual bool deserialize(ofJson&jsonGroup);
    
    bool isAxisAligned(); 

    bool update();
    
    bool hitTest(glm::vec2& p);
    bool hitTest(float x, float y);
    bool hitTestEdges(glm::vec2& p, float lineWidth);
    bool hitTestEdges(float x, float y, float lineWidth);
   //void updatePolyline();
    void updateBounds(); 
    //ofPolyline poly;
    glm::vec2 boundsTopLeft;
    glm::vec2 boundsBottomRight;
    
    bool isDirty = true;
 
    
};
}

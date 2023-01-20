//
//  ofxLaserZoneUIQuad.h
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#pragma once

#include "ofxLaserZoneUIBase.h"
#include "ofxLaserDragHandle.h"

namespace ofxLaser {
class ZoneUIQuad :public ZoneUIBase {
    
    public :
    ZoneUIQuad(); 
    
    virtual bool hitTest(ofPoint mousePoint) override ;
    
    virtual void draw() ; 
    
    bool setCorners(const vector<glm::vec2*>& points);
    vector<DragHandle*> getCornersClockwise();
    bool isSquare(); 
    
    void pointsUpdated();
   
    
    virtual void mouseMoved(ofMouseEventArgs &e) override;
    virtual bool mousePressed(ofMouseEventArgs &e) override;
    virtual void mouseDragged(ofMouseEventArgs &e) override;
    virtual void mouseReleased(ofMouseEventArgs &e) override;

    
    
    vector<DragHandle> cornerHandles; // all handles for all points
    glm::vec2 centre; 
    
    ofMesh zoneMesh;
    ofPolyline zonePoly;
    
};
}

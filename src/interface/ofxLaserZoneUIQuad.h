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
class ZoneUiQuad :public ZoneUiBase {
    
    public :
    ZoneUiQuad(); 
    
    void draw()  override;
    bool update() override;
    bool setSelected(bool v);
    
    void drawHandlesIfSelected() override;

    bool updateDataFromUI(ZoneTransformBase* zonetransform) override;
    bool updateFromData(ZoneTransformBase* zonetransform) override;
    
    void updateMeshAndPoly() override;

    bool hitTest(ofPoint mousePoint) override ;

    bool setCorners(const vector<glm::vec2*>& points);
    vector<DragHandle*> getCornersClockwise();
    bool isSquare();
    DragHandle* getMainDragHandle();
    int getMainDragHandleIndexClockwise(); 

    bool mousePressed(ofMouseEventArgs &e) override;
    void mouseDragged(ofMouseEventArgs &e) override;
    void mouseReleased(ofMouseEventArgs &e) override;

    int mainDragHandleIndexClockwise = -1;
    bool constrainedToSquare = false;
    bool isDragging = false; 

    
};
}

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
    
    virtual void draw()  override;
    virtual bool update() override;
    virtual bool setSelected(bool v);
    bool updateFromOutputZone(OutputZone* outputZone) override; 
    
    virtual void updateMeshAndPoly() override;

    virtual bool hitTest(ofPoint mousePoint) override ;

    bool setCorners(const vector<glm::vec2*>& points);
    vector<DragHandle*> getCornersClockwise();
    bool isSquare(); 

   
    virtual bool mousePressed(ofMouseEventArgs &e) override;
    virtual void mouseDragged(ofMouseEventArgs &e) override;
    virtual void mouseReleased(ofMouseEventArgs &e) override;


    

    
};
}

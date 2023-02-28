//
//  ofxLaserShapeTargetCanvas.h
//
//  Created by Seb Lee-Delisle on 23/02/2023.
//
//

#pragma once

#include "ofxLaserShapeTarget.h"
#include "ofxLaserInputZone.h"
#include "ofxLaserZoneIdContainer.h"

namespace ofxLaser {
class ShapeTargetCanvas : public ShapeTarget, public ZoneIdContainer {
    public :
    
    //void clearZones();
    void addInputZone(float x, float y, float w, float h);
//    bool deleteInputZone(InputZone* zone);
//    bool deleteInputZone(int zoneindex);
//    void renumberZones(); 
    vector<Shape*>getShapesForZoneId(ZoneId& zoneid); 
    InputZone* getInputZoneForZoneId(ZoneId& zoneid);
    InputZone* getInputZoneForZoneIndex(int index);

    
    vector<InputZone*>& getInputZones(); 
    
    
};
}


//
//  ofxLaserShapeTargetCanvas.h
//
//  Created by Seb Lee-Delisle on 23/02/2023.
//
//

#pragma once

#include "ofxLaserShapeTarget.h"
#include "ofxLaserInputZone.h"

namespace ofxLaser {
class ShapeTargetCanvas : public ShapeTarget {
    public :
    
    void clearZones(); 
    void addInputZone(float x, float y, float w, float h);
    bool deleteInputZone(InputZone* zone);
    bool deleteInputZone(int zoneindex);
    void renumberZones(); 
    vector<Shape*>getShapesForZone(int zoneindex); 
    
    std::vector<InputZone*> zones;
    
    
    
    
};
}


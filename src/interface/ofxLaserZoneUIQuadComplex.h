//
//  ofxLaserZoneUIQuadComplex.h
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#pragma once

#include "ofxLaserZoneUIBase.h"
#include "ofxLaserZoneTransformQuadData.h"

namespace ofxLaser {
class ZoneUIQuadComplex : public ZoneUIBase {
    
    public :
    ZoneUIQuadComplex();
    
    void draw() override;
    
    bool updateDataFromUI(std::shared_ptr<OutputZone>& outputZone) override;
    bool updateFromData(std::shared_ptr<OutputZone>& outputZone) override;

    glm::vec2 getPointAtPosition(int x, int y) ;

    protected :

    int subdivisionsX, subdivisionsY;
    
    
};
}

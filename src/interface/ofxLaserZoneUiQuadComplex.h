//
//  ofxLaserZoneUiQuadComplex.h
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#pragma once

#include "ofxLaserZoneUiBase.h"
#include "ofxLaserZoneTransformQuadData.h"

namespace ofxLaser {
class ZoneUiQuadComplex : public ZoneUiBase {
    
    public :
    ZoneUiQuadComplex();
    
    void draw() override;
    
    bool updateDataFromUi(OutputZone* outputZone) override;
    bool updateFromData(OutputZone* outputZone) override;
    
    bool setCorners(const vector<glm::vec2*>& points);
    
    glm::vec2 getPointAtPosition(int x, int y) ;
    
   //ofPolyline perimeterPolyline;
    
    protected :
    
    
    int subdivisionsX, subdivisionsY;
   //virtual void drawShape() override;
    
    
};
}

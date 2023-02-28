//
//  BeamZoneContainer.h
//
//  Created by Seb Lee-Delisle on 27/02/2023.
//
//

#pragma once

#include "ofxLaserZoneIdContainer.h"
#include "ofxLaserShapeTargetBeamZone.h"

namespace ofxLaser {
class BeamZoneContainer : public ZoneIdContainer {
    
    public :
    
    ZoneId addBeamZone();
    ShapeTargetBeamZone* getBeamZoneForZoneId(ZoneId& zoneid);
    ShapeTargetBeamZone* getBeamZoneAtIndex(int index);
    void deleteShapes(); 

};

}

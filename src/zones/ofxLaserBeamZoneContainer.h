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
    
    virtual bool addZoneByJson(ofJson& json) override;
    int getNumBeamZones() {
        return getNumZoneIds();
    } 
    
    std::shared_ptr<ShapeTargetBeamZone> getBeamZoneForZoneId(ZoneId& zoneid);
    std::shared_ptr<ShapeTargetBeamZone> getBeamZoneAtIndex(int index);
    void deleteShapes(); 

};

}

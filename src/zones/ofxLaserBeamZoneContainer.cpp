//
//  BeamZoneContainer.cpp
//
//  Created by Seb Lee-Delisle on 27/02/2023.
//
//

#include "ofxLaserBeamZoneContainer.h"
using namespace ofxLaser;



bool BeamZoneContainer :: addZoneByJson(ofJson& json) {
    
    std::shared_ptr<ShapeTargetBeamZone> zoneIdObject = std::make_shared<ShapeTargetBeamZone>();
    if(zoneIdObject->deserialize(json)) {
        addZoneIdObject(zoneIdObject);
        return true;
    } else {
        return false;
    }
    
}


ZoneId BeamZoneContainer :: addBeamZone(){
    std::shared_ptr<ShapeTargetBeamZone> newbeamzone = std::make_shared<ShapeTargetBeamZone>();

   addZoneIdObject(newbeamzone);
   return newbeamzone->zoneId;
} 

std::shared_ptr<ShapeTargetBeamZone> BeamZoneContainer :: getBeamZoneForZoneId(ZoneId& zoneid) {
    
    std::shared_ptr<ObjectWithZoneId> zoneobject = getObjectForZoneId(zoneid);
    return std::dynamic_pointer_cast<ShapeTargetBeamZone>(zoneobject);
    
}

std::shared_ptr<ShapeTargetBeamZone> BeamZoneContainer :: getBeamZoneAtIndex(int index) {
    
    std::shared_ptr<ObjectWithZoneId> zoneobject = getObjectAtIndex(index);
    return std::dynamic_pointer_cast<ShapeTargetBeamZone>(zoneobject);
    
}

void BeamZoneContainer :: deleteShapes() {
    for(int i = 0; i<getNumZoneIds(); i++) {
        getBeamZoneAtIndex(i)->deleteShapes();
    }
    
}

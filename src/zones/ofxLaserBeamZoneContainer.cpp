//
//  BeamZoneContainer.cpp
//
//  Created by Seb Lee-Delisle on 27/02/2023.
//
//

#include "ofxLaserBeamZoneContainer.h"
using namespace ofxLaser;

ZoneId BeamZoneContainer :: addBeamZone(){
   ShapeTargetBeamZone* newbeamzone = new ShapeTargetBeamZone();
   addZoneIdObject(newbeamzone);
   return newbeamzone->zoneId;
} 

ShapeTargetBeamZone* BeamZoneContainer :: getBeamZoneForZoneId(ZoneId& zoneid) {
    
    ObjectWithZoneId* zoneobject = getObjectForZoneId(zoneid);
    return dynamic_cast<ShapeTargetBeamZone*>(zoneobject);
    
}

ShapeTargetBeamZone* BeamZoneContainer :: getBeamZoneAtIndex(int index) {
    
    ObjectWithZoneId* zoneobject = getObjectAtIndex(index);
    return dynamic_cast<ShapeTargetBeamZone*>(zoneobject);
    
}

void BeamZoneContainer :: deleteShapes() {
    for(int i = 0; i<getNumZoneIds(); i++) {
        getBeamZoneAtIndex(i)->deleteShapes();
    }
    
}

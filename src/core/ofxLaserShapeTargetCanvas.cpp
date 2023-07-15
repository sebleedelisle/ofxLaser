//
//  ofxLaserShapeTargetCanvas.cpp
//
//  Created by Seb Lee-Delisle on 23/02/2023.
//
//

#include "ofxLaserShapeTargetCanvas.h"

using namespace ofxLaser;

ShapeTargetCanvas :: ShapeTargetCanvas() {
    zoneGroup = 0;
    type = ZoneId::CANVAS;
} 
//void ShapeTargetCanvas :: clearZones() {
//    for(InputZone* zone : zones) {
//        delete zone;
//    }
//    zones.clear(); 
//}
//
ZoneId ShapeTargetCanvas :: addInputZone(float x, float y, float w, float h){
    InputZone* newcanvaszone = new InputZone( x, y, w, h);
    addZoneIdObject(newcanvaszone);
    return newcanvaszone->getZoneId();
}



bool ShapeTargetCanvas :: addZoneByJson(ofJson& json) {
    cout << json.dump(3) << endl;
    InputZone* zoneIdObject = new InputZone();
    if(zoneIdObject->deserialize(json)) {
        addZoneIdObject(zoneIdObject);
        return true;
    } else {
        delete zoneIdObject;
        return false;
    }
    
}


//bool ShapeTargetCanvas :: deleteInputZone(InputZone* zone){
//    vector<InputZone*>::iterator it = std::find(zones.begin(), zones.end(), zone);
//    if(it == zones.end()) return false;
//    
//    deleteInputZone(it-zones.begin());
//    
//    return true;
//}
//bool ShapeTargetCanvas :: deleteInputZone(int zoneindex){
//    if((zones.size()>zoneindex) && (zoneindex>0)) {
//        InputZone* zone = zones[zoneindex];
//        zones.erase(zones.begin() + zoneindex);
//        renumberZones();
//        
//        delete zone;
//        return true;
//    } else {
//        return false;
//    } 
//}
//
//void ShapeTargetCanvas :: renumberZones(){
//    for(int i = 0; i<zones.size(); i++ ) {
//        zones[i]->getZoneId().zoneIndex = i;
////        zones[i]->setIndex(i);
//    }
//    
//}



vector<Shape*> ShapeTargetCanvas :: getShapesForZoneId(ZoneId& zoneid){
    
    InputZone* inputZone = getInputZoneForZoneId(zoneid);
    
    vector<Shape*> newshapes;
    
    if(inputZone!=nullptr) {

        for(Shape* shape : shapes) { //size_t i= 0; i<shapes.size(); i++) {
            
            // if (zone should have shape) then
            if(shape->intersectsRect(inputZone->getRect())) {
                newshapes.push_back(shape);
            }
        }
    } else {
        ofLogError("ShapeTargetCanvas :: getShapesForZoneId - missing zone id");
    }
    return newshapes;
    
}

InputZone* ShapeTargetCanvas :: getInputZoneForZoneId(ZoneId& zoneid) {
    ObjectWithZoneId* zoneobject = getObjectForZoneId(zoneid);
    return dynamic_cast<InputZone*>(zoneobject);
     
}

InputZone* ShapeTargetCanvas :: getInputZoneForZoneIndex(int index) {
    ObjectWithZoneId* zoneobject = getObjectAtIndex(index);
    return dynamic_cast<InputZone*>(zoneobject);
     
}

//vector<InputZone*>& ShapeTargetCanvas :: getInputZones() {
//    return dynamic_cast<vector<InputZone*>>(zoneIdObjects);
//}
    
    

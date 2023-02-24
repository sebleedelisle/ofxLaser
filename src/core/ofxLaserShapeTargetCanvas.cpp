//
//  ofxLaserShapeTargetCanvas.cpp
//
//  Created by Seb Lee-Delisle on 23/02/2023.
//
//

#include "ofxLaserShapeTargetCanvas.h"

using namespace ofxLaser;


void ShapeTargetCanvas :: clearZones() {
    for(InputZone* zone : zones) {
        delete zone;
    }
    zones.clear(); 
}

void ShapeTargetCanvas :: addInputZone(float x, float y, float w, float h){
    zones.push_back(new InputZone( x, y, w, h));
    //zones.back()->loadSettings();
    renumberZones();
}
bool ShapeTargetCanvas :: deleteInputZone(InputZone* zone){
    vector<InputZone*>::iterator it = std::find(zones.begin(), zones.end(), zone);
    if(it == zones.end()) return false;
    
    deleteInputZone(it-zones.begin());
    
    return true;
}
bool ShapeTargetCanvas :: deleteInputZone(int zoneindex){
    
    InputZone* zone = zones[zoneindex];
    zones.erase(zones.begin() + zoneindex);
    renumberZones();
    
    delete zone;
}

void ShapeTargetCanvas :: renumberZones(){
    for(int i = 0; i<zones.size(); i++ ) {
        zones[i]->setIndex(i);
    }
    
}



vector<Shape*> ShapeTargetCanvas :: getShapesForZone(int zoneindex){
    
    InputZone*inputZone = zones[zoneindex];
    
    vector<Shape*> newshapes;
    for(Shape* shape : shapes) { //size_t i= 0; i<shapes.size(); i++) {
        
        // if (zone should have shape) then
        if(shape->intersectsRect(inputZone->getRect())) {
            newshapes.push_back(shape);
        }
    }
    return newshapes;
    
}

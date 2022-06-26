//
//  ofxLaserLaserZone.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 13/04/2021.
//

#include "ofxLaserLaserZone.h"
using namespace ofxLaser;

LaserZone :: LaserZone(Zone& _zone) : zone(_zone) {
    // init params?
    soloed = false;
    ZoneTransform :: init(zone.rect);
    
    // TODO make params object for serialisation / deserialisation?
    zoneParams.setName("laserZone"+ofToString(zone.getIndex()));
    zoneParams.add(muted.set("mute", false));
    zoneParams.add(soloed.set("solo", false));
    
    zoneMaskGroup.setName(getLabel());
    zoneMaskGroup.add(bottomEdge.set("Bottom", 0,0,1));
    zoneMaskGroup.add(topEdge.set("Top", 0,0,1));
    zoneMaskGroup.add(leftEdge.set("Left", 0,0,1));
    zoneMaskGroup.add(rightEdge.set("Right", 0,0,1));
    ofAddListener(zoneMaskGroup.parameterChangedE(), this, &LaserZone::zoneMaskChanged);
    zoneParams.add(zoneMaskGroup);
    isDirty = true;
    enabled = true;
    visible = true;
    ofAddListener(zoneParams.parameterChangedE(), this, &LaserZone::paramChanged);
   
}
LaserZone :: ~LaserZone() {
    // Transform object should be automatically deleted i think
    // make sure destructor cleans up ok
    ofRemoveListener(zoneMaskGroup.parameterChangedE(), this, &LaserZone::zoneMaskChanged);
}

bool LaserZone :: update() {
    
    bool wasDirty = isDirty;
    wasDirty = ZoneTransform :: update() | wasDirty;
    isDirty = false;
    
    return wasDirty;
}

void LaserZone :: draw() {
     
    if(!visible) return ;
    ofPushStyle();
    //ofEnableAlphaBlending();
    string label =ofToString(zone.getIndex()+1);
    if(isAlternate) label += "ALT";
    ZoneTransform::draw(label);

   
    ofPushMatrix();
    
    ofTranslate(offset);
    ofScale(scale, scale);
     
    // go through and draw blue rectangles around the warper
    ofPoint p;
   
   
    ofRectangle& mask = zoneMask;
    
    
//
//    ofFill();
//    ofSetColor(0,0,255,30);
//
//    ofBeginShape();
//    p = zoneTransform.getWarpedPoint((ofPoint)mask.getTopLeft());
//    ofVertex(p);
//    p = zoneTransform.getWarpedPoint((ofPoint)mask.getTopRight());
//    ofVertex(p);
//    p = zoneTransform.getWarpedPoint((ofPoint)mask.getBottomRight());
//    ofVertex(p);
//    p = zoneTransform.getWarpedPoint((ofPoint)mask.getBottomLeft());
//    ofVertex(p);
//    ofEndShape(true);
    
    //ofDisableAlphaBlending();
    
    ofPopStyle();
    ofPopMatrix();

}

string LaserZone :: getLabel() {
    return ofToString(zone.getIndex()+1);
}
void LaserZone :: setScale(float _scale) {
    scale = _scale;
    //zoneTransform.scale = scale;
}
void LaserZone :: setOffset(ofPoint _offset) {
    offset = _offset;
    //zoneTransform.offset = offset;
}
void LaserZone :: zoneMaskChanged(ofAbstractParameter& e) {
    updateZoneMask();
}
void LaserZone :: updateZoneMask() {
    //zoneTransform.updateZoneMask();
    zoneMask.setX(zone.rect.getLeft()+(leftEdge*zone.rect.getWidth()));
    zoneMask.setY(zone.rect.getTop()+(topEdge*zone.rect.getHeight()));
    zoneMask.setWidth(zone.rect.getWidth()*(1-leftEdge-rightEdge));
    zoneMask.setHeight(zone.rect.getHeight()*(1-topEdge-bottomEdge));
    isDirty = true; 
}

void LaserZone :: paramChanged(ofAbstractParameter& e) {
    isDirty=true; 
}
bool LaserZone :: serialize(ofJson& json){
   
   // ofJson jsonGroup;
    
    // params contain muted, soloed and the mask edge group
    ofJson paramsJson;
    ofSerialize(paramsJson, zoneParams);
    json["zoneparams"] = paramsJson;
    ofJson zoneTransformJson;
    ZoneTransform :: serialize(zoneTransformJson);
    json["zonetransform"] = zoneTransformJson;
    // zoneTransform
    //json["laserzone"+ofToString(zone.index))] = jsonGroup);
    return true;
}


bool LaserZone :: deserialize(ofJson& json){
 
    // TODO Error check! Try / catch
    ofJson paramsJson = json["zoneparams"];
    ofDeserialize(paramsJson, zoneParams);
    ofJson zoneTransformJson = json["zonetransform"];
    ZoneTransform :: deserialize(zoneTransformJson);
    updateZoneMask();
    
    return true; 
    
}

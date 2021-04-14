//
//  ofxLaserProjectorZone.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 13/04/2021.
//

#include "ofxLaserProjectorZone.h"
using namespace ofxLaser;

ProjectorZone :: ProjectorZone(Zone& _zone) : zone(_zone) {
    // init params?
    soloed = false;
    zoneTransform.init(zone.rect);
    
    // TODO make params object for serialisation / deserialisation?
    params.setName("projectorZone"+ofToString(zone.index));
    params.add(muted.set("mute", false));
    params.add(soloed.set("solo", false));
    
    zoneMaskGroup.setName(getLabel());
    zoneMaskGroup.add(bottomEdge.set("Bottom Edge", 0,0,1));
    zoneMaskGroup.add(topEdge.set("Top Edge", 0,0,1));
    zoneMaskGroup.add(leftEdge.set("Left Edge", 0,0,1));
    zoneMaskGroup.add(rightEdge.set("Right Edge", 0,0,1));
    ofAddListener(zoneMaskGroup.parameterChangedE(), this, &ProjectorZone::zoneMaskChanged);
    params.add(zoneMaskGroup); 
   
}
ProjectorZone :: ~ProjectorZone() {
    // Transform object should be automatically deleted i think
    // make sure destructor cleans up ok
    ofRemoveListener(zoneMaskGroup.parameterChangedE(), this, &ProjectorZone::zoneMaskChanged);
}

void ProjectorZone :: update() {
    zoneTransform.update();
   
}
void ProjectorZone :: setVisible(bool visible) {
    zoneTransform.setVisible(visible);
    
}
void ProjectorZone :: draw() {
     
    zoneTransform.draw(ofToString(zone.index+1));

    if(!enabled) return;
   
    ofPushMatrix();
    
    ofTranslate(offset);
    ofScale(scale, scale);
     
    // go through and draw blue rectangles around the warper
    ofPoint p;
   
   
    ofRectangle& mask = zoneMask;
    
    //ofPushMatrix();
    //ofTranslate();
    ofEnableAlphaBlending();
    ofFill();
    ofSetColor(0,0,255,30);
    
    ofBeginShape();
    p = zoneTransform.getWarpedPoint((ofPoint)mask.getTopLeft());
    ofVertex(p);
    p = zoneTransform.getWarpedPoint((ofPoint)mask.getTopRight());
    ofVertex(p);
    p = zoneTransform.getWarpedPoint((ofPoint)mask.getBottomRight());
    ofVertex(p);
    p = zoneTransform.getWarpedPoint((ofPoint)mask.getBottomLeft());
    ofVertex(p);
    ofEndShape(true);
    
    ofPopMatrix();

}

string ProjectorZone :: getLabel() {
    return ofToString(zone.index+1);
}
void ProjectorZone :: setScale(float _scale) {
    scale = _scale;
    zoneTransform.scale = scale;
}
void ProjectorZone :: setOffset(ofPoint _offset) {
    offset = _offset;
    zoneTransform.offset = offset;
}
void ProjectorZone :: zoneMaskChanged(ofAbstractParameter& e) {
    updateZoneMask();
}
void ProjectorZone :: updateZoneMask() {
    //zoneTransform.updateZoneMask();
    zoneMask.setX(zone.rect.getLeft()+(leftEdge*zone.rect.getWidth()));
    zoneMask.setY(zone.rect.getTop()+(topEdge*zone.rect.getHeight()));
    zoneMask.setWidth(zone.rect.getWidth()*(1-leftEdge-rightEdge));
    zoneMask.setHeight(zone.rect.getHeight()*(1-topEdge-bottomEdge));
}


bool ProjectorZone :: serialize(ofJson& json){
   
   // ofJson jsonGroup;
    
    // params contain muted, soloed and the mask edge group
    ofJson paramsJson;
    ofSerialize(paramsJson, params);
    json["params"] = paramsJson;
    ofJson zoneTransformJson;
    zoneTransform.serialize(zoneTransformJson);
    json["zonetransform"] = zoneTransformJson;
    // zoneTransform
    //json["projectorzone"+ofToString(zone.index))] = jsonGroup);
    return true;
}


bool ProjectorZone :: deserialize(ofJson& json){
 
    // TODO Error check! Try / catch
    ofJson paramsJson = json["params"];
    ofDeserialize(paramsJson, params);
    ofJson zoneTransformJson = json["zonetransform"];
    zoneTransform.deserialize(zoneTransformJson);
    
    return true; 
    
}

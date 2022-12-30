//
//  ofxLaserLaserZone.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 13/04/2021.
//

#include "ofxLaserOutputZone.h"
using namespace ofxLaser;

OutputZone :: OutputZone(InputZone& _zone) : zone(_zone) {
    
    // init params?
    soloed = false;
    //ofLogNotice("OutputZone() rect : ") << zone.getRect();
    ofRectangle rect = zone.getRect();
    zoneTransformQuad.init(rect);
    zoneTransformLine.init(rect);
    
    zoneParams.setName("OutputZone");
    zoneParams.add(muted.set("mute", false));
    zoneParams.add(soloed.set("solo", false));
    zoneParams.add(transformType.set("Transform type", 0,0,1));

    isDirty = true;
    enabled = true;
    visible = true;
    ofAddListener(zoneParams.parameterChangedE(), this, &OutputZone::paramChanged);
    isAlternate = false;
    
}
OutputZone :: ~OutputZone() {
    // Transform object should be automatically deleted i think
    // make sure destructor cleans up ok
}

bool OutputZone :: update() {
    
    bool wasDirty = isDirty;
    wasDirty = getZoneTransform().update() | wasDirty;
   // if(isDirty) {
        // bit hacky, need to ensure all transforms are selected / deselected together
        zoneTransformQuad.setSelected(getZoneTransform().getSelected());
        zoneTransformLine.setSelected(getZoneTransform().getSelected());
   // }
    
    zoneTransformQuad.setSrc(zone.getRect());
    zoneTransformLine.setSrc(zone.getRect());
    
    zoneTransformQuad.setVisible(transformType==0);
    zoneTransformLine.setVisible(transformType==1);

    isDirty = false;
    
    return wasDirty;
}

bool OutputZone :: setGrid(bool snapstate, int gridsize) {
    if((snapstate!=snapToGrid) || (gridSize!=gridsize)) {
        snapToGrid = snapstate;
        gridSize = gridsize;
        
        zoneTransformLine.setGrid(snapToGrid, gridSize);
        zoneTransformQuad.setGrid(snapToGrid, gridSize);
        return true; 
    } else {
        return false;
    }
}

void OutputZone :: draw() {
     
    if(!visible) return ;
    ofPushStyle();
    ofEnableAlphaBlending();
    string label =ofToString(zone.getIndex()+1);
    if(getIsAlternate()) label += "ALT";
    getZoneTransform().draw(label);

    ofDisableAlphaBlending();
    
    ofPopStyle();


}

string OutputZone :: getLabel() {
    return ofToString(zone.getIndex()+1);
}
void OutputZone :: setScale(float _scale) {
    scale = _scale;
    
    getZoneTransform().setScale(scale);
    //getZoneTransform().scale = scale;
}
void OutputZone :: setOffset(ofPoint _offset) {
    offset = _offset;
    getZoneTransform().setOffset(offset);
}


void OutputZone :: paramChanged(ofAbstractParameter& e) {
    isDirty=true; 
}


bool OutputZone::getEnabled() {
    return enabled;
}
void OutputZone::setEnabled(bool value) {
    enabled = value;
    getZoneTransform().setEditable(enabled);
}
void OutputZone::setVisible(bool value) {
    visible = value;
    getZoneTransform().setVisible(visible);
}
bool OutputZone::getVisible() {
    return visible;
}

const int OutputZone::getZoneIndex() const {
    return zone.getIndex();
};


bool OutputZone::getSelected() {
    return getZoneTransform().getSelected();
    
};

void OutputZone::setSelected(bool v) {

   getZoneTransform().setSelected(v);
    
};


ofxLaser::Point OutputZone::getWarpedPoint(const ofxLaser::Point& p){
    return getZoneTransform().getWarpedPoint(p);
    
}
//ofxLaser::Point OutputZone::getUnWarpedPoint(const ofxLaser::Point& p){
//    return getZoneTransform().getUnWarpedPoint(p);
//
//}
ofPoint OutputZone::getWarpedPoint(const ofPoint& p){
    return getZoneTransform().getWarpedPoint(p);
    
}
ofPoint OutputZone::getUnWarpedPoint(const ofPoint& p){
    return getZoneTransform().getUnWarpedPoint(p);
    
}

bool OutputZone::getIsAlternate() {
    return isAlternate;
}
void OutputZone::setIsAlternate(bool v){
    isAlternate = v;
    if(isAlternate) {
        zoneTransformLine.setHue(100);
        zoneTransformQuad.setHue(100);
    }
}

void OutputZone ::setSourceRect(ofRectangle & rect) {
    
    zoneTransformQuad.setSrc(rect);
    zoneTransformLine.setSrc(rect);
}

void OutputZone :: init(ofRectangle sourceRectangle) {
    zoneTransformQuad.init(sourceRectangle);
    zoneTransformLine.init(sourceRectangle);
}

ofRectangle OutputZone :: getBounds() {
    
    vector<glm::vec3> perimeterpoints;
    ofRectangle bounds;
    getZoneTransform().getPerimeterPoints(perimeterpoints);
    bounds.setPosition(*perimeterpoints.begin());
    
    for(glm::vec3& p:perimeterpoints) {
        bounds.growToInclude(p);
    }
    
    return bounds;
    
}

void OutputZone :: drawPerimeterAsShape() {
    
    vector<glm::vec3> perimeterpoints;
    getZoneTransform().getPerimeterPoints(perimeterpoints);
    
    ofBeginShape();
    for(glm::vec3& p:perimeterpoints) {
        ofVertex(p);
    }
    ofEndShape();

}




bool OutputZone :: serialize(ofJson& json){
   
    // params contain muted, soloed
    ofJson paramsJson;
    ofSerialize(paramsJson, zoneParams);
    json["zoneparams"] = paramsJson;
    ofJson zoneTransformQuadJson;
    zoneTransformQuad.serialize(zoneTransformQuadJson);
    json["zonetransformquad"] = zoneTransformQuadJson;

    ofJson zoneTransformLineJson;
    zoneTransformLine.serialize(zoneTransformLineJson);
    json["zonetransformline"] = zoneTransformLineJson;

    return true;
}


bool OutputZone :: deserialize(ofJson& json){
 
    if(json.contains("zoneparams")) {
        ofJson paramsJson = json["zoneparams"];
       // because there is a random name in here
        ofJson fixedJson;
        fixedJson[zoneParams.getName()] = *paramsJson.begin();
       // ofLogNotice() << paramsJson.dump(3);
        //ofJson& paramsObjectJson = *paramsJson.begin();
        
        ofDeserialize(fixedJson, zoneParams);
    }
    if(json.contains("zonetransformquad")) {
        ofJson zoneTransformQuadJson = json["zonetransformquad"];
        zoneTransformQuad.deserialize(zoneTransformQuadJson);
    }
    if(json.contains("zonetransformline")) {
        ofJson zoneTransformLineJson = json["zonetransformline"];
        zoneTransformLine.deserialize(zoneTransformLineJson);
    }
    
    // deprecated, can delete eventually
    if(json.contains("zonetransform")) {
        ofJson zoneTransformJson = json["zonetransform"];
        zoneTransformQuad.deserialize(zoneTransformJson);
    }
          
    return true; 
    
}

ZoneTransformBase& OutputZone :: getZoneTransform(){
    if(transformType==1) {
        return zoneTransformLine;
    } else {
        return zoneTransformQuad;
    }
}

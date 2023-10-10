//
//  ofxLaserLaserZone.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 13/04/2021.
//

#include "ofxLaserOutputZone.h"
using namespace ofxLaser;

OutputZone :: OutputZone(ZoneId zoneid, ofRectangle sourcerect ) {
    
    //ofLogNotice("OutputZone :: OutputZone()");
    
    // init params?
    soloed = false;

//    sourceRect =sourcerect;
    zoneTransformQuad.init();
    zoneTransformLine.init();
    zoneTransformQuadComplex.init();
    zoneId = zoneid;
    
    zoneParams.setName("OutputZone");
    zoneParams.add(muted.set("mute", false));
    zoneParams.add(soloed.set("solo", false));
    zoneParams.add(transformType.set("Transform type", 0,0,1));

    isDirty = true;
    enabled = true;
    
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

    
    //zoneTransformQuad.updateSrc(zone.getRect());
    //zoneTransformLine.updateSrc(zone.getRect());

    isDirty = false;
    
    return wasDirty;
}


string OutputZone :: getLabel() {
    return zoneId.getLabel() + (getIsAlternate()?" ALT":"");
}


void OutputZone :: paramChanged(ofAbstractParameter& e) {
    isDirty=true; 
}

//
//bool OutputZone::getEnabled() {
//    return enabled;
//}
//void OutputZone::setEnabled(bool value) {
//    enabled = value;
//}


ZoneId OutputZone::getZoneId() const {
    return zoneId;
};
bool OutputZone::setZoneId(ZoneId& zoneid) {
    if(zoneId!=zoneid) {
        zoneId = zoneid;
        return true;
    } else {
        return false;
    }
    return false; 
}

ofxLaser::Point OutputZone::getWarpedPoint(const ofxLaser::Point& p){
    return getZoneTransform().getWarpedPoint(p);
    
}

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
   
}

bool OutputZone ::setSourceRect(const ofRectangle & rect) {
    if(zoneTransformQuad.srcRect!=rect) {
        //sourceRect = rect;
        zoneTransformQuad.updateSrc(rect);
        zoneTransformQuadComplex.updateSrc(rect);
        zoneTransformLine.updateSrc(rect);
        return true;
    } else {
        return false;
    }
}

//ofRectangle OutputZone :: getSourceRect() {
//    return sourceRect;
//}


ofRectangle OutputZone :: getBounds() {
    
    vector<glm::vec2> perimeterpoints;
    ofRectangle bounds;
    getZoneTransform().getPerimeterPoints(perimeterpoints);
    glm::vec3 start(*perimeterpoints.begin(), 0);
    bounds.setPosition(start);
    
    for(glm::vec2& p:perimeterpoints) {
        bounds.growToInclude(p);
    }
    
    return bounds;
    
}

void OutputZone :: drawPerimeterAsShape() {
    
    vector<glm::vec2> perimeterpoints;
    getZoneTransform().getPerimeterPoints(perimeterpoints);
    
    ofBeginShape();
    for(glm::vec2& p:perimeterpoints) {
        ofVertex(p);
    }
    ofEndShape();

}




void OutputZone :: serialize(ofJson& json) const{
   
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
    zoneId.serialize(json);
    
    ofJson zoneTransformQuadComplexJson;
    zoneTransformQuadComplex.serialize(zoneTransformQuadComplexJson);
    json["zonetransformquadcomplex"] = zoneTransformQuadComplexJson;


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
    if(json.contains("zonetransformquadcomplex")) {
        ofJson zoneTransformQuadComplexJson = json["zonetransformquadcomplex"];
        zoneTransformQuadComplex.deserialize(zoneTransformQuadComplexJson);
    }
    if(json.contains("zonetransformline")) {
        ofJson zoneTransformLineJson = json["zonetransformline"];
        zoneTransformLine.deserialize(zoneTransformLineJson);
    }
    
    // deprecated, can delete eventually
//    if(json.contains("zonetransform")) {
//        ofJson zoneTransformJson = json["zonetransform"];
//        zoneTransformQuad.deserialize(zoneTransformJson);
//    }
    zoneId.deserialize(json); 
    return true; 
    
}

ZoneTransformBase& OutputZone :: getZoneTransform(){
    if(transformType==1) {
        return zoneTransformLine;
    } else if(transformType==2) {
        return zoneTransformQuadComplex;
    } else {
        return zoneTransformQuad;
    }
}

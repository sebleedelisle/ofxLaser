//
//  ofxLaserVisualiser3DPreset.cpp
//
//  Created by Seb Lee-Delisle on 18/06/2022.
//

#include "ofxLaserVisualiser3DLaserSettings.h"



using namespace ofxLaser;

Visualiser3DLaserSettings :: Visualiser3DLaserSettings(){
    params.add(label.set("label", "Default"));
    
    
    params.add(showCanvas.set("Show canvas", false));
    params.add(canvasPos.set("Canvas position", {0,-52,0}, {-1000,-1000,-1000}, {1000,1000,1000}));
    params.add(canvasRotation.set("Canvas rotation", {0,0,0}, {-180,-180,-180}, {180,180,180}));
    params.add(canvasScale.set("Canvas scale", {0.5,0.5}, {0.01,0.01}, {10,10}));
//    ofParameter<glm::vec3> canvasPos;
//    ofParameter<glm::vec3> canvasRotation;
//    ofParameter<glm::vec2> canvasScale;
    
    
}

Visualiser3DLaserSettings& Visualiser3DLaserSettings :: operator=( Visualiser3DLaserSettings& that){
    ofJson json;
    that.serialize(json);
    deserialize(json);
    return *this;
    
}
bool Visualiser3DLaserSettings :: operator == (Visualiser3DLaserSettings& that){
    ofJson json1;
    that.serialize(json1);
    json1["label"] = "";
    //string jsonstring = json1.dump();
    ofJson json2;
    serialize(json2);
    json2["label"] = "";
    
    return json1.dump() == json2.dump();
}
bool Visualiser3DLaserSettings :: operator != (Visualiser3DLaserSettings& that){
    return !(*this==that);
}
void Visualiser3DLaserSettings :: serialize(ofJson&json) const{
    ofSerialize(json, params);
    ofJson& jsonGroup = json["lasers"];
    for(const Laser3DVisualObject& laser : laserObjects ){
        ofJson laserjson;
        laser.serialize(laserjson);
        jsonGroup.push_back(laserjson);
    }
    
   
    
   // ofLogNotice("Visualiser 3D laser json : ") << jsonGroup.dump(3);
    
}
bool Visualiser3DLaserSettings :: deserialize(ofJson&json){
    //ofLogNotice("Visualiser laser json : ") << json.dump(3);
    ofDeserialize(json, params);
    
    ofJson& jsonLaserObjects = json["lasers"];
    laserObjects.resize(jsonLaserObjects.size());
    for(size_t i = 0; i<jsonLaserObjects.size(); i++)  {
        laserObjects[i].deserialize(jsonLaserObjects[i]);
    }
    
    return true;
}

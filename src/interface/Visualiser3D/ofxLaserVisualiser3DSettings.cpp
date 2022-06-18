//
//  ofxLaserVisualiser3DPreset.cpp
//
//  Created by Seb Lee-Delisle on 18/06/2022.
//

#include "ofxLaserVisualiser3DSettings.h"



using namespace ofxLaser;

Visualiser3DSettings :: Visualiser3DSettings(){
    
    
    params.add(brightness.set("Brightness adjustment", 1,0.1,10));
    params.add(cameraDistance.set("Camera distance", 50,0,1000));
    params.add(cameraFov.set("Camera FOV", 45,10,120));
    
    params.add(cameraOrbit.set("Camera orbit", glm::vec3(0,0,0), glm::vec3(-90,-90,0), glm::vec3(90,90,0)));
    params.add(cameraOrientation.set("Camera orientation", glm::vec3(0,0,0), glm::vec3(-180,-90,0), glm::vec3(180,90,0) ));
    
}

Visualiser3DSettings& Visualiser3DSettings :: operator=( Visualiser3DSettings& that){
    ofJson json;
    that.serialize(json);
    deserialize(json);
    return *this;
    
}
bool Visualiser3DSettings :: operator == (Visualiser3DSettings& that){
    ofJson json1;
    that.serialize(json1);
    json1["label"] = "";
    //string jsonstring = json1.dump();
    ofJson json2;
    serialize(json2);
    json2["label"] = "";
    
    return json1.dump() == json2.dump();
}
bool Visualiser3DSettings :: operator != (Visualiser3DSettings& that){
    return !(*this==that);
}
void Visualiser3DSettings :: serialize(ofJson&json){
    ofSerialize(json, params);
    ofJson& jsonGroup = json["lasers"];
    for(Object3D& laser : laserObjects ){
        ofJson laserjson;
        laser.serialize(laserjson);
        jsonGroup.push_back(laserjson);
    }
    
    
}
bool Visualiser3DSettings :: deserialize(ofJson&jsonGroup){
    ofLogNotice("Visualiser json : ") << jsonGroup.dump(3);
    ofDeserialize(jsonGroup, params);
    
    ofJson& jsonLaserObjects = jsonGroup["lasers"];
    laserObjects.resize(jsonLaserObjects.size());
    for(size_t i = 0; i<jsonLaserObjects.size(); i++)  {
        laserObjects[i].deserialize(jsonLaserObjects[i]);
        
    }
    
    return true;
}

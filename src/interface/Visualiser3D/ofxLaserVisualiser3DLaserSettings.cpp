//
//  ofxLaserVisualiser3DPreset.cpp
//
//  Created by Seb Lee-Delisle on 18/06/2022.
//

#include "ofxLaserVisualiser3DLaserSettings.h"



using namespace ofxLaser;

Visualiser3DLaserSettings :: Visualiser3DLaserSettings(){
    
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
void Visualiser3DLaserSettings :: serialize(ofJson&jsonGroup){
    //ofSerialize(json, params);
    //ofJson& jsonGroup = json["lasers"];
    for(Object3D& laser : laserObjects ){
        ofJson laserjson;
        laser.serialize(laserjson);
        jsonGroup.push_back(laserjson);
    }
    
    
}
bool Visualiser3DLaserSettings :: deserialize(ofJson&jsonGroup){
    //ofLogNotice("Visualiser json : ") << jsonGroup.dump(3);
    //ofDeserialize(jsonGroup, params);
    
    ofJson& jsonLaserObjects = jsonGroup;
    laserObjects.resize(jsonLaserObjects.size());
    for(size_t i = 0; i<jsonLaserObjects.size(); i++)  {
        laserObjects[i].deserialize(jsonLaserObjects[i]);
        
    }
    
    return true;
}

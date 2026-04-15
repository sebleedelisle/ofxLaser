//
//  ofxLaserMaskManager .cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 02/02/2018.
//


#include "ofxLaserMaskManager.h"


using namespace ofxLaser;

MaskManager  ::MaskManager  () {
    
}

MaskManager  ::~MaskManager  () {
    quads.clear();
}

bool MaskManager  ::update() {
    
    //dirty = false;
    for(int i = 0; i<quads.size(); i++) {
        dirty = quads[i]->update() | dirty;
    }
    
    bool wasDirty = dirty;
    if(dirty) {
        dirty = false;
        //saveSettings();
    }
    return wasDirty;
}

std::shared_ptr<QuadMask>& MaskManager::addQuadMask(int level) {
    
    quads.push_back(std::make_shared<QuadMask>());
    std::shared_ptr<QuadMask>& quad = quads.back();
    quad->maskLevel= level;
    
    quad->setRectangle(((quads.size()-1)%16)*60+100,((quads.size()-1)/16)*60+100,50,50);
    //quad->setName(ofToString(quads.size()));
    //quad->lineColour = ofColor::red;
    
    return quad;
}

void MaskManager  ::init(int w, int h){
    width = w;
    height = h;
}

 bool MaskManager :: deleteQuadMask(std::shared_ptr<QuadMask>& mask) {
    
     vector<std::shared_ptr<QuadMask>> :: iterator it = find(quads.begin(), quads.end(), mask);
     if(it==quads.end()) {
         return false;
     }
     
     quads.erase(it);
     dirty = true;

     return true;
    
    
}

void MaskManager::serialize(ofJson&json) {
    
    // create an empty json object
    ofJson maskJson;
    
    for(int i = 0; i<(int)quads.size(); i++) {
        // create node with the index of the label and
        // serialize the quad data into it
        quads[i]->serialize(maskJson[ofToString(i)]);
    }
    // create a node called "maskmanager" and put the quad
    // data in
    json["maskmanager"] = maskJson;
    //cout << maskJson.dump(3) << endl;
    //cout << json.dump(3) << endl;
}

bool MaskManager::deserialize(ofJson& jsonGroup) {
    ofJson maskJson = jsonGroup["maskmanager"];
    quads.clear();
    bool success = true;
    for(auto quadjson : maskJson) {
        //cout << quadjson.dump(3) << endl;
        addQuadMask();
        success &= quads.back()->deserialize(quadjson);
    }
    return success;
}



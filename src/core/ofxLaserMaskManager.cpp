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
    
    while(quads.size()>0) {
        delete quads.back();
        quads.pop_back();
    }
    
}

bool MaskManager  ::update() {
    
    dirty = false;
    for(int i = 0; i<quads.size(); i++) {
        
        dirty = quads[i]->checkDirty() | dirty;
//        if(firstUpdate) {
//            quads[i]->draw();
//            
//        }
    }
   // firstUpdate = false;
    bool wasDirty = dirty;
    if(dirty) {
        dirty = false;
        //saveSettings();
    }
    return wasDirty;
}

bool MaskManager  ::draw() {

    for(int i= 0; i<quads.size(); i++) {
        quads[i]->draw();
    }
    
    return true;
}

QuadMask& MaskManager::addQuadMask(int level) {
    QuadMask* quad = new QuadMask();
    quads.push_back(quad);
    quad->maskLevel= level;
    quad->set(((quads.size()-1)%16)*60,((quads.size()-1)/16)*60,50,50);
    quad->setName(ofToString(quads.size()));
    //quad->lineColour = ofColor::red;
    
    quad->setOffsetAndScale(offset, scale);
    return *quad;
}

void MaskManager  ::init(int w, int h){
    width = w;
    height = h;
}

void MaskManager::setOffsetAndScale(glm::vec2 newoffset, float newscale){
    if((offset == newoffset) && (newscale==scale)) return;
    offset = newoffset;
    scale = newscale;
    dirty = true;
    for(QuadMask* quad : quads) {
        quad->setOffsetAndScale(offset, scale);
    }
    
}



 bool MaskManager :: deleteQuadMask(QuadMask* mask) {
    
     vector<QuadMask*> :: iterator it = find(quads.begin(), quads.end(), mask);
     if(it==quads.end()) {
         return false;
     }
     
     quads.erase(it);
     delete mask;
     
     for(int i = 0; i<(int)quads.size(); i++) {
         quads[i]->displayLabel = ofToString(i+1);
         
     }
     
     return true;
    
    
}

void MaskManager  ::enableUI(){
    for(QuadMask* quad : quads) {
        quad->setEditable(true);
    }
}
void MaskManager  ::disableUI(){
    for(QuadMask* quad : quads) {
        quad->setEditable(false);
    }
}


vector<ofPolyline*>  MaskManager :: getLaserMaskShapes(){
    
    vector<ofPolyline*> polylines;
    for(int i = 0 ;i<quads.size(); i++) {
        QuadMask& quad = *quads[i];
        ofPolyline* poly = ofxLaser::Factory :: getPolyline();
        
        
        poly->addVertex(quad.handles[0]);
        poly->addVertex(quad.handles[1]);
        poly->addVertex(quad.handles[3]);
        poly->addVertex(quad.handles[2]);
        poly->setClosed(true);
        polylines.push_back(poly);
        
        
        
    }
    return polylines;
    
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
    while(quads.size()>0) {
        delete quads.back();
        quads.pop_back();
    }
   // cout << maskJson.size() << endl;
    bool success = true;
    for(auto quadjson : maskJson) {
        //cout << quadjson.dump(3) << endl;
        addQuadMask();
        success &= quads.back()->deserialize(quadjson);
    }
    return success;
}



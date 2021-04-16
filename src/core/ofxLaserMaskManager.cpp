//
//  ofxLaserMaskManager .cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 02/02/2018.
//

// ************************ TODO MASK SYSTEM BROKEN **************************
//      - create subclass of QuadGui which stores the mask level (and index number?)
//      - create serialize / deserialize methods
//      - fix load and save settings
//      - add interface to add and remove masks.
//      - add dirty flag to QuadGui so we know to save it

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
        if(firstUpdate) {
            quads[i]->draw();
            
        }
    }
    firstUpdate = false;
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
    quad->lineColour = ofColor::red;

    
    // TODO Load / save quad masks
    
    //quad->loadSettings();
    quad->offset = offset;
    quad->scale = scale;
    return *quad;
}

void MaskManager  ::init(int w, int h){
    width = w;
    height = h;
    
    
}

void MaskManager::setOffsetAndScale(ofPoint newoffset, float newscale){
    if((offset == newoffset) && (newscale==scale)) return;
    offset = newoffset;
    scale = newscale;
    dirty = true;
    for(QuadMask* quad : quads) {
        quad->offset = offset;
        quad->scale = scale;
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

vector<ofPolyline*>  MaskManager  ::getLaserMaskShapes(){
    
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
    ofJson maskJson;// = json["maskmanager"];
    for(int i = 0; i<(int)quads.size(); i++) {
        quads[i]->serialize(maskJson[ofToString(i)]);
    }
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
    for(auto quadjson : maskJson) {
        addQuadMask();
        quads.back()->deserialize(quadjson);
    }
    return true;
}



// *************************************** BROKEN ********************************************
//
//bool MaskManager  ::loadSettings() {
//    //    for(int i = 0; i<quads.size(); i++) {
//    //        quads[i]->loadSettings();
//    //
//    //    }
//    ofFile jsonfile("Masks.json");
//    if(jsonfile.exists()) {
//        ofJson json = ofLoadJson("Masks.json");
//        
//        for(ofJson& jsonGroup : json) {
//            
//            
//        }
//        
//        dirty = true;
//        
//    }
//    
//    return true;
//}
//bool MaskManager  ::saveSettings() {
//    
//    if(quads.size()==0) return false; // maybe also delete masks file?
//    ofJson json;
//    
//    
//    for(int i = 0; i<quads.size(); i++) {
//        ofJson jsonGroup; quads[i]->serialize(jsonGroup);
//        json.push_back(jsonGroup);
//    }
//    
//    ofSavePrettyJson("Masks.json", json);
//    
//    return true;
//}

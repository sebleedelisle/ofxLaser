//
//  ofxLaserMask.cpp
//  LightningStrikesAberdeen
//
//  Created by Seb Lee-Delisle on 02/02/2018.
//

#include "ofxLaserMaskManager.h"


using namespace ofxLaser;

MaskManager::MaskManager() {
    
}

MaskManager::~MaskManager() {
    
    if(fbo.isAllocated()) fbo.clear();
    while(quads.size()>0) {
        delete quads.back();
        quads.pop_back();
    }
}

bool MaskManager::update() {
    
    dirty = false;
    for(int i = 0; i<quads.size(); i++) {
        
        dirty = quads[i]->checkDirty() | dirty;
        if(firstUpdate) {
            quads[i]->draw();
			
        }
    }
    firstUpdate = false;
    if(dirty) {
        fbo.begin();
        ofBackground(255);
       
        ofFill();
        
        for(int i = 0; i<quads.size(); i++) {
            QuadMask& quad = *quads[i];
            
            ofSetColor(255*(1.0f-quad.maskLevel));
            
            ofBeginShape();
            
            ofVertex(quad.handles[0]);
            ofVertex(quad.handles[1]);
            ofVertex(quad.handles[3]);
            ofVertex(quad.handles[2]);
            ofEndShape();
        }
        
        
        fbo.end();
        fbo.readToPixels(pixels);
        saveSettings();
    }
    return dirty;
}

bool MaskManager::draw(bool showBitmap) {
    
    if(showBitmap) {
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofSetColor(50,0,0);
        fbo.draw(0,0);
        
        ofPopStyle();
        
        for(int i= 0; i<quads.size(); i++) {
            quads[i]->draw();
        }
    }

//    if(dirty) {
//        ofSetColor(255,0,0);
//        ofFill();
//        ofDrawRectangle(0,0,20,20);
//    }
}

QuadMask& MaskManager::addQuadMask(float level) {
    QuadMask* quad = new QuadMask();
    quads.push_back(quad);
    quad->maskLevel= level;
    quad->set(((quads.size()-1)%16)*60,((quads.size()-1)/16)*60,50,50);
    quad->setName("maskquad"+ofToString(quads.size()),ofToString(quads.size()));
    quad->loadSettings();
    return *quad;
}

void MaskManager::init(int width, int height){
    
    if(fbo.isAllocated()) {
        fbo.clear();
    }
    
    fbo.allocate(width, height, GL_RGB);
    
    fbo.begin();
    ofBackground(255);
    fbo.end();
    fbo.readToPixels(pixels);
    
}

float MaskManager::getBrightness(int x, int y) {
    ofFloatColor c = pixels.getColor(x,y);
    return c.getBrightness();
    
}
ofPixels* MaskManager::getPixels() {
    return &pixels;
}


bool MaskManager::loadSettings() {
    for(int i = 0; i<quads.size(); i++) {
        quads[i]->loadSettings();
    }
    return true;
}
bool MaskManager::saveSettings() {
    for(int i = 0; i<quads.size(); i++) {
        quads[i]->saveSettings();
    }
    return true;
}

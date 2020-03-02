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
		ofDisableBlendMode();
        ofBackground(255);
		if(maskBitmap.isAllocated()) {
			maskBitmap.draw(0,0,width, height);
		}
       
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
		ofPushMatrix();
		ofTranslate(offset);
		ofScale(scale, scale);
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofSetColor(50,0,0);
        fbo.draw(0,0);
        
        ofPopStyle();
		ofPopMatrix();
        for(int i= 0; i<quads.size(); i++) {
            quads[i]->draw();
        }
    }
    return true;

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
	quad->offset = offset;
	quad->scale = scale; 
    return *quad;
}

void MaskManager::init(int w, int h){
	width = w;
	height = h; 
	maskBitmap.load("LaserMask.png");
//	if(!maskBitmap.loadImage("img/LaserMask.png")) {
//		maskBitmap.allocate(width, height, OF_IMAGE_COLOR);
//	};
	
    if(fbo.isAllocated()) {
        fbo.clear();
    }
    
    fbo.allocate(width, height, GL_RGB);
    
    fbo.begin();
    ofBackground(255);
	if(maskBitmap.isAllocated()) {
		maskBitmap.draw(0,0,w,h);
	} 
    fbo.end();
    fbo.readToPixels(pixels);
    
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

float MaskManager::getBrightness(int x, int y) {
    ofFloatColor c = pixels.getColor(x,y);
    return c.getBrightness();
    
}
ofPixels* MaskManager::getPixels() {
    return &pixels;
}

vector<ofPolyline*>  MaskManager::getLaserMaskShapes(){
	
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

bool MaskManager::loadSettings() {
    for(int i = 0; i<quads.size(); i++) {
        quads[i]->loadSettings();
		
    }
	dirty = true; 
    return true;
}
bool MaskManager::saveSettings() {
    for(int i = 0; i<quads.size(); i++) {
        quads[i]->saveSettings();
    }
    return true;
}

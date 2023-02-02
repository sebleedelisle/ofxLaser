//
//  ofxLaserBitmapMaskManager.cpp
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

#include "ofxLaserBitmapMaskManager.h"


using namespace ofxLaser;

BitmapMaskManager ::BitmapMaskManager() {
    
}

BitmapMaskManager ::~BitmapMaskManager() {
    
    if(fbo.isAllocated()) fbo.clear();
    while(quads.size()>0) {
        delete quads.back();
        quads.pop_back();
    }
}

bool BitmapMaskManager ::update() {
    
    bool isdirty = MaskManager::update();
    
    firstUpdate = false;
    if(isdirty) {
        fbo.begin();
		ofDisableBlendMode();
        ofBackground(255);
//		if(maskBitmap.isAllocated()) {
//			maskBitmap.draw(0,0,width, height);
//		}
       
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
        //saveSettings();
    }
    return isdirty;
}
//
//bool BitmapMaskManager ::draw(bool showBitmap) {
//    
//    if(showBitmap) {
//        ofPushStyle();
//		ofPushMatrix();
//		ofTranslate(offset);
//		ofScale(scale, scale);
//        ofEnableBlendMode(OF_BLENDMODE_ADD);
//        ofSetColor(50,0,0);
//        fbo.draw(0,0);
//        
//        ofPopStyle();
//		ofPopMatrix();
//        for(int i= 0; i<quads.size(); i++) {
//            quads[i]->draw();
//        }
//    }
//    return true;
//
////    if(dirty) {
////        ofSetColor(255,0,0);
////        ofFill();
////        ofDrawRectangle(0,0,20,20);
////    }
//}

void BitmapMaskManager ::init(int w, int h){
	width = w;
	height = h; 
//	maskBitmap.load("LaserMask.png");
//	if(!maskBitmap.loadImage("img/LaserMask.png")) {
//		maskBitmap.allocate(width, height, OF_IMAGE_COLOR);
//	};
	
    if(fbo.isAllocated()) {
        fbo.clear();
    }
    
    fbo.allocate(width, height, GL_RGB);
    
    fbo.begin();
    ofBackground(255);
//	if(maskBitmap.isAllocated()) {
//		maskBitmap.draw(0,0,w,h);
//	}
    fbo.end();
    fbo.readToPixels(pixels);
    
}


float BitmapMaskManager ::getBrightness(int x, int y) {
    ofFloatColor c = pixels.getColor(x,y);
    return c.getBrightness();
    
}
ofPixels* BitmapMaskManager ::getPixels() {
    return &pixels;
}

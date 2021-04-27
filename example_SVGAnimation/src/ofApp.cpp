#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    svgLoader.startLoad("svgs/");
    
}

//--------------------------------------------------------------
void ofApp::update(){
    laser.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(15,15,20);

    int frameNum = ofGetElapsedTimef()*30; // 30 frames per second
    int framesLoaded = svgLoader.getLoadCount();
    if(framesLoaded>0) frameNum = frameNum%framesLoaded;
    
    ofxLaser::Graphic& graphic = svgLoader.getLaserGraphic(frameNum);
    graphic.renderToLaser(laser, 1, OFXLASER_PROFILE_FAST);
    laser.send();
    laser.drawUI();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == OF_KEY_TAB) {
        laser.nextProjector();
    }
}

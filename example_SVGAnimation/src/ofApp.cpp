#include "ofApp.h"

// You can use the SVGLoader utility class to load an SVG sequence
// as an animation. The example animated SVG sequence is from
// a collaboration with comedian Bec Hill, see
// https://youtu.be/U4D0wJNYtWs for the back story.


//--------------------------------------------------------------
void ofApp::setup(){
    // the first time it runs, the SVGLoader will load the SVGs
    // and save each one as an optimised .ofxlg file
    // (in an "/optimised" subfolder). The next time it runs
    // it will load these instead (and is MUCH faster). If you
    // don't want this behaviour, use :
    //svgLoader.setLoadOptimisation(false);
    
    svgLoader.startLoad("svgs/");
    
}

//--------------------------------------------------------------
void ofApp::update(){
    laserManager.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(15,15,20);

    int frameNum = ofGetElapsedTimef()*30; // 30 frames per second
    frameNum = frameNum%svgLoader.getTotalFileCount();
    if(!svgLoader.hasFinishedLoading()) {
        ofDrawRectangle(0,0,ofMap(svgLoader.getLoadedPercent(), 0, 100, 0,ofGetWidth()), 8);
    }

    ofxLaser::Graphic& graphic = svgLoader.getLaserGraphic(frameNum);
    laserManager.drawLaserGraphic(graphic, 1, OFXLASER_PROFILE_FAST);

    laserManager.send();
    laserManager.drawUI();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == OF_KEY_TAB) {
        laserManager.selectNextLaser();
    }
}

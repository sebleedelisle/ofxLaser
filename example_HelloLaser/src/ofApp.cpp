#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
    
    // NOTE that you no longer need to set up anything at all in ofxLaser!
    // If you want to change the size of the laser area, use
    // laser.setCanvasSize(int width, int height) - default is 800 x 800.
    ofSetVerticalSync(false); 
    starPoly = makeStarPolyline(4);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    // prepares laser manager to receive new graphics
    laser.update();
}


void ofApp::draw() {
    
    ofBackground(15,15,20);
    
    // Drawing a laser line, provide the start point, end point and colour.
    // (you can also use glm::vec3). The final parameter is the render profile.
    // Three profiles are provided, default, fast and detail.
    ofPushMatrix();
    //if(testscale) ofScale(0,0,0);
    laser.drawLine(200, 200, 600, 200, ofColor::white, OFXLASER_PROFILE_FAST);
    
    // Draw dots. Note that if you leave out the render profile, it
    // will use the default profile. If there is haze in the room, this is
    // how you make laser beams
    
    int numDots = 6;
    for(int x = 0; x<numDots; x++) {
        // intensity is a unit value (0 - 1) that determines how long the laser lingers
        // to make the dot. It defaults to 1. 
        float intensity = (float)(x+1)/numDots;
        float xposition = ofMap(x, 0,numDots-1,200,600);
        laser.drawDot(xposition, 600, ofColor::red, intensity, OFXLASER_PROFILE_FAST);
    }
    
    // note that matrix transformations work as well, even in 3D!
    ofPushMatrix();
    ofTranslate(500,400);
    
    // 3D rotation around the y axis
    ofRotateYRad(ofGetElapsedTimef());
    laser.drawPoly(starPoly, ofColor::magenta, OFXLASER_PROFILE_DEFAULT);
    ofPopMatrix();
    
    laser.drawCircle(275, 400, 80, ofColor(0,50,255), OFXLASER_PROFILE_FAST);
    
    glm::vec2 mousepos(ofGetMouseX(), ofGetMouseY());
    mousepos = laser.screenToLaserInput(mousepos);
    laser.drawCircle(mousepos, 5, ofColor(0,50,255), OFXLASER_PROFILE_FAST);
    
    ofPopMatrix();
    // sends points to the DAC
    laser.send();
    // draw the laser UI elements
    laser.drawUI();
    //ofSetColor(255);
    //ofDrawBitmapString(ofToString(ofGetFrameRate()), 10, 20);
}


ofPolyline ofApp::makeStarPolyline(int numSides) {
    ofPolyline poly;
    // create a star polyline object
    float angle = 360.0f / (float)numSides /2.0f;
    for(int i = 0; i<=numSides; i++) {
        poly.rotateDeg(angle, glm::vec3(0,0,1));
        poly.addVertex(-100,0);
        if(i==numSides) continue;
        poly.rotateDeg(angle, glm::vec3(0,0,1));
        poly.addVertex(-30,0);
    }
    return poly;
}

//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs& e){
    
    if(e.key==OF_KEY_TAB) {
        laser.selectNextLaser();
    }// if(e.key == ' ') {
     //   testscale = !testscale;
    //}
    
}

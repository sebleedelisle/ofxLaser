#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
    
    // NOTE that you no longer need to set up anything at all in ofxLaser!
    ofSetVerticalSync(false); 
   
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // prepares laser manager to receive new graphics
    laser.update();
}


void ofApp::draw() {
    
    ofBackground(5,5,10);
    

    for(int i = 0; i<laser.getNumBeamZones(); i++ ) {
        laser.setTargetBeamZone(i);
        //textGraphic.alignment =  StringGraphic::ALIGN_CENTRE;
        textGraphic.setString(ofToString(i), ofColor::white);
        textGraphic.autoCentre();
        laser.pushMatrix();
        laser.translate(400,400);
        laser.scale(20,20);
        laser.drawLaserGraphic(textGraphic);
        laser.popMatrix();
        //laser.drawDot(400, 400, ofColor::red);
    }
        
    
    // sends points to the DAC
    laser.send();
    // draw the laser UI elements
    laser.drawUI();
   
}



//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs& e){
    
    if(e.key==OF_KEY_TAB) {
        laser.selectNextLaser();
    }// if(e.key == ' ') {
     //   testscale = !testscale;
    //}
    
}

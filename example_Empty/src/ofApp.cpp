#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
	
	laserWidth = 800;
	laserHeight = 800;
	laser.setup(laserWidth, laserHeight);
	
	ofxGuiSetDefaultWidth(300);
    ofxGuiSetFillColor(ofColor::fromHsb(144,100,112));
	laserGui.setup();
	laserGui.add(laser.parameters);
	
	laserGui.add(laser.redParams);
	laserGui.add(laser.greenParams);
	laserGui.add(laser.blueParams);

	laserGui.loadFromFile("laserSettings.xml");
    laser.laserArmed = false; 

	laserGui.setPosition(ofGetWidth()-320, 10);
    
    laserGui.loadFont("Verdana.ttf", 8, false);

    


}

//--------------------------------------------------------------
void ofApp::update(){
	

	laser.update();

}



//--------------------------------------------------------------
void ofApp::draw() {
	

	ofBackground(0);
	laser.draw();
	laserGui.draw();
	colourGui.draw();
	
	ofNoFill();
	ofSetLineWidth(1);
	ofDrawRectangle(0,0,laserWidth, laserHeight);
	

}

//--------------------------------------------------------------
void ofApp::exit(){
	laserGui.saveToFile("laserSettings.xml");
    laser.warp.saveSettings(); 

}

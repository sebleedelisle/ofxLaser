#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
	
	laserWidth = 800;
	laserHeight = 800;
	laser.setup(laserWidth, laserHeight);
	
	ofxGuiSetDefaultWidth(300);
	laserGui.setup();
	laserGui.add(laser.parameters);
	
	laserGui.add(laser.redParams);
	laserGui.add(laser.greenParams);
	laserGui.add(laser.blueParams);

	laserGui.loadFromFile("laserSettings.xml");

	laserGui.setPosition(laserWidth+50, 0);
	

    
    
    
    
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

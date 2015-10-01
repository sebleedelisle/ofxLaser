#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
	
	laserWidth = 800;
	laserHeight = 800;
	laser.setup(laserWidth, laserHeight);
	laser.connectToEtherdream();

	
	ofxGuiSetDefaultWidth(300);
	laserGui.setup(laser.parameters);
	laserGui.setPosition(laserWidth+50, 0);
	
	laserGui.loadFromFile("laserSettings.xml");
	
	redGui.setup("Laser Red");
	redGui.add(laser.redParams );
	greenGui.setup("Laser Green", "lasergreen.xml");
	greenGui.add(laser.greenParams );
	blueGui.setup("Laser Blue", "laserblue.xml");
	blueGui.add(laser.blueParams );
	
	redGui.loadFromFile("laserred.xml");
	greenGui.loadFromFile("lasergreen.xml");
	blueGui.loadFromFile("laserblue.xml");

	
	int panelwidth  = 220;
	redGui.setPosition(ofPoint(10 ,10));
	greenGui.setPosition(ofPoint(10 +panelwidth + 10,10));
	blueGui.setPosition(ofPoint(10 +panelwidth*2 + 20,10));

	
	//gui.setWidthElements(400);
	
	
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
	redGui.draw();
	greenGui.draw();
	blueGui.draw();
	
	ofNoFill();
	ofSetLineWidth(1);
	ofRect(0,0,laserWidth, laserHeight);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	
}

void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::exit(){
	redGui.saveToFile("laserred.xml");
	greenGui.saveToFile("lasergreen.xml");
	blueGui.saveToFile("laserblue.xml");

	
}

#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
	
	laserWidth = 800;
	laserHeight = 600;
	laser.setup(laserWidth, laserHeight);
	
	laser.addZone(0,0,400,600);
	laser.addZone(400,0,400,600);
	
	laser.addProjector();
	laser.addProjector();

	laser.addZoneToProjector(0,0);
	laser.addZoneToProjector(1,1);
    
    laser.initGui();
    laser.showZones = true;

	 
	
}

//--------------------------------------------------------------
void ofApp::update(){
    
	float deltaTime = ofClamp(ofGetLastFrameTime(), 0, 0.2);
	elapsedTime+=deltaTime;
    
    // prepares laser manager to receive new points
    laser.update();
	
}


void ofApp::draw() {
	
	ofBackground(15,15,20);
	
    
    float speed = 1;
    for(int i = 0; i<30; i++) {

        ofColor c;
        c.setHsb(i*6,255,255);
        ofPoint p;
        float spread = ofMap(cos(elapsedTime*0.4),1,-1,0.01,0.1);
        p.x = sin((elapsedTime-((float)i*spread)) *1.83f * speed) * 300;
        p.y = sin((elapsedTime-((float)i*spread)) *2.71f *speed) * 150;
        p.x+=laserWidth/2;
        p.y+=laserHeight/2;
        
        laser.drawDot(p, c);
        
    }
    
    // sends points to the DAC
    laser.send();

    laser.drawUI();


}




//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
	if(key=='f') {
        ofToggleFullscreen();
	}
    if(key==OF_KEY_TAB) {
        laser.nextProjector();
    }
}

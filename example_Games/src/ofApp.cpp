#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
	
	laserWidth = 800;
	laserHeight = 800;
	laser.setup(laserWidth, laserHeight);
	laser.maskRectangle.height = 600;
	//laser.connectToEtherdream();

	ofxGuiSetDefaultWidth(300);
	laserGui.setup();
	laserGui.add(laser.parameters);
	
	//laserGui.add(laser.redParams);
	//laserGui.add(laser.greenParams);
	//laserGui.add(laser.blueParams);

	laserGui.add(flappyBird.sensitivity.set("flappy sensitivity", 1,0,10));
	laserGui.add(timeSpeed.set("time speed", 1,0,1));
	
	laserGui.loadFromFile("laserSettings.xml");

	laserGui.setPosition(laserWidth+50, 0);
	
	//laserGui.setWidthElements(400);
	currentLaserEffect = 0;
	numLaserEffects = 3;
	
	elapsedTime = deltaTime = 0;
	
	smoothedInputVolume = 0;
	soundStream.setup(this, 0, 2, 48000, 1024, 1);
	left.resize(1024);
	right.resize(1024);
	showGui = true;
	blankAll = false;
	
	
	
}

//--------------------------------------------------------------
void ofApp::update(){
	deltaTime = ofClamp(ofGetLastFrameTime()*timeSpeed, 0, 0.2);
	
	
	elapsedTime+=deltaTime;
	
	
	showLaserEffect(currentLaserEffect);
	
	laser.update();

}



//--------------------------------------------------------------
void ofApp::draw() {
	

	ofBackground(0);
	laser.draw();
	
	if(showGui) laserGui.draw();
	// if you need to adjust the colour, uncomment :
	//colourGui.draw();
	
	if(!blankAll) {
		ofNoFill();
		ofSetLineWidth(1);
		ofRect(laser.maskRectangle);
		
		int ypos = laserHeight+20;
		ofDrawBitmapString("Current Effect : "+ofToString(currentLaserEffect), 20, ypos+=30);
		
		ofDrawBitmapString("[ and ] to change current effect", 20, ypos+=30);
		ofDrawBitmapString(ofToString(smoothedInputVolume), 20, ypos+=30);
		ofRect(20,ypos+=30,smoothedInputVolume*600, 20);
	}

	ofSetColor(255);
	ofPushMatrix();
	ofTranslate(ofGetMouseX(), ofGetMouseY());
	ofLine(0,0,0,10);
	ofLine(0,0,6,8);
	ofPopMatrix();
}


void ofApp :: showLaserEffect(int effectnum) {
	
	float left = laserWidth*0.0;
	float top = laserHeight*0.0;
	float right = laserWidth*1.0;
	float bottom = laserHeight*1.0;
	float width = laserWidth*1.0;
	float height = laserHeight*1.0;
	
	asteroids.active = false;
	
	switch (currentLaserEffect) {
			
		case 1: {
			asteroids.active = true;
			// ASTEROIDS
			asteroids.update(deltaTime);
			asteroids.draw(laser);
			break;

		}
		
		case 2: {
			
			flappyBird.update(deltaTime, smoothedInputVolume);
			flappyBird.draw(laser);
			break;
			
		}
		
			
	}

	
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
	if(key =='c') {
		
	} else if (key == '[') {
		currentLaserEffect--;
		if(currentLaserEffect<0) currentLaserEffect = numLaserEffects-1;
	} else if (key == ']') {
		currentLaserEffect++;
		if(currentLaserEffect>=numLaserEffects) currentLaserEffect = 0;
	} else if (key == ' ') {
		if(currentLaserEffect == 2) flappyBird.flap();
	} else if (key == 'r') {
		flappyBird.reset();
	} else if (key == OF_KEY_TAB) {
		showGui = !showGui;
	} else if(key == 'b') {
		blankAll = !blankAll;
		laser.renderLaserPreview = !blankAll;
		laser.renderLaserPathPreview = !blankAll;
	} else if(key =='w') {
		laser.showWarpUI = !laser.showWarpUI;
	} else if(key =='f') {
		ofToggleFullscreen();
	} else if(key =='p') {
		if(timeSpeed>0.5) timeSpeed = 0;
		else timeSpeed = 1;
	}


}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){


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
void ofApp::audioIn(float * input, int bufferSize, int numChannels){
	float curVol = 0.0;
	
	// samples are "interleaved"
	int numCounted = 0;
	
	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume
	for (int i = 0; i < bufferSize; i++){
		left[i]		= input[i*2]*0.5;
		right[i]	= input[i*2+1]*0.5;
		
		curVol += left[i] * left[i];
		curVol += right[i] * right[i];
		numCounted+=2;
	}
	
	//this is how we get the mean of rms :)
	curVol /= (float)numCounted;
	
	// this is how we get the root of rms :)
	curVol = sqrt( curVol );
	
	smoothedInputVolume *= 0.8;
	smoothedInputVolume += 0.5 * curVol;
	//smoothedInputVolume = curVol;
	//bufferCounter++;
	
	
}

//--------------------------------------------------------------
void ofApp::exit(){
	laserGui.saveToFile("laserSettings.xml");
    laser.warp.saveSettings();

}

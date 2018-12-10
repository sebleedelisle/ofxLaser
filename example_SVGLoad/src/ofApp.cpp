#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
	
    
    // get the filenames of all the svgs in the data/svgs folder
    string path = "svgs/";
    ofDirectory dir(path);
    dir.allowExt("svg");
    dir.listDir();
    dir.sort();
    
    // and load them all
    const vector<ofFile>& files = dir.getFiles();
    //laserGraphics.resize(files.size());
    //fileNames.resize(files.size());
    
    for(int i = 0; i<files.size();i++) {
        const ofFile & file = files.at(i);
		ofxSVG svg;
        svg.load(file.getAbsolutePath());
		laserGraphics.push_back(ofxLaser::Graphic());
		laserGraphics.back().addSvg(svg);
		//laserGraphics.back().connectLineSegments();
		laserGraphics.back().autoCentre();
		
		ofLog(OF_LOG_NOTICE,file.getAbsolutePath());
		fileNames.push_back(file.getFileName());
		
		
    }
    
    // set up the laser manager
    
	laserWidth = 800;
	laserHeight = 800;
	laser.setup(laserWidth, laserHeight);

    laser.addProjector(dac);
    
    // load the IP address of the Etherdream
    ofBuffer buffer = ofBufferFromFile("dacIP.txt");
    string dacIp = buffer.getText();
	// if there's no file, then use the default IP address :
    if(dacIp=="") dacIp ="10.0.1.12";
    dac.setup(dacIp);
	
    laser.initGui();
    
    ofParameterGroup params;
    params.add(scale.set("SVG scale", 2, 0.1,6));
    laser.gui.add(params);
    currentSVG = 0;
	
	 
}

//--------------------------------------------------------------
void ofApp::update(){
    laser.update();
}


void ofApp::draw() {
	

	ofBackground(0);
    
	ofxLaser::Graphic& laserGraphic = laserGraphics[currentSVG];
	
	ofNoFill();
	ofSetLineWidth(1);
	ofDrawRectangle(0,0,laserWidth, laserHeight);
    
	int ypos = laserHeight+10;
    int xpos = 360;
	ofDrawBitmapString("Current SVG : "+ofToString(currentSVG) + " "+fileNames[currentSVG], xpos, ypos+=30);
	
	ofDrawBitmapString("Left and Right Arrows to change current SVG", xpos, ypos+=30);
    ofDrawBitmapString("'F' to toggle fullscreen", xpos, ypos+=20);
    ofDrawBitmapString("'TAB' to toggle laser preview mode", xpos, ypos+=20);
    ofDrawBitmapString("Adjust Render Profiles -> Default to affect laser speed", xpos, ypos+=20);
    
   
    int laserframerate = laser.getProjectorFrameRate(0); 
    ofDrawBitmapString("Laser framerate  : " + ofToString(laserframerate), 20, 20);
	ofDrawBitmapString("Render framerate : " + ofToString(ofGetFrameRate()), 20, 35);
	
    ofPushMatrix();

    ofTranslate(400, 400);
	ofScale(scale, scale); 
    if(laserGraphics.size()>currentSVG) {
		laserGraphics[currentSVG].renderToLaser(laser, 1, OFXLASER_PROFILE_DEFAULT);
    }
    ofPopMatrix();
    laser.send();
    laser.drawUI();
	
	

}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
   
   if(key =='f') {
        ofToggleFullscreen();
    } else if (key == OF_KEY_LEFT) {
		currentSVG--;
		if(currentSVG<0) currentSVG = laserGraphics.size()-1;
	} else if (key == OF_KEY_RIGHT) {
		currentSVG++;
		if(currentSVG>=laserGraphics.size()) currentSVG = 0;
	}
    
    if(key==OF_KEY_TAB) laser.nextProjector();

}

//--------------------------------------------------------------
void ofApp::exit(){
    laser.saveSettings();
    dac.close();
}

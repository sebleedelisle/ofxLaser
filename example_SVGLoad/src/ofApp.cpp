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

	svgs.resize(files.size());
	
	for(int i = 0; i<files.size();i++) {
        const ofFile & file = files.at(i);
		
		// note that we are using ofxSvgExtra rather than the basic
		// ofxSVG - I have added some functionality to the SVG object
		// and put in a pull request to oF, but in the meantime,
		// we can use this surrogate custom SVG class.
		
		ofxSVGExtra& svg = svgs[i];
		
        svg.load(file.getAbsolutePath());
		laserGraphics.push_back(ofxLaser::Graphic());
		
		// the ofxLaser::Graphic object is a way to manage a bunch
		// of shapes for laser rendering, and it can load SVGs.
		
		// addSvg(ofxSVGExtra& svg, bool optimise, bool subtractFills)
		// optimise : if true, will join similar line segments into conjoined paths
		// (way faster to laser)
		// subtractFills : if true, will subtract filled areas from shapes underneath
		// this is important because otherwise, all our objects will appear transparent.
	
		laserGraphics.back().addSvg(svg, true, true);
		
		// this centres all the graphics
		laserGraphics.back().autoCentre();
		
		ofLog(OF_LOG_NOTICE,file.getAbsolutePath());
		fileNames.push_back(file.getFileName());
		
    }
    
    // set up the laser manager
    
	laserWidth = 800;
	laserHeight = 800;
	laser.setup(laserWidth, laserHeight);

    laser.addProjector(dac);
	
#ifdef USE_LASERDOCK
	dac.setup();
#else
    // load the IP address of the Etherdream / IDN DAC
    ofBuffer buffer = ofBufferFromFile("dacIP.txt");
    string dacIp = buffer.getText();
	// if there's no file, then use the default IP address :
    if(dacIp=="") dacIp ="10.0.1.12";
    dac.setup(dacIp);
#endif
	
	laser.addCustomParameter(scale.set("SVG scale", 1.3, 0.1,6));
    laser.initGui();
    
	
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
	//	svgs[currentSVG].draw();
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
#ifndef USE_LASERDOCK
    dac.close();
#endif
}

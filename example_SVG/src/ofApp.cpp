#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
	
    // ensures the laser framerate can be faster than the screen update
    ofSetVerticalSync(false);
    
    
    // get the filenames of all the svgs in the data/svgs folder
    string path = "svgs/";
    ofDirectory dir(path);
    dir.allowExt("svg");
    dir.listDir();
    dir.sort();
    
    // and load them all
    const vector<ofFile>& files = dir.getFiles();
    
    dir.close();

	//svgs.resize(files.size());
	
	for(int i = 0; i<files.size();i++) {
        const ofFile & file = files.at(i);

		laserGraphics.emplace_back();
		
        // addSvgFromFile(string filename, bool optimise, bool subtractFills)
		laserGraphics.back().addSvgFromFile(file.getAbsolutePath(), false, true);
		
		// this centres all the graphics
		laserGraphics.back().autoCentre();
		
		ofLog(OF_LOG_NOTICE,file.getAbsolutePath());
		fileNames.push_back(file.getFileName());
    }
    
		
    laserManager.addCustomParameter(currentSVG.set("Current SVG", 0, 0, laserGraphics.size()-1));
    laserManager.addCustomParameter(currentSVGFilename.set("Filename"));
	laserManager.addCustomParameter(scale.set("SVG scale", 1.0, 0.1,6));
    laserManager.addCustomParameter(rotate3D.set("Rotate 3D", true));
    laserManager.addCustomParameter(renderProfileLabel.set("Render Profile name",""));
    laserManager.addCustomParameter(renderProfileIndex.set("Render Profile", 1, 0, 2));
    
    ofParameter<string> description;
    description.set("description", "INSTRUCTIONS : \nLeft and Right Arrows to change current SVG \nTAB to toggle output editor \nF to toggle full screen");
    laserManager.addCustomParameter(description);
	 
}

//--------------------------------------------------------------
void ofApp::update(){
    laserManager.update();
}


void ofApp::draw() {
	
	ofBackground(15,15,20);
       
    string renderProfile;
    switch (renderProfileIndex) {
        case 0 :
            renderProfile = OFXLASER_PROFILE_DEFAULT;
            break;
        case 1 :
            renderProfile = OFXLASER_PROFILE_DETAIL;
            break;
        case 2 :
            renderProfile = OFXLASER_PROFILE_FAST;
            break;
    }
    if(renderProfileLabel.get() != ("Render Profile : OFXLASER_PROFILE_" + renderProfile))
        renderProfileLabel = ("Render Profile : OFXLASER_PROFILE_" + renderProfile);
    
	ofxLaser::Graphic& laserGraphic = laserGraphics[currentSVG];
	
    if(currentSVGFilename.get()!=fileNames[currentSVG]) currentSVGFilename = fileNames[currentSVG];
	
    laserManager.beginDraw();
    
    laserManager.pushMatrix();

    laserManager.translate(400, 400);
    laserManager.scale(scale, scale);
    if(rotate3D) {
        float angle = fmod(ofGetElapsedTimef()*30, 180)-90;
        laserManager.rotateYDeg(angle);
    }
    if(laserGraphics.size()>currentSVG) {
		laserManager.drawLaserGraphic(laserGraphics[currentSVG], 1, renderProfile);
    }
    laserManager.popMatrix();
    
    laserManager.endDraw();
    
    laserManager.send();
    laserManager.drawUI();
	
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
    
    if(key==OF_KEY_TAB) laserManager.selectNextLaser();

}

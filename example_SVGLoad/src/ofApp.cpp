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
    
    dir.close();

	//svgs.resize(files.size());
	
	for(int i = 0; i<files.size();i++) {
        const ofFile & file = files.at(i);
		
		// note that we are using ofxSvgExtra rather than the basic
		// ofxSVG - I have added some functionality to the SVG object
		// and put in a pull request to oF, but in the meantime,
		// we can use this surrogate custom SVG class.
		
		//ofxSVGExtra& svg = svgs[i];
		
        //svg.load(file.getAbsolutePath());
		laserGraphics.emplace_back();
		
		// the ofxLaser::Graphic object is a way to manage a bunch
		// of shapes for laser rendering, and it can load SVGs.
		
		// addSvg(ofxSVGExtra& svg, bool optimise, bool subtractFills)
		// optimise : if true, will join similar line segments into conjoined paths
		// (way faster to laser)
		// subtractFills : if true, will subtract filled areas from shapes underneath
		// this is important because otherwise, all our objects will appear transparent.
	
		laserGraphics.back().addSvgFromFile(file.getAbsolutePath(), false, true);
		
		// this centres all the graphics
		laserGraphics.back().autoCentre();
		
		ofLog(OF_LOG_NOTICE,file.getAbsolutePath());
		fileNames.push_back(file.getFileName());
    }
    
		
    laser.addCustomParameter(currentSVG.set("Current SVG", 0, 0, laserGraphics.size()));
    laser.addCustomParameter(currentSVGFilename.set("Filename"));
	laser.addCustomParameter(scale.set("SVG scale", 1.0, 0.1,6));
    laser.addCustomParameter(rotate3D.set("Rotate 3D", true));
    laser.addCustomParameter(renderProfileLabel.set("Render Profile name",""));
    laser.addCustomParameter(renderProfileIndex.set("Render Profile", 1, 0, 2));
    
    ofParameter<string> description;
    description.set("INSTRUCTIONS : \nLeft and Right Arrows to change current SVG \nTAB to toggle output editor \nF to toggle full screen");
    laser.addCustomParameter(description.set("Description", ""));
    
   // laser.initGui();
    
	
    currentSVG = 0;
	
	 
}

//--------------------------------------------------------------
void ofApp::update(){
    laser.update();
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
    renderProfileLabel = "Render Profile : OFXLASER_PROFILE_" + renderProfile;
    
	ofxLaser::Graphic& laserGraphic = laserGraphics[currentSVG];
	
    currentSVGFilename = fileNames[currentSVG];
	
    laser.beginDraw();
    
    ofPushMatrix();

    ofTranslate(400, 400);
	ofScale(scale, scale);
    if(rotate3D) {
        float angle = fmod(ofGetElapsedTimef()*30, 180)-90;
        ofRotateYDeg(angle);
    }
    if(laserGraphics.size()>currentSVG) {
		laserGraphics[currentSVG].renderToLaser(laser, 1, renderProfile);
    }
    ofPopMatrix();
    
    laser.endDraw();
    
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

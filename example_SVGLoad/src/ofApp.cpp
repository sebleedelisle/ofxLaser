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
    fileNames.resize(files.size());
    
    for(int i = 0; i<files.size();i++) {
        const ofFile & file = files.at(i);
        svgs[i].load(file.getAbsolutePath());
        ofLog(OF_LOG_NOTICE,file.getAbsolutePath());
        fileNames[i] = file.getFileName();
        
    }
    
    // set up the laser manager
    
	laserWidth = 800;
	laserHeight = 800;
	laser.setup(laserWidth, laserHeight);

    laser.addProjector(dac);
    
    // load the IP address of the Etherdream
    ofBuffer buffer = ofBufferFromFile("dacIP.txt");
    string dacIp = buffer.getText();
    if(dacIp=="") dacIp ="169.254.70.201";
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
    
    ofxSVG& svg = svgs[currentSVG];
	
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
    ofDrawBitmapString("Laser framerate : " + ofToString(laserframerate), 20, 20);
    
    ofPushMatrix();

    ofPoint pos( 400, 400, 0);
    if(svgs.size()>currentSVG) {
        ofxSVG & svg = svgs[currentSVG];
        
       ofPoint centrePoint = ofPoint(svg.getWidth()/2, svg.getHeight()/2);
        
        for(int i=0; i<svg.getNumPath(); i++ ) {
            ofPath& path = svg.getPathAt(i);
            
            const vector<ofPolyline>& lines = path.getOutline();
           
            ofColor col = path.getStrokeColor();
            
            // if there are black lines, change them to white
            if(col.getBrightness()<30) col = ofColor::white;
            
            for(int j=0; j<lines.size(); j++) {
                ofPolyline line = lines[j];
                
                auto & vertices = line.getVertices();
                for(int i = 0; i<vertices.size(); i++) {
                    auto & v = vertices[i];
                    v-=centrePoint;
                    v*=scale;
                    v+=pos;
                }

                laser.drawPoly(line,col);
            }
        }
        

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
		if(currentSVG<0) currentSVG = svgs.size()-1;
	} else if (key == OF_KEY_RIGHT) {
		currentSVG++;
		if(currentSVG>=svgs.size()) currentSVG = 0;
	}
    
    if(key==OF_KEY_TAB) laser.nextProjector();

}

//--------------------------------------------------------------
void ofApp::exit(){
    laser.saveSettings();
    dac.close();
}

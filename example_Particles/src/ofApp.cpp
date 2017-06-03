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
    laser.laserArmed = false;
    laser.intensity = 0.1;

	laserGui.setPosition(laserWidth+50, 0);
	
	
	currentLaserEffect = 0;
	numLaserEffects = 8;
    
    for(int i = 0; i<10; i++) {
        Particle p;
        p.pos.set(ofRandom(laserWidth), ofRandom(laserHeight));
        //p.vel.set(0,80);
        p.vel.set(ofRandom(-70,70),80);
        
        float z = ofRandom(50,500);
        p.size = (250.0+z) / z;
        particles.push_back(p);
        
        
        ofLog(OF_LOG_NOTICE, ofToString(p.size));
    }
    ofRectangle rect(laserWidth*0.1,laserHeight*0.1,laserWidth*0.8, laserHeight*0.8);
    square.addVertex(rect.getTopLeft());
    square.addVertex(rect.getTopRight());
    square.addVertex(rect.getBottomRight());
    square.addVertex(rect.getBottomLeft());
    square.addVertex(rect.getTopLeft());
    
    
    ofSetVerticalSync(false);

    
}

//--------------------------------------------------------------
void ofApp::update(){
	
	
	
	showLaserEffect(currentLaserEffect);
	
	
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
	
	int ypos = laserHeight+20;
	ofDrawBitmapString("Current Effect : "+ofToString(currentLaserEffect), 20, ypos+=30);
	
	ofDrawBitmapString("Left and Right Arrows to change current effect", 20, ypos+=30);
    ofDrawBitmapString("Mouse to draw polylines, 'C' to clear", 20, ypos+=30);
    ofDrawBitmapString("Mouse to draw polylines, 'C' to clear", 20, ypos+=30);

    ofDrawBitmapString(ofToString(ofGetFrameRate()), 10,10);

    
}


void ofApp :: showLaserEffect(int effectnum) {
	
    float deltaTime = ofClamp(ofGetLastFrameTime(), 0, 0.2);
    deltaTime*=0.5;
    elapsedTime+=deltaTime;
    
    //laser.addLaserPolyline(square, ofColor::cyan);
    
    for(int i = 0; i<particles.size(); i++) {
        
        Particle &p = particles[i];
        p.pos+=p.vel*p.size * deltaTime;
        while(p.pos.y>laserHeight) {
            p.pos.y -= laserHeight;
           // p.vel.set(ofRandom(-70,70),80);
        }
        if(p.pos.x<0) p.pos.x+=laserWidth;
        else if(p.pos.x>laserWidth) p.pos.x-=laserWidth;
        
       if(p.size<4)
            laser.addLaserDot(p.pos, ofColor::white,p.size/5.0f);
        else {
             laser.addLaserCircle(p.pos, p.size, ofColor::white);
        }
        
    }
    
    
    
	
	
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key =='c') {
		polyLines.clear();
	} else if (key == OF_KEY_LEFT) {
		currentLaserEffect--;
		if(currentLaserEffect<0) currentLaserEffect = numLaserEffects-1;
	} else if (key == OF_KEY_RIGHT) {
		currentLaserEffect++;
		if(currentLaserEffect>=numLaserEffects) currentLaserEffect = 0;
	}

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	if(!drawingShape) return;
	
	ofPolyline &poly = polyLines.back();
	//if(poly.getofVec3f end =poly.getVertices().back();
	//if(ofDist(x, y, end.x, end.y) > 5) {
		//poly.simplify();
	poly.addVertex(x, y);
	
	//}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	polyLines.push_back(ofPolyline());
	drawingShape = true;
}

void ofApp::mouseReleased(int x, int y, int button) {
	if(drawingShape) {
		ofPolyline &poly = polyLines.back();
		poly = poly.getSmoothed(2);
		drawingShape = false;
	}
	// TODO add dot if the line is super short
	
}

//--------------------------------------------------------------
void ofApp::exit(){
	laserGui.saveToFile("laserSettings.xml");
    laser.warp.saveSettings(); 

}

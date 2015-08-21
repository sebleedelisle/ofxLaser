#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
	
	laserWidth = 800;
	laserHeight = 800;
	laser.setup(laserWidth, laserHeight);
	laser.connectToEtherdream();

	
	for(int i = 0; i<10; i++) {
		positions.push_back(ofPoint(ofRandom(laserWidth), ofRandom(laserHeight)));
		velocities.push_back(ofPoint(30, 0));
		velocities[i].rotate(ofRandom(360),ofPoint(0,0,1));
		
		
	}
	ofxGuiSetDefaultWidth(300);
	laserGui.setup(laser.parameters);
	laserGui.setPosition(laserWidth+50, 0);
	
	laserGui.loadFromFile("laserSettings.xml");
	
	//laserGui.setWidthElements(400);
	
	
}

//--------------------------------------------------------------
void ofApp::update(){
	float deltaTime = ofClamp(ofGetLastFrameTime(), 0, 0.2);
	elapsedTime+=deltaTime;
	
	
	showLaserEffect(currentLaserEffect);
	
	
	laser.update();

}



//--------------------------------------------------------------
void ofApp::draw() {
	

	ofBackground(0);
	laser.draw();
	laserGui.draw();
	
	ofNoFill();
	ofSetLineWidth(1);
	ofRect(0,0,laserWidth, laserHeight);

}


void ofApp :: showLaserEffect(int effectnum) {
	
	// LASER PARTICLES ---------------------------------
	//
	//	for(int x = laserWidth*0.1; x<laserWidth*0.9; x+=laserWidth*0.2) {
	//
	//		laser.addLaserDot(ofPoint(x,laserHeight/2 ), ofColor::white, 1);
	//	}
	//
	//	for(int i = 0; i<positions.size(); i++) {
	//		ofPoint&p = positions[i];
	//		ofPoint&v = velocities[i];
	//		p+=v*ofGetLastFrameTime();
	//		if((p.x>laserWidth) || (p.x<0)) {
	//			v.x*=-1;
	//			p.x = ofClamp(p.x, 0, laserWidth);
	//		}
	//		if((p.y>laserHeight) || (p.y<0)) {
	//			v.y*=-1;
	//			p.y = ofClamp(p.y, 0, laserHeight);
	//		}
	//		laser.addLaserDot(p, ofColor::white, 1);
	//
	//
	//	}
	
	
	
	// LASER SINE WAVE PARTICLES --------------------------------------
	
	float speed = 1;
	for(int i = 0; i<30; i++) {
		
		ofColor c;
		c.setHsb(i*6,255,255);
		ofPoint p;
		float spread = ofMap(cos(elapsedTime*0.4),1,-1,0.01,0.1);
		p.x = sin((elapsedTime-((float)i*spread)) *1.83f * speed) * 300;
		p.y = sin((elapsedTime-((float)i*spread)) *2.71f *speed) * 300;
		p.x+=laserWidth/2;
		p.y+=laserHeight/2;
		laser.addLaserDot(p, c);
		
	}
	
	// LASER LINES
	
	//	for(int i = 0; i<10; i++) {
	//		laser.addLaserLine(ofPoint(i*100, 100), ofPoint(i*100, 700), ofColor::white);
	//
	//	}
	//
	//
	//	// LASER CIRCLES
	//
	//	for(int i = 1; i<10; i++) {
	//		laser.addLaserCircle(ofPoint(i*100, 500), 50, ofColor::white);
	//	}
	//
	
	// LASER POLYLINES
	for(int i = 0; i<polyLines.size(); i++) {
		laser.addLaserPolyline(polyLines[i], ofColor::white);
	}
	

	
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key =='c') {
		polyLines.clear();
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

}

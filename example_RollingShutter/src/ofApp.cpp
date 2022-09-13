#include "ofApp.h"


// PLEASE NOTE - NOT CURRENTLY WORKING
// The DAC code has been refactored considerably and is now very powerful
// but the raw points functionality needs to be reworked. Sorry for the
// inconvenience.

// This example demonstrates how you can use the rolling shutter in
// cameras to create cool effects. See my collaboration with Tom Scott
// for more information https://www.youtube.com/watch?v=8YONOexk0Ek&t=319s
// (apologies for the lack of comments / documentation)



//--------------------------------------------------------------
void ofApp::setup(){
	
	startTime = 0;

    laserManager.getLaser(0).pps = 25000; // set this to be 1000 x your frame rate
    
	laserManager.addCustomParameter(startOffset.set("start offset", 0, 0,500));
	laserManager.addCustomParameter(endOffset.set("end offset", 0, 0,500));
	laserManager.addCustomParameter(pointsPerFrame.set("points per frame", 1000, 900,1100));
	laserManager.addCustomParameter(beamPos.set("beam pos", 0.5,0,1));
	laserManager.addCustomParameter(timeSpeed.set("time speed", 1,0.01,2));
	points.resize(pointsPerFrame);
	
    currentLaserEffect = 0;
    numLaserEffects = 7;
		
}

//--------------------------------------------------------------
void ofApp::update(){
    
	float deltaTime = ofGetLastFrameTime();
    pointsToSend+= deltaTime*laserManager.getLaserPointRate(0);
	elapsedTime+=(deltaTime*timeSpeed);
	
    // prepares laser manager to receive new points
    laserManager.update();
	
	while(pointsToSend>points.size()) {
		
		laserManager.sendRawPoints(points);
		pointsToSend-=points.size();
	}
}


void ofApp::draw() {
	
	ofBackground(15,15,20);
	
    showLaserEffect(currentLaserEffect);

    // sends points to the DAC
    //laser.send();
	
	ofMesh mesh;
	for(int i = 0; i<points.size(); i++) {
		mesh.addVertex(glm::vec3(ofMap(i, 0, points.size(), 0, 800), points[i].x,0));
		mesh.addColor(points[i].getColour());
		
		
	}
	
    laserManager.drawUI();
    
    if(laserManager.viewMode==OFXLASER_VIEW_CANVAS) {
        
        int ypos = 816;
        ofDrawBitmapString("Current Effect : "+ofToString(currentLaserEffect), 400, ypos+=30);
        ofDrawBitmapString("TAB to change view, F to toggle full screen", 400, ypos+=30);
        ofDrawBitmapString("Left and Right Arrows to change current effect", 400, ypos+=30);
        ofDrawBitmapString("[ and ] to sync to camera", 400, ypos+=30);
        
        
        ofNoFill();
        ofSetLineWidth(2);
        mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
        ofPushMatrix();
        ofTranslate(laserManager.getPreviewOffset());
        ofScale(laserManager.getPreviewScale());
        
        mesh.draw();
        ofPopMatrix();
    }
}


void ofApp :: showLaserEffect(int effectnum) {
    
    
	float left = laserManager.width*0.1;
	float top = laserManager.height*0.1;
	float right = laserManager.width*0.9;
	float bottom = laserManager.height*0.9;
	float width = laserManager.width*0.8;
	float height = laserManager.height*0.8;
	
	switch (currentLaserEffect) {
			
		case 0: {
			
			points.clear();
			for(int i = 0; i<pointsPerFrame; i++) {
				ofxLaser::Point p;
				p.set(400, 400);
				int r, g, b;
				r = g = b = 0;
				p.setColour(r,g,b);
				points.push_back(p);
			}
			break;
		}
			
		case 1: {

			points.clear();
			
			for(int i = 0; i<pointsPerFrame; i++) {
				ofxLaser::Point p;
				p.set(400, 400);
				int r, g, b;
				r = g = b = 255;
				if(i<startOffset) r = b = 0;
				else if(i>pointsPerFrame-endOffset) g=b=0;
				if(abs(i-(pointsPerFrame/2))<50) r = g = b = 0;
				p.setColour(r,g,b);
				points.push_back(p);
			}
			if(ofGetKeyPressed('[')) {
				points.pop_back();
				points.pop_back();
				points.pop_back();
				points.pop_back();
				points.pop_back();
			} else if(ofGetKeyPressed(']')) {
				points.push_back(points.back());
				points.push_back(points.back());
				points.push_back(points.back());
				points.push_back(points.back());
				points.push_back(points.back());
			}
				
			
      		break;

		}
		
			
		case 2: {
			
			points.clear();
			
			for(int i = 0; i<pointsPerFrame; i++) {
				ofxLaser::Point p;
				p.set(400, 400);
				int r, g, b;
				r = g = b = 255;
				if(i<startOffset) r = g = b = 0;
				else if(i>pointsPerFrame-endOffset) r=g=b=0;
				if(i%100<90) r = b = g = 0;
				p.setColour(r,g,b);
				points.push_back(p);
			}
			
			if(ofGetKeyPressed('[')) {
				points.pop_back();
			} else if(ofGetKeyPressed(']')) {
				points.push_back(points.back());
			}
			
			break;
			
		}
		
			
		case 3: {
			// beam flying right to left
			points.clear();
			
			float progress = ofMap((int)(elapsedTime*1000)%1000,0,1000,0,2);
			
			int beampos;
			int beamwidth = ofMap(progress, 0, 1, 20, 40);
			
			
			progress = Quad::easeIn(progress, 0, 1, 1);
			beampos = ofMap(progress, 0, 1,startOffset, pointsPerFrame-endOffset);
		
			
			for(int i = 0; i<pointsPerFrame; i++) {
				ofxLaser::Point p;
				p.set(400, 400);
				int r, g, b;
				r = g = b = 0;
				if(abs(i-beampos)<beamwidth) r = g = b = 255;
				if(i<startOffset) r = g = b = 0;
				else if(i>pointsPerFrame-endOffset) r=g=b=0;
				//if(i%100<90) r = b = g = 0;
				p.setColour(r,g,b);
				points.push_back(p);
			}
			
			if(ofGetKeyPressed('[')) {
				points.pop_back();
			} else if(ofGetKeyPressed(']')) {
				points.push_back(points.back());
			}
			
			

			
			break;
			
		}
			
		case 4: {
			// beam bouncing
			points.clear();
			
			float progress = ofMap((int)(elapsedTime*1000)%1000,0,1000,0,2);
			
			int beampos;
			if(progress<1) {
				progress = Cubic::easeInOut(progress, 0, 1, 1);
				beampos = ofMap(progress, 0, 1,startOffset, pointsPerFrame-endOffset);
			} else {
				progress = Cubic::easeInOut(progress-1, 0, 1, 1);
				beampos = ofMap(progress, 0, 1, pointsPerFrame-endOffset, startOffset);
			}
			for(int i = 0; i<pointsPerFrame; i++) {
				ofxLaser::Point p;
				p.set(400, 400);
				int r, g, b;
				r = g = b = 0;
				if(abs(i-beampos)<20) r = g = b = 255;
				if(i<startOffset) r = g = b = 0;
				else if(i>pointsPerFrame-endOffset) r=g=b=0;
				//if(i%100<90) r = b = g = 0;
				p.setColour(r,g,b);
				points.push_back(p);
			}
			
			if(ofGetKeyPressed('[')) {
				points.pop_back();
			} else if(ofGetKeyPressed(']')) {
				points.push_back(points.back());
			}
			
			
			
			break;
			
		}
			
		case 5: {
			
			//beam flying out and stopping
			points.clear();
			
			float progress = elapsedTime-startTime;
			
			int beampos;
			int beamwidth = 30;
			
			
			
			beampos = ofMap(progress, 0, 0.2,startOffset, (pointsPerFrame-endOffset-startOffset)*beamPos+startOffset, true);
			glm::vec3 random (ofSignedNoise(elapsedTime*20), ofSignedNoise((elapsedTime-startTime)*20), 0);
			
			for(int i = 0; i<pointsPerFrame; i++) {
				ofxLaser::Point p;
				p.set(400, 400);
				p+=(random*20);
				int r, g, b;
				r = g = b = 0;
				if(abs(i-beampos)<beamwidth) r = g = b = 255;
				if(i<startOffset) r = g = b = 0;
				else if(i>pointsPerFrame-endOffset) r=g=b=0;
				//if(i%100<90) r = b = g = 0;
				p.setColour(r,g,b);
				points.push_back(p);
			}
			
			if(ofGetKeyPressed('[')) {
				points.pop_back();
			} else if(ofGetKeyPressed(']')) {
				points.push_back(points.back());
			}
			
			if(ofGetKeyPressed('[')) {
				points.pop_back();
			} else if(ofGetKeyPressed(']')) {
				points.push_back(points.back());
			}
			
			
			break;
			
		}
		case 6: {
            points.clear();
			for(int i = 0; i<pointsPerFrame; i++) {
				ofxLaser::Point p;
				p.set(400, 400);
				float angle = ofMap(i, 0, pointsPerFrame, 0,PI*4);
				
				p.x+=cos(angle)*100;
				p.y+=sin(angle)*100;
				int r, g, b;
				r = g = b = 255;
//				if(abs(i-beampos)<beamwidth) r = g = b = 255;
//				if(i<startOffset) r = g = b = 0;
//				else if(i>pointsPerFrame-endOffset) r=g=b=0;
//				//if(i%100<90) r = b = g = 0;
				p.setColour(r,g,b);
				points.push_back(p);
			}
			break;
			
		}
	}


	//laser.sendRawPoints(points);

}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == OF_KEY_LEFT) {
		currentLaserEffect--;
		if(currentLaserEffect<0) currentLaserEffect = numLaserEffects-1;
		startTime = elapsedTime;
	} else if (key == OF_KEY_RIGHT) {
		currentLaserEffect++;
		if(currentLaserEffect>=numLaserEffects) currentLaserEffect = 0;
		startTime = elapsedTime;
	}
    if(key==OF_KEY_TAB) {
        laserManager.selectNextLaser();
    }
}

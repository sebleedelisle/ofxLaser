#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
	
	laserWidth = 800;
	laserHeight = 800;
	laserManager.setCanvasSize(laserWidth, laserHeight);
	
    numLaserEffects = 9;
    
	// if you don't want to manage your own GUI for your
    // app you can add extra params to the laser GUI
    laserManager.addCustomParameter(renderProfileLabel);
    laserManager.addCustomParameter(renderProfileIndex.set("Render Profile", 0, 0, 2));
    
    laserManager.addCustomParameter(currentLaserEffect.set("Current effect", 0, 0, numLaserEffects-1));
    laserManager.addCustomParameter(timeSpeed.set("Animation speed", 1, 0, 2));
   
	laserManager.addCustomParameter(colour.set("Colour", ofColor(0, 255, 0), ofColor(0), ofColor(255)));
    
    ofParameter<string> description;
    description.setName("description"); 
    description.set("INSTRUCTIONS : \nTAB to toggle output editor \nLeft and Right Arrows to change current effect \nMouse to draw polylines \nC to clear");
    laserManager.addCustomParameter(description);
    
    currentLaserEffect = 0;
     
}

//--------------------------------------------------------------
void ofApp::update(){
    
	float deltaTime = ofClamp(ofGetLastFrameTime(), 0, 0.2);
	elapsedTime += (deltaTime*timeSpeed);
    
    // prepares laser manager to receive new points
    laserManager.update();
    
}


void ofApp::draw() {
	
	ofBackground(15,15,20);
    
    showLaserEffect(currentLaserEffect);

    // sends points to the DAC
    laserManager.send();

    laserManager.drawUI();


}


void ofApp :: showLaserEffect(int effectnum) {
    
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
    
    
	float left = laserWidth*0.1;
	float top = laserHeight*0.1;
	float right = laserWidth*0.9;
	float bottom = laserHeight*0.9;
	float width = laserWidth*0.8;
	float height = laserHeight*0.8;
	
	switch (currentLaserEffect) {
			
		case 1: {

			// LASER LINES
			int numlines = 7;
			
			for(int i = 0; i<numlines; i++) {
				
				float progress =(float)i/(float)(numlines-1);
                float hue =(float)i/(float)(numlines);
                
				float xpos =left + (width*progress);
									
				laserManager.drawLine(ofPoint(xpos, top+height*0.1), ofPoint(xpos, top+height*0.4), ofColor(255), renderProfile);
              
				ofColor c;
				c.setHsb(hue*255, 255, 255);
                
				laserManager.drawLine(ofPoint(xpos, top+height*0.6), ofPoint(xpos, top+height*0.9), c, renderProfile);
		
			}

      break;

		}
		
			
		case 2: {
			
			// LASER LINES ANIMATING
			int numlines = 7;
			
			for(int i = 0; i<numlines; i++) {
				
				float progress =(float)i/(float)(numlines-1);
                float hue =(float)i/(float)(numlines);
                
				float xpos =left + (width*progress) + (sin(elapsedTime*4+i*0.5)*width*0.05);
				
				laserManager.drawLine(ofPoint(xpos, top+height*0.1), ofPoint(xpos, top+height*0.4), ofColor(255), renderProfile);
				ofColor c;
				c.setHsb(hue*255, 255, 255);
				laserManager.drawLine(ofPoint(xpos, top+height*0.6), ofPoint(xpos, top+height*0.9), c, renderProfile);
				
			}
			
			break;
			
		}
		
			
		case 3: {
			
			// LASER CIRCLES
			int numCircles = 4;
			
			for(int i = 0; i<numCircles; i++) {
				
				float progress =(float)i/(float)(numCircles-1);
                float hue =(float)i/(float)(numCircles);
                
				float xpos =left + (width*progress);
				
				laserManager.drawCircle(ofPoint(xpos, top+height*0.3),30, ofColor(255), renderProfile);
				ofColor c;
				c.setHsb(hue*255, 255, 255);
				
				laserManager.drawCircle(ofPoint(xpos, top+height*0.7), 30, c, renderProfile);
				
			}
			
			break;
			
		}
			
		case 4: {
			
			// LASER CIRCLES ANIMATING
			int numCircles = 4;
			
			for(int i = 0; i<numCircles; i++) {
				
				float progress =(float)i/(float)(numCircles-1);
                float hue =(float)i/(float)(numCircles);
				
				float xpos =left + (width*progress) + (sin(elapsedTime*4+i*0.5)*width*0.05);
				
				laserManager.drawCircle(ofPoint(xpos, top+height*0.3), 30, ofColor::white, renderProfile);
				ofColor c;
				c.setHsb(hue*255, 255, 255);
				
				laserManager.drawCircle(ofPoint(xpos, top+height*0.7), 30, c, renderProfile);
				
			}
			
			break;
			
		}
			
		case 5: {
			
			// LASER PARTICLES
			int numParticles = 12;
			
			for(int i = 0; i<numParticles; i++) {
				
				float progress =(float)i/(float)(numParticles-1);
				
				float xpos =left + (width*progress) ;
				
				laserManager.drawDot(ofPoint(xpos, top+height*0.3), ofColor(255),1, renderProfile);
				ofColor c;
				c.setHsb(progress*255, 255, 255);
				laserManager.drawDot(ofPoint(xpos, top+height*0.7), c, 1,  renderProfile);
				
			}
			
			break;
			
		}
		case 6: {
			
			// LASER PARTICLES ANIMATING
			
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
                
				laserManager.drawDot(p, c, 1, renderProfile);
				
			}
			
			break;
			
		}
        case 7: {
            
            // 3D rotation
            // you don't need to wrap your draw calls in
            // laser.beginDraw() and laser.endDraw() unless
            // you're doing 3D (it fixes the viewport perspective)
            
            laserManager.beginDraw();
            
            float speed = 20;
            ofPushMatrix();
            ofTranslate(laserWidth/2,laserHeight/2);
            ofRotateYDeg(elapsedTime*speed);
            int hue = (int)(elapsedTime*32)%255; // 8 seconds to go around
            ofColor c;
            c.setHsb(hue, 255, 255);
            
            ofPolyline poly;
            
            for(int j = 0; j<4; j++) {
                poly.clear();
                ofPushMatrix();
                ofRotateXDeg(j*90);
                
                poly.addVertex(glm::vec3(100,-100,100));
                poly.addVertex(glm::vec3(100, 100,100));
                poly.addVertex(glm::vec3(-100, 100,100));
                poly.addVertex(glm::vec3(-100, -100,100));
                laserManager.drawPoly(poly, c, renderProfile);
               
                ofPopMatrix();
            }
            ofPopMatrix();
            
            laserManager.endDraw();
          
            
            break;
            
        }
			
	}

	// LASER POLYLINES
	for(size_t i = 0; i<polyLines.size(); i++) {
		laserManager.drawPoly(polyLines[i], colour, renderProfile );
	}
	
}


//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs& e){
	if(e.key =='c') {
		polyLines.clear();
	} else if (e.key == OF_KEY_LEFT) {
		currentLaserEffect--;
		if(currentLaserEffect<0) currentLaserEffect = numLaserEffects-1;
	} else if (e.key == OF_KEY_RIGHT) {
		currentLaserEffect++;
		if(currentLaserEffect>=numLaserEffects) currentLaserEffect = 0;
	}
    if(e.key==OF_KEY_TAB) {
        laserManager.selectNextLaser();
       
    }
   
}

//--------------------------------------------------------------
void ofApp::mouseDragged(ofMouseEventArgs& e){
	if(!drawingShape) return;
	
    glm::vec2 mouse = laserManager.screenToLaserInput(e);
    
    
	ofPolyline &poly = polyLines.back();
	poly.addVertex((ofPoint)mouse);

}

//--------------------------------------------------------------
void ofApp::mousePressed(ofMouseEventArgs& e){
    if(laserManager.viewMode == OFXLASER_VIEW_CANVAS) {
        polyLines.push_back(ofPolyline());
        drawingShape = true;
    }
}

void ofApp::mouseReleased(ofMouseEventArgs& e) {
	if(drawingShape) {
		ofPolyline &poly = polyLines.back();
		poly = poly.getSmoothed(2);
		drawingShape = false;
	}
	
}

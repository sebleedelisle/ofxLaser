#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
    
    // NOTE that you no longer need to set up anything at all in code!
    // If you want to change the size of the laser area, use
    // laser.setCanvasSize(int width, int height) - default is 800 x 800.
    ofSetVerticalSync(false); 
    starPoly = makeStarPolyline(4, 100);
    starPolyInner = makeStarPolyline(4, 50);

    auto vertices = starPoly.getVertices();
    int hue = 0;
    for(auto& v : vertices) {
        starPolyColours.push_back(ofColor::fromHsb(hue%255, 255,255));
        hue+=20;
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // prepares laser manager to receive new graphics
    laser.update();
}


void ofApp::draw() {
    
    ofBackground(5,5,10);
   
    // Drawing a laser line, provide the start point, end point and colour.
    // (you can also use glm::vec3). The final parameter is the render profile.
    // Three profiles are provided, default, fast and detail.
    laser.pushMatrix();
    //laser.translate(ofGetMouseX(), ofGetMouseY());
    laser.drawLine(200, 200, 600, 200, ofColor::white, OFXLASER_PROFILE_FAST);
    laser.popMatrix();
    
    // Draw dots. Note that if you leave out the render profile, it
    // will use the default profile. If there is haze in the room, this is
    // how you make laser beams

    int numDots = 6;
    for(int x = 0; x<numDots; x++) {
        // intensity is a unit value (0 - 1) that determines how long the laser lingers
        // to make the dot. It defaults to 1.
        float intensity = (float)(x+1)/numDots;
        float xposition = ofMap(x, 0,numDots-1,200,600);
        laser.drawDot(xposition, 600, ofColor::red, intensity, OFXLASER_PROFILE_FAST);
    }
    
    //laser.drawCircle(275, 400, 80, ofColor(0,50,255), OFXLASER_PROFILE_FAST);
    laser.setFill(true); // when true, subtracts the center of polys and circles!
    
    // note that matrix transformations work as well, even in 3D!
    laser.pushMatrix();
    laser.translate(500,400);
    //laser.scale(2);
    //ofLogNotice() <<ofGetMouseX();
    
    laser.pushMatrix();
    // 3D rotation around the y axis
    laser.rotateYRad(ofGetElapsedTimef());
    
    //laser.drawPoly(starPoly, starPolyColours, OFXLASER_PROFILE_DEFAULT);
    //laser.drawPoly(starPolyInner, starPolyColours, OFXLASER_PROFILE_DEFAULT);
    vector<vector<ofColor>> colours = {starPolyColours, starPolyColours};
    laser.drawPolys({starPoly, starPolyInner}, colours, OFXLASER_PROFILE_DEFAULT);
    
    
    
    laser.popMatrix();
    
    laser.rotateYRad(ofGetElapsedTimef()*-0.83);
    laser.translate(100,0,0);
    laser.rotateYRad(ofGetElapsedTimef()*0.83); // so that circle always faces front
    laser.drawCircle(0,0,10,ofColor::cyan, OFXLASER_PROFILE_FAST);
    
    laser.popMatrix();
    
    
    glm::vec2 mousepos(ofGetMouseX(), ofGetMouseY());
    mousepos = laser.screenToLaserInput(mousepos);
//
    laser.drawCircle(glm::vec3(250, 400,0), 60, ofColor(0,50,255), OFXLASER_PROFILE_FAST);
   
    laser.drawCircle(glm::vec3(mousepos.x, mousepos.y,1), 10, ofColor(0,50,255), OFXLASER_PROFILE_FAST);
   // laser.drawDot(glm::vec3(mousepos.x+55, mousepos.y, 10), ofColor::red);
   
//
////
//    for(int i = 0; i<360; i+=90) {
//        laser.pushMatrix();
//
//        laser.translate(400,400);
//
//        laser.rotateYDeg(i+ofGetMouseX() + (ofGetElapsedTimef()*10));
//        laser.translate(150, 0);
//        //laser.rotateZDeg(ofGetElapsedTimef()*60 + i/10);
//        //laser.rotateZDeg(i/10);
////        laser.drawCircle(0,0,60, ofColor(0,50,255), OFXLASER_PROFILE_FAST);
//        laser.drawPoly(starPoly, starPolyColours, true, OFXLASER_PROFILE_DEFAULT);
//        laser.popMatrix();
//
//
//    }
//
//
    // sends points to the DAC
    laser.send();
    
    // draw the laser UI elements
    laser.drawUI();
    
}


ofPolyline ofApp::makeStarPolyline(int numSides, float size) {
    ofPolyline poly;
    // create a star polyline object
    float angle = 360.0f / (float)numSides /2.0f;
    for(int i = 0; i<numSides; i++) {
        poly.rotateDeg(angle, glm::vec3(0,0,1));
        poly.addVertex(-size,0);
        //if(i==numSides) continue;
        poly.rotateDeg(angle, glm::vec3(0,0,1));
        poly.addVertex(-size*0.3,0);
    }
    poly.close();
    return poly;
}

//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs& e){
    
    if(e.key==OF_KEY_TAB) {
        laser.selectNextLaser();
    }// if(e.key == ' ') {
     //   testscale = !testscale;
    //}
    
}

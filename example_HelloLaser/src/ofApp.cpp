#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
    
    // NOTE that you no longer need to set up anything at all in code!
    // If you want to change the size of the laser area, use
    laser.setCanvasSize(900, 800); //  - default is 800 x 800.
    ofSetVerticalSync(false);
    starPoly = makeStarPolyline(4, 100);
    starPolyInner = makeStarPolyline(4, 50);

    auto vertices = starPoly.getVertices();
    int hue = 0;
    for(auto& v : vertices) {
        starPolyColours.push_back(ofColor::fromHsb(hue%255, 255,255));
        hue+=20;
    }
    laser.globalLatency = 300;

}

//--------------------------------------------------------------
void ofApp::update(){
    
    // prepares laser manager to receive new graphics
    laser.update();
}


void ofApp::draw() {
    
    ofBackground(5,5,10);
    
//    laser.useClipRectangle = true;
//    laser.clipRectangle.set(ofGetMouseX(),ofGetMouseY(),200,200);
   
    // Drawing a laser line, provide the start point, end point and colour.
    // (you can also use glm::vec3). The final parameter is the render profile.
    // Three profiles are provided, default, fast and detail.
    laser.pushMatrix();
    //laser.translate(ofGetMouseX(), ofGetMouseY());
  //  laser.drawLine(200, 200, 600, 200, ofColor::white, OFXLASER_PROFILE_FAST);
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

    laser.setFill(fillShapes); // when true, subtracts the center of polys and circles!
    
    // note that matrix transformations work as well, even in 3D!
    laser.pushMatrix();
    laser.translate(500,400);

    laser.pushMatrix();
    // 3D rotation around the y axis
    laser.rotateYRad(ofGetElapsedTimef());
   
    vector<vector<ofColor>> colours = {starPolyColours, starPolyColours};
    //laser.drawPoly(starPoly, starPolyColours, OFXLASER_PROFILE_DEFAULT);
    laser.drawPolys({starPoly, starPolyInner}, colours, OFXLASER_PROFILE_DEFAULT);
    
    laser.popMatrix();
    
    laser.rotateYRad(ofGetElapsedTimef()*-0.83);
    laser.translate(100,0,0);
    laser.rotateYRad(ofGetElapsedTimef()*0.83); // so that circle always faces front
    laser.drawCircle(0,0,10,ofColor::cyan, OFXLASER_PROFILE_FAST);
    
    laser.popMatrix();
    
    glm::vec2 mousepos(ofGetMouseX(), ofGetMouseY());
    mousepos = laser.screenToLaserInput(mousepos);
    laser.pushMatrix();
    laser.translate(400,420);
    laser.rotateDeg(-100,1,0,0);
    laser.drawCircle(glm::vec3((mousepos.x-400), (mousepos.y-400),0), 60, ofColor(0,50,255), OFXLASER_PROFILE_FAST);
    laser.popMatrix();

    laser.drawCircle(glm::vec3(mousepos.x, mousepos.y,1), 10, ofColor(0,50,255), OFXLASER_PROFILE_FAST);
  
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
    }
    if(e.key =='f') fillShapes = !fillShapes; 
    
}

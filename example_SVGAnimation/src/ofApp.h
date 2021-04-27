#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"
#include "SVGLoader.h"

class ofApp : public ofBaseApp{

	public:
    void setup();
    void update();
    void draw();

    

    void keyPressed(int key);
    
    
    ofxLaser::Manager laser;
    SVGLoader svgLoader; 
    
    
};

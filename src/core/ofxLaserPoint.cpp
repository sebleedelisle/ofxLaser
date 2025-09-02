

//
//  ofxLaserPoint.h
//
//  Created by Seb Lee-Delisle on 20/05/2021.
//
//

#include "ofxLaserPoint.h"
using namespace ofxLaser;

ofxLaser :: Point :: Point() {
    x = y = z = 0;
    r = g = b = 0;
    useCalibration = true;
    //intensity = 1;
    
}
ofxLaser :: Point :: Point(ofPoint p, ofColor c, bool usecalibration ) {
    
    x = p.x;
    y = p.y;
    z = p.z;
    r = c.r;
    g = c.g;
    b = c.b;
    useCalibration = usecalibration;
    
}

ofColor ofxLaser :: Point :: getColour() {
    return ofColor(r, g, b);
}
void ofxLaser :: Point :: copyColourFromPoint(Point& pointToCopyFrom) {
    r = pointToCopyFrom.r;
    g = pointToCopyFrom.g;
    b = pointToCopyFrom.b;
}
void ofxLaser :: Point :: setColour(int pr, int pg, int pb) {
    r = pr;
    g = pg;
    b = pb;
}
void ofxLaser :: Point :: multiplyColour(float value) {
    r*=value;
    g*=value;
    b*=value;
    
}

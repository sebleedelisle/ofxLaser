//
//  ofxLaserPoint.h
//
//  Created by Seb Lee-Delisle on 13/08/2015.
//

#pragma once
#include "ofMain.h"

namespace ofxLaser {
class Point : public ofPoint {

    // TODO remove intensity - I don't think it's used
    
public:
	
	Point() {
		x = y = z = 0;
		r = g = b = 0;
		useCalibration = true;
		//intensity = 1;
		
	}
	Point(ofPoint p, ofColor c, bool usecalibration = true ) {
		
		x = p.x;
		y = p.y;
		z = p.z;
		r = c.r;
		g = c.g;
		b = c.b;
		useCalibration = usecalibration;
		
	}
	
	ofColor getColour() {
		return ofColor(r, g, b);
	}
	void copyColourFromPoint(Point& pointToCopyFrom) {
		r = pointToCopyFrom.r;
		g = pointToCopyFrom.g;
		b = pointToCopyFrom.b;
	}
	void setColour(int pr, int pg, int pb) {
		r = pr;
		g = pg;
		b = pb;
	}
    void multiplyColour(float value) {
        r*=value;
        g*=value;
        b*=value;
        
    }
	
	float r; //0-255
	float g;
	float b;
	
	//float intensity;
	bool useCalibration;
	

};
}

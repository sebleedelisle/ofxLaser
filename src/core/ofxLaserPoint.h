//
//  ofxLaserPoint.h
//
//  Created by Seb Lee-Delisle on 13/08/2015.
//

#pragma once
#include "ofMain.h"

namespace ofxLaser {
class Point : public ofPoint {

public:
	
	Point() {
		x = y = z = 0;
		r = g = b = 0;
		useCalibration = true;
		intensity = 1;
		
	}
	Point(ofPoint p, ofColor c, float pointIntensity = 1, bool usecalibration = true ) {
		
		x = p.x;
		y = p.y;
		z = p.z;
		r = c.r;
		g = c.g;
		b = c.b;
		useCalibration = usecalibration;
		intensity = pointIntensity;
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
	
	float r; //0-255
	float g;
	float b;
	
	float intensity;
	bool useCalibration;
	

};
}

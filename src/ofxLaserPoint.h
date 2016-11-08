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
	
	ofColor getColor() {
		return ofColor(r, g, b);
	}
	
	float r;
	float g;
	float b;
	
	float intensity;
	bool useCalibration;
	

};
}
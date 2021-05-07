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
	
    Point();
    Point(ofPoint p, ofColor c, bool usecalibration = true ) ;
	
    ofColor getColour();
    void copyColourFromPoint(Point& pointToCopyFrom);
    void setColour(int pr, int pg, int pb);
    void multiplyColour(float value) ;
	
	float r; //0-255
	float g;
	float b;
	
	//float intensity;
	bool useCalibration;
	

};
}

//
//  LaserDot.h
//
//  Created by Seb Lee-Delisle on 01/08/2013.
//
//

#pragma once

#include "ofxLaserShape.h"

namespace ofxLaser {
class Dot : public Shape{
	
	public :
	
	Dot(const ofPoint& dotPosition, ofFloatColor& dotColour, float dotIntensity = 1, int dotMaxPoints = 5){
		set(dotPosition, dotColour, dotIntensity);
		maxPoints = dotMaxPoints;
	}
	
	void set(const ofPoint& dotPosition, ofFloatColor& dotColour, float dotIntensity = 1) {
		colour = dotColour;
		startPos.set(dotPosition);
		endPos.set(dotPosition);
		intensity = dotIntensity; 
		tested = false;
	}
	
	void appendPointsToVector(vector<ofxLaser::Point>& points) {
		int particlecount = maxPoints * intensity;// ceil(dotMaxPoints* dot->intensity);
		
		for(int i = 0; i<particlecount; i++) {
			//addIldaPoint(dot.getStartPos(), dot.colour);
			points.push_back(ofxLaser::Point(getStartPos(), colour));
		}
	};

		
	float intensity = 1;
	int maxPoints = 5;
	ofFloatColor colour;
		
};

}

//
//  LaserLine.h
//
//  Created by Seb Lee-Delisle on 01/08/2013.
//
//

#pragma once

#include "LaserShape.h"

class LaserLine : public LaserShape{
	
	public :
	
	LaserLine(const ofPoint& startpos, const ofPoint& endpos, ofFloatColor& col, float lineSpeed, float lineAcceleration){
	
		reversable = true; 
		colour = col;
		
		startPos = startpos;
		endPos = endpos;
		
		tested = false;
		
		speed = lineSpeed;
		acceleration = lineAcceleration;
	//cout<<"LINE SET : " << startPos << " " << endPos << " " <<endl;
		
	}
	
	
	void appendPointsToVector(vector<ofxLaserPoint>& points) {
		
		ofPoint& start = getStartPos();
		ofPoint& end = getEndPos();
		ofVec2f v = end-start;

		float distanceTravelled = ofDist(start.x, start.y, end.x, end.y);
		vector<float> unitDistances = getPointsAlongDistance(distanceTravelled, acceleration, speed);
		
		ofPoint p;
		ofColor segmentColour;
		
		for(int i = 0; i<unitDistances.size(); i++) {
			
			float unitDistance = unitDistances[i];
			
			points.push_back(ofxLaserPoint(start + (v*unitDistance), colour));
		}
		
		
		
		
		
	};
	
	ofFloatColor colour;
	float speed;
	float acceleration;
		
};


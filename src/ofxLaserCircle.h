//
//  LaserCircle.h
//
//  Created by Seb Lee-Delisle on 01/08/2013.
//
//

#pragma once

#include "ofxLaserShape.h"

namespace ofxLaserX {
class Circle : public Shape{
	
	public :
	
	Circle(const ofPoint& circleCentre, float circleRadius, ofFloatColor circleColour,  float circleSpeed, float circleAcceleration, float overlap = 0){
		reversable = true;
		//set(position, colour, circleRadius, circleIntensity, overlap);
	
	
		
		
		pos = circleCentre;
		radius = circleRadius;
		colour = circleColour;
		speed = circleSpeed;
		acceleration = circleAcceleration;
		
		if(overlap>0) {
			
			float circumference = PI*2*radius;
			overlapDistance = overlap;
			overlapAngle = overlap / circumference * 360;
			
			startPos.set(0,-radius);
			endPos = startPos;
			startPos.rotate(-overlapAngle/2, ofPoint(0,0,1));
			startPos+=pos;
			endPos.rotate(overlapAngle/2, ofPoint(0,0,1));
			endPos+=pos;
			
			
		} else {
			
			startPos = pos;
			startPos.y-=radius;
			
			endPos = startPos;
			
		}
				
	
	}
	
	void appendPointsToVector(vector<ofxLaserX::Point>& points) {
		
		
		float distanceTravelled = 2 * PI * radius + overlapDistance;
		vector<float> unitDistances = getPointsAlongDistance(distanceTravelled, acceleration, speed);
		
		ofPoint p;
		ofColor segmentColour;
		
		for(int i = 0; i<unitDistances.size(); i++) {
			
			float unitDistance = unitDistances[i];
			float angle;
			if(!reversed) angle = ofMap(unitDistance,0,1,-overlapAngle/2,360+(overlapAngle/2)) ;
			else angle = ofMap(unitDistance,1,0,-overlapAngle/2,360+(overlapAngle/2)) ;
			
			segmentColour = colour;
			
			// an attempt to fade out the overlap. Not sure if it works.
			// TODO check that it works!
			
			if(angle<overlapAngle/2) {
				segmentColour*= ofMap(angle, -overlapAngle/2,overlapAngle/2, 0, 1);
			} if(angle> 360 - overlapAngle/2) {
				segmentColour *= ofMap(angle, 360 -overlapAngle/2,360 + overlapAngle/2, 1, 0);
			}
			
			p.set(pos);
			p.x+=sin(ofDegToRad(angle))*radius;
			p.y-=cos(ofDegToRad(angle))*radius;
			
			points.push_back(ofxLaserX::Point(p, colour));
		}
		
		
	};
	
	float overlapAngle = 0;
	float overlapDistance = 0; 
	ofFloatColor colour;
	
	ofPoint pos;
	float radius;
	float speed;
	float acceleration;
		
};
}

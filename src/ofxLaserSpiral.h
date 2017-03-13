//
//  LaserCircle.h
//
//  Created by Seb Lee-Delisle on 01/08/2013.
//
//

#pragma once

#include "ofxLaserShape.h"

namespace ofxLaser {
class Spiral : public Shape{
	
	public :
	
	Spiral(const ofPoint& spiralCentre, float spiralInnerRadius,float spiralOuterRadius, ofFloatColor spiralColour,  float spiralSpeed, float spiralSpacing){
        
		reversable = false; // could maybe change this later?
				
		pos = spiralCentre;
        innerRadius = spiralInnerRadius;
        outerRadius = spiralOuterRadius;
		colour = spiralColour;
		speed = spiralSpeed;
		spacing = spiralSpacing;
		

        startPos = pos;
        endPos = startPos;

        float revolutions = ((outerRadius - innerRadius)/spiralSpacing);
        
        ofVec2f offset(spiralOuterRadius, 0);
        
        float maxAngle = 360 * revolutions;
        offset.rotate(maxAngle);
        
        endPos+=offset;
       
	
					
	
	}
	
	void appendPointsToVector(vector<ofxLaser::Point>& points) {
		
        float revolutions = ((outerRadius - innerRadius)/spacing);
        float maxAngle = 360 * revolutions;
        float currentAngle = 0;
        
        //float speed = 0;
        //float maxSpeed = 20;
        
        //float rotateSpeed = 1;
        ofPolyline path;
        //path.setFilled(false);
        //path.setStrokeWidth(1);
        
        ofVec2f temppos = pos;
        
        // TODO - DRAW BACKWARDS if reversed!
        
        while (currentAngle<=maxAngle + 360) {
            
            temppos.set(ofMap(currentAngle, 0, maxAngle, innerRadius, outerRadius, true), 0);
            temppos.rotate(currentAngle);
            temppos+=pos;
            
            //rotateSpeed = ofMap(currentAngle, 1, 45,0,maxSpeed, true);
            // should be a setting!
            currentAngle+=1;
            path.addVertex(temppos);
            
            
            //addIldaPoint(pos, spiral.colour, spiral.intensity);
            
        }
        
        float totaldistance = path.getPerimeter();
        
        vector<float> unitDistances = getPointsAlongDistance(totaldistance, 10, speed);
        
        float fadeOutDistance = 1-((outerRadius*PI) / totaldistance); // fade out should be half a rev on the outside.
        
        for(int i = 0; i<unitDistances.size(); i++) {
            float unit = unitDistances[i];
            temppos = path.getPointAtLength(min(unit*totaldistance, totaldistance-0.01f));
 
            ofColor col = colour;
            if(i<5) col*=ofMap(i,0,5,0,1);
            else if(unit>=fadeOutDistance) col*=ofMap(unit, fadeOutDistance, 1, 1,0);
            points.push_back(ofxLaser::Point(temppos, col));
            
        }
        
        
        
//        
//		float distanceTravelled = 2 * PI * radius + overlapDistance;
//		vector<float> unitDistances = getPointsAlongDistance(distanceTravelled, acceleration, speed);
//		
//		ofPoint p;
//		ofColor segmentColour;
//		
//		for(int i = 0; i<unitDistances.size(); i++) {
//			
//			float unitDistance = unitDistances[i];
//			float angle;
//			if(!reversed) angle = ofMap(unitDistance,0,1,-overlapAngle/2,360+(overlapAngle/2)) ;
//			else angle = ofMap(unitDistance,1,0,-overlapAngle/2,360+(overlapAngle/2)) ;
//			
//			segmentColour = colour;
//			
//			// an attempt to fade out the overlap. Not sure if it works.
//			// TODO check that it works!
//			if(overlapAngle>0) {
//				if(angle<overlapAngle/2) {
//					segmentColour*= ofMap(angle, -overlapAngle/2,overlapAngle/2, 0, 1);
//				} if(angle> 360 - overlapAngle/2) {
//					segmentColour *= ofMap(angle, 360 -overlapAngle/2,360 + overlapAngle/2, 1, 0);
//				}
//			}
//			p.set(pos);
//			p.x+=sin(ofDegToRad(angle))*radius;
//			p.y-=cos(ofDegToRad(angle))*radius;
//			
//			points.push_back(ofxLaser::Point(p, colour));
//		}
		
		
	};
	
	ofFloatColor colour;
	
	ofPoint pos;
	float innerRadius;
    float outerRadius;
    float speed;
    float spacing;
		
};
}

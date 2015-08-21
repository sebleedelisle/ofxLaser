//
//  LaserCircle.h
//
//  Created by Seb Lee-Delisle on 01/08/2013.
//
//

#pragma once

#include "ofxLaserShape.h"
#include "ofMain.h"

namespace ofxLaser {
	
class Polyline : public Shape{
	
	public :
	
	Polyline(const ofPolyline& sourcepoly, ofFloatColor polyColour, float polyAcceleration, float polySpeed, float polyCornerThresholdAngle){
		
		// to do - implement reversibility!
		reversable = false;
		
		polyline = sourcepoly; // makes a copy
		
		startPos = polyline.getVertices().front();
		// to avoid a bug in polyline in open polys
		endPos = polyline.getVertices().back();
		
		acceleration = polyAcceleration;
		speed = polySpeed;
		//previewMesh.setMode(OF_PRIMITIVE_LINES);
		cornerThresholdAngle = polyCornerThresholdAngle;
		
	}

	void appendPointsToVector(vector<ofxLaser::Point>& points) {
		
		ofPolyline& poly = polyline;
		int startpoint = 0;
		int endpoint = 0;

		while(endpoint<(int)poly.getVertices().size()-1) {

			do {
				endpoint++;
			} while ((endpoint< (int)poly.getVertices().size()-1) && abs(poly.getAngleAtIndex(endpoint)) < cornerThresholdAngle);


			float startdistance = poly.getLengthAtIndex(startpoint);
			float enddistance = poly.getLengthAtIndex(endpoint);

			float length = enddistance - startdistance;

			ofPoint lastpoint;

			if(length>0) {

				vector<float> unitDistances = getPointsAlongDistance(length, acceleration, speed);

				ofColor pointcolour;

				for(int i = 0; i<unitDistances.size(); i++) {
					ofPoint p = poly.getPointAtLength((unitDistances[i]*0.999* length) + startdistance);
					
					//pointcolour = laserpoly.getColourForPoint(unitDistances[i], p);

					//addIldaPoint(p, pointcolour, laserpoly.intensity);
					points.push_back(ofxLaser::Point(p, colour));
					// TODO reimplement preview mesh
//					if(i>0) {
//						laserpoly.previewMesh.addVertex(lastpoint);
//						laserpoly.previewMesh.addVertex(p);
//						laserpoly.previewMesh.addColor(pointcolour*laserpoly.intensity);
//						laserpoly.previewMesh.addColor(pointcolour*laserpoly.intensity);
//						//cout << lastpoint << " " << p << " " << pointcolour << endl;
//
//					}
					
					lastpoint = p;
				}
				
			}
			
			startpoint=endpoint;
			
		}

		
	}
	
//	
//	void drawLaserPolyline(LaserPolyline& laserpoly, bool onlyUpdatePreviewMesh) {
//		
//		ofPolyline& poly = laserpoly.polyline;
//		
//		laserpoly.previewMesh.clear();
//		laserpoly.previewMesh.setMode(OF_PRIMITIVE_LINES);
//		int startpoint = 0;
//		int endpoint = 0;
//		
//		while(endpoint<(int)poly.getVertices().size()-1) {
//			
//			do {
//				endpoint++;
//			} while ((endpoint< (int)poly.getVertices().size()-1) && abs(poly.getAngleAtIndex(endpoint)) < 30);
//			
//			
//			float startdistance = poly.getLengthAtIndex(startpoint);
//			float enddistance = poly.getLengthAtIndex(endpoint);
//			
//			float length = enddistance - startdistance;
//			
//			ofPoint lastpoint;
//			
//			if(length>0) {
//				
//				vector<float> unitDistances = getPointsAlongDistance(length, accelerationLine, speedLine);
//				
//				ofColor pointcolour;
//				
//				for(int i = 0; i<unitDistances.size(); i++) {
//					ofPoint p = poly.getPointAtLength((unitDistances[i]*0.999* length) + startdistance);
//					pointcolour = laserpoly.getColourForPoint(unitDistances[i], p);
//					
//					if(!onlyUpdatePreviewMesh) addIldaPoint(p, pointcolour, laserpoly.intensity);
//					
//					if(i>0) {
//						laserpoly.previewMesh.addVertex(lastpoint);
//						laserpoly.previewMesh.addVertex(p);
//						laserpoly.previewMesh.addColor(pointcolour*laserpoly.intensity);
//						laserpoly.previewMesh.addColor(pointcolour*laserpoly.intensity);
//						//cout << lastpoint << " " << p << " " << pointcolour << endl;
//						
//					}
//					lastpoint = p;
//				}
//				
//			}
//			
//			startpoint=endpoint;
//			
//		}
//		
//		
//	}
//

	
	
//	ofColor getColourForPoint(float distance, ofPoint& pos){
//		if(colourSystem!=NULL) {
//			return colourSystem->getColourForPoint(distance, pos);
//		} else {
//			return ofColor::white;
//		}
//	}
	
	
	~Polyline() {
		// not sure if there's any point clearing the polyline or the
		// previewMesh - they should just get destroyed, right?
		polyline.clear();
		// assumes that this object owns the colourSystem object.
		// Bit nasty.
		//if(colourSystem!=NULL) delete colourSystem;
		//previewMesh.clear();
		
	}
	
	ofPolyline polyline;
	//ofMesh previewMesh;
	
	ofFloatColor colour;
	float acceleration;
	float speed;
	float cornerThresholdAngle;
};
}

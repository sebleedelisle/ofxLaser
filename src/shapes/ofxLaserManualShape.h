//
//  ofxLaserManualShape.h
//
//  Created by Seb Lee-Delisle on 01/08/2013.
//
//

#pragma once

#include "ofxLaserShape.h"

namespace ofxLaser {
class ManualShape : public Shape{
	
	public :
	
	ManualShape(const vector<ofPoint> allpoints, const vector<ofColor> pointcolours, bool usecalibration, string profilelabel){
		
		useCalibration = usecalibration;
		
		startPos.set(allpoints.front());
		endPos.set(allpoints.back());
		
		points = allpoints; // hopefully copies
		colours = pointcolours;
		
		while(colours.size()<points.size()) {
			colours.push_back(colours.back());
		}
		
		tested = false;
		profileLabel = profilelabel;
		
	}
	void appendPointsToVector(vector<ofxLaser::Point>& destpoints, const RenderProfile& profile, float speedMultiplier) {
		
		for(size_t i = 0; i<points.size(); i++) {
			destpoints.push_back(ofxLaser::Point(points[i], colours[i], useCalibration));
		}
	};
	
	
	void addPreviewToMesh(ofMesh& mesh){
		for(size_t i = 0; i<points.size();i++) {
			mesh.addVertex(points[i]);
			mesh.addColor(colours[i]); 
			
		}
	}

		
	vector<ofPoint> points;
	vector<ofColor> colours;
	bool useCalibration;
		
};

}

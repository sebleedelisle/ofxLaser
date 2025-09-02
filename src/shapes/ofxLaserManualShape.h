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
	
	ManualShape(const vector<glm::vec3> allpoints, const vector<ofColor> pointcolours, bool usecalibration, string profilelabel){
		
		useCalibration = usecalibration;
		
		points = allpoints; // hopefully copies
		colours = pointcolours;
		
		tested = false;
		profileLabel = profilelabel;
		
	}
    
    virtual std::shared_ptr<ofxLaser::Shape> clone() const override {
        return std::make_shared<ManualShape>(points, colours, useCalibration, profileLabel);
    }
    
	void appendPointsToVector(vector<ofxLaser::Point>& destpoints, const RenderProfile& profile, float speedMultiplier) override {
		
		for(size_t i = 0; i<points.size(); i++) {
			destpoints.push_back(ofxLaser::Point(points[i], colours[i], useCalibration));
		}
	};
	
	
	void addPreviewToMesh(ofMesh& mesh) override {
		for(size_t i = 0; i<points.size();i++) {
			mesh.addVertex(points[i]);
			mesh.addColor(colours[i]); 
			
		}
	}
    
    bool clipNearPlane(float nearPlaneZ) override {
        return false;
    }
    
	vector<glm::vec3> points;
	vector<ofColor> colours;
	bool useCalibration;
		
};

}

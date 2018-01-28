//
//  LaserLine.h
//
//  Created by Seb Lee-Delisle on 01/08/2013.
//
//

#pragma once

#include "ofxLaserShape.h"


namespace ofxLaser {
class Line : public Shape{
	
	public :
	
	Line(const ofPoint& startpos, const ofPoint& endpos, const ofColor& col, string profilelabel){
	
		reversable = true; 
		colour = col;
		
		startPos = startpos;
		endPos = endpos;
		
		tested = false;
		profileLabel = profilelabel;
		
	}
	
	
	void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile) {
		
	
		ofPoint& start = getStartPos();
		ofPoint& end = getEndPos();
		ofVec2f v = end-start;

		float distanceTravelled = ofDist(start.x, start.y, end.x, end.y);
		vector<float> unitDistances = getPointsAlongDistance(distanceTravelled, profile.acceleration, profile.speed);
		
		ofPoint p;
		
		for(int i = 0; i<unitDistances.size(); i++) {
			
			float unitDistance = unitDistances[i];
			
			points.push_back(ofxLaser::Point(start + (v*unitDistance), colour));
		}
	};
	
	void addPreviewToMesh(ofMesh& mesh){
		mesh.addColor(ofColor::black);
		mesh.addVertex(getStartPos());
		
		mesh.addColor(colour);
		mesh.addVertex(getStartPos());
		mesh.addColor(colour);
		mesh.addVertex(getEndPos());
		
		mesh.addColor(ofColor::black);
		mesh.addVertex(getEndPos());
	}
	
	virtual bool intersectsRect(ofRectangle & rect) {
		return rect.intersects(startPos, endPos);
		
	};

		
};
}

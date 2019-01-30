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
	
	Dot(const ofPoint& dotPosition, const ofColor& dotColour, float dotIntensity, string profilelabel){
		
		colour = dotColour;
		startPos.set(dotPosition);
		endPos.set(dotPosition);
		intensity = dotIntensity;
		tested = false;
		profileLabel = profilelabel;
		
	}
	void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier) {
		int maxPoints = profile.dotMaxPoints; 
		int pointcount = ceil(maxPoints * intensity);// ceil(dotMaxPoints* dot->intensity);
		
		for(int i = 0; i<pointcount; i++) {
			//addIldaPoint(dot.getStartPos(), dot.colour);
			points.push_back(ofxLaser::Point(getStartPos(), colour));
		}
	};
	
	
	void addPreviewToMesh(ofMesh& mesh){
		float radius = ofMap(intensity, 0, 1,0.5,2);
		ofColor c(colour);
		c.setBrightness(ofMap(intensity,0,0.5,150,255,true));
		
		ofVec3f v(0,-radius);
		mesh.addColor(ofColor(0));
		mesh.addVertex(v + getStartPos());
		
		for(int i = 0; i<=360; i+=30) {
			v.set(0, -radius);
			v.rotate(i, ofVec3f(0,0,1));
			mesh.addColor(c);
			mesh.addVertex(v+getStartPos());
		}
		
		v.set(0, -radius);
		mesh.addColor(ofColor(0));
		mesh.addVertex(v+getStartPos());
	}
	
	virtual bool intersectsRect(ofRectangle & rect) {
		return rect.inside(startPos);
		
	};

		
	float intensity = 1;
	int maxPoints = 5;
		
};

}

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
	
    Dot(const ofPoint& dotPosition, const ofColor& dotColour, float dotIntensity, string profilelabel);
    void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier);
    void addPreviewToMesh(ofMesh& mesh);
    virtual bool intersectsRect(ofRectangle & rect);

		
	float intensity = 1;
	int maxPoints = 5;
		
};

}

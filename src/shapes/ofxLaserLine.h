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
	
    Line(const ofPoint& startpos, const ofPoint& endpos, const ofColor& col, string profilelabel);
	
	
	void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier);
	
    void addPreviewToMesh(ofMesh& mesh);
	
    virtual bool intersectsRect(ofRectangle & rect);

		
};
}

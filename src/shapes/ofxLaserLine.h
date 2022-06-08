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
	
    virtual Shape* clone() const override {
        return new Line(startPos, endPos, colour, profileLabel);
    }
    
	void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier) override;
	
    void addPreviewToMesh(ofMesh& mesh) override;
	
    virtual bool intersectsRect(ofRectangle & rect) override;

		
};
}

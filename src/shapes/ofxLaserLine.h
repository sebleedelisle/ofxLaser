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
	
    Line(glm::vec3 startpos, glm::vec3 endpos, ofColor col, string profilelabel);
	
    virtual Shape* clone() const override {
        glm::vec3 start = points.front();
        glm::vec3 end = points.front();
        ofColor c = colours.front();
        Line* line =new Line(start, end, c, profileLabel);
        line->setClipRectangle(clipRectangle);
        return line;
    }
    
	void appendPointsToVector(vector<ofxLaser::Point>& pointsToAppendTo, const RenderProfile& profile, float speedMultiplier) override;
	
    void addPreviewToMesh(ofMesh& mesh) override;
	
    virtual bool clipNearPlane(float nearPlaneZ) override;
    //virtual bool intersectsRect(ofRectangle & rect) override;

		
};
}

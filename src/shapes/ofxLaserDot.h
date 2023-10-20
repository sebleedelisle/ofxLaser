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
	
    Dot(const glm::vec3& dotPosition, const ofColor& dotColour, float dotIntensity, string profilelabel);
    void appendPointsToVector(vector<ofxLaser::Point>& pointsToAppendTo, const RenderProfile& profile, float speedMultiplier) override;
    
    virtual Shape* clone() const override {
        return new Dot(getStartPos(), getColour(), intensity, profileLabel);
    }

    void addPreviewToMesh(ofMesh& mesh) override;
    
    virtual bool clipNearPlane(float nearPlaneZ) override;
  
		
	float intensity = 1;
	int maxPoints = 5;
		
};

}

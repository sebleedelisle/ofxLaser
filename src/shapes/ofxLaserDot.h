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
    
    virtual std::shared_ptr<ofxLaser::Shape> clone() const override {
        std::shared_ptr<ofxLaser::Dot> dot = std::make_shared<Dot>(getStartPos(), getColour(), intensity, profileLabel);
        dot->setClipRectangle(clipRectangle);
        return dot;
    }

    void addPreviewToMesh(ofMesh& mesh) override;
    
    virtual bool clipNearPlane(float nearPlaneZ) override;
  
		
	float intensity = 1;
	int maxPoints = 5;
		
};

}

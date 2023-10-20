//
//  ofxLaserCircle.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 16/11/2017.
//
//

#pragma once

#include "ofxLaserPolyline.h"

namespace ofxLaser {
	class Circle :public Polyline {
	
		public:
		Circle(){};
		Circle(const glm::vec3& _centre, const float _radius, const ofColor& col, string profilelabel);
		
        virtual Shape* clone() const override {
            return new Circle(centre, radius, getColour(), profileLabel);
        }
        
//        void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier) override;
//
//        void addPreviewToMesh(ofMesh& mesh) override;
//
//        virtual bool clipNearPlane(float nearPlaneZ) override { return false; } ;
//
//        ofPolyline polyline; // to store the circle shape in once it's been projected
  
		protected:
		
	    float radius;
        glm::vec3 centre;
		
		private:

	};
}

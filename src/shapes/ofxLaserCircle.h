//
//  ofxLaserCircle.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 16/11/2017.
//
//

#pragma once

#include "ofxLaserShape.h"

namespace ofxLaser {
	class Circle :public Shape {
	
		public:
		Circle(){};
		Circle(const ofPoint& _centre, const float _radius, const ofColor& col, string profilelabel);
		
        virtual Shape* clone() const override {
            return new Circle(centre, radius, colour, profileLabel);
        }
        
        void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier) override;
		
		virtual bool intersectsRect(ofRectangle & rect)  override;
		
		void addPreviewToMesh(ofMesh& mesh) override;
        ofPolyline polyline; // to store the circle shape in once it's been projected
  
		protected:
		
	    float radius;
        glm::vec3 centre;
		
		private:

	};
}

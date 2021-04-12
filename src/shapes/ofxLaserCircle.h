//
//  ofxLaserCircle.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 16/11/2017.
//
//

#pragma once

#include "ofxLaserShape.h"
#include "ofxLaserManager.h"

namespace ofxLaser {
	class Circle :public Shape {
	
		public:
		Circle(){};
		Circle(const ofPoint& _centre, const float _radius, const ofColor& col, string profilelabel);
		void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier);
		
		virtual bool intersectsRect(ofRectangle & rect);
		
		void addPreviewToMesh(ofMesh& mesh);
		protected:
		
		ofPolyline polyline; // to store the circle shape in once it's been projected
        float radius;
        glm::vec3 centre;
		
		private:

	};
}

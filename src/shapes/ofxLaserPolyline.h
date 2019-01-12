//
//  ofxLaserPolyline.h
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 16/11/2017.
//
//

#pragma once
#include "ofxLaserManager.h"

namespace ofxLaser {
	class Polyline : public Shape {
	
		public :
		
		Polyline(const ofPolyline& poly, const ofColor& col, string profilelabel);
		Polyline(const ofPolyline& poly, const vector<ofColor>& colours, string profilelabel);
		
		
		~Polyline();
		
		void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier);
		
		void addPreviewToMesh(ofMesh& mesh);
		virtual bool intersectsRect(ofRectangle & rect);
		
		protected :
		void initPoly(const ofPolyline& poly);
		ofPolyline polyline;
		const RenderProfile* cachedProfile;
		std::vector<ofxLaser::Point> cachedPoints;
		std::vector<ofColor> colours;
		bool multicoloured; 
	};
}

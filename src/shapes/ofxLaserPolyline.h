//
//  ofxLaserPolyline.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 16/11/2017.
//
//

#pragma once
#include "ofxLaserFactory.h"
#include "ofxLaserShape.h"

namespace ofxLaser {
	class Polyline : public Shape {
	
		public :
		
		Polyline(); 
		
		Polyline(const ofPolyline& poly, const ofColor& col, string profilelabel);
		Polyline(const ofPolyline& poly, const vector<ofColor>& colours, string profilelabel);
		void init(const ofPolyline& poly, const ofColor& col, string profilelabel);
		void init(const ofPolyline& poly, const vector<ofColor>& colours, string profilelabel);
		
		
		~Polyline();
		
		void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier);
		
		void addPreviewToMesh(ofMesh& mesh);
		virtual bool intersectsRect(ofRectangle & rect);
		
		protected :
		void initPoly(const ofPolyline& poly);
		ofPolyline* polylinePointer = NULL;
		const RenderProfile* cachedProfile;
		std::vector<ofxLaser::Point> cachedPoints;
		std::vector<ofColor> colours;
		bool multicoloured;
		ofRectangle boundingBox; 
	};
}

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
		
        Polyline(const ofPolyline& poly, const ofColor& col, string profilelabel, float brightness = 1);
        Polyline(const ofPolyline& poly, const vector<ofColor>& colours, string profilelabel, float brightness = 1);
        Polyline(const vector<glm::vec3>& points, const vector<ofColor>& colours, string profilelabel, float brightness = 1);
		void init(const ofPolyline& poly, const ofColor& col, string profilelabel, float brightness);
        void init(const ofPolyline& poly, const vector<ofColor>& colours, string profilelabel, float brightness);
        void init(const vector<glm::vec3>& points, const vector<ofColor>& colours, string profilelabel, float brightness);
    
		
		~Polyline();
		
		void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier);
		
		void addPreviewToMesh(ofMesh& mesh);
		virtual bool intersectsRect(ofRectangle & rect);
        
        ofPolyline* polylinePointer = nullptr;

		protected :
        void initPoly(const ofPolyline& poly);
        void initPoly(const vector<glm::vec3> verticesToCopy);
        
		const RenderProfile* cachedProfile;
		std::vector<ofxLaser::Point> cachedPoints;
		std::vector<ofColor> colours;
		bool multicoloured;
		ofRectangle boundingBox; 
	};
}

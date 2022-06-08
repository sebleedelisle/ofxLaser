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
		
        virtual Shape* clone() const override {
            // todo check for nullptr 
            if(multicoloured)
                return new Polyline(*polylinePointer, colours, profileLabel);
            else
                return new Polyline(*polylinePointer, colour, profileLabel);
        }
        
		void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier) override;
		
		void addPreviewToMesh(ofMesh& mesh) override;
		virtual bool intersectsRect(ofRectangle & rect) override;
        
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

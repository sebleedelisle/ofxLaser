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
        Polyline(const vector<glm::vec3>& points, bool _closed, const vector<ofColor>& colours, string profilelabel, float brightness = 1);
        
        Polyline(const ofPolyline& poly, const vector<ofFloatColor>& colours, string profilelabel, float brightness = 1);
        Polyline(const vector<glm::vec3>& points, bool _closed, const vector<ofFloatColor>& colours, string profilelabel,  float brightness = 1);
       
        void reset(); 

		
		~Polyline();
		
        virtual std::shared_ptr<ofxLaser::Shape> clone() const override {
            std::shared_ptr<Polyline> p = std::make_shared<Polyline>(points, closed, colours, profileLabel);
            p->setFilled(filled);
            p->setClipRectangle(clipRectangle);
            return p;
            
        }
        
		void appendPointsToVector(vector<ofxLaser::Point>& pointsToAppendTo, const RenderProfile& profile, float speedMultiplier) override;
		
        void addPreviewToMesh(ofMesh& mesh) override;
        
        virtual bool clipNearPlane(float nearPlaneZ) override;
        
       //ofFloatColor& getColourAt(int index);

        //ofPolyline* polylinePointer = nullptr;

		protected :
        
//      void initPoly(const ofPolyline& poly);
//      void initPoly(const vector<glm::vec3> verticesToCopy);
        
//		const RenderProfile* cachedProfile;
//		std::vector<ofxLaser::Point> cachedPoints;

		
        
	};
}

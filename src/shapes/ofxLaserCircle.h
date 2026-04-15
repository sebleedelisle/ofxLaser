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
		
        virtual std::shared_ptr<ofxLaser::Shape> clone() const override {
            std::shared_ptr<Circle> circle =  std::make_shared<Circle>();
            circle->setPoints(points);
            circle->setColours(colours);
            circle->setFilled(isFilled());
            circle->setClosed(true); 
            circle->profileLabel = profileLabel ;
            circle->setClipRectangle(clipRectangle);
            return circle;
        }
        
		protected:
		
		private:

	};
}

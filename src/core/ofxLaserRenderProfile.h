//
//  ofxLaserRenderProfile.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 13/11/2017.
//
//
#pragma once
#include "ofMain.h"

namespace ofxLaser {

	class RenderProfile {
		public :
		RenderProfile(string _label = "") {
			
			ofLogNotice("RenderProfile default constructor called "+_label);
            init(_label);
		}
		
		void init(string _label) {
			
            label = _label;
			speed = 2;
			acceleration = 4;
			cornerThreshold = 125;
			
			params.setName(label);
			params.add(speed.set("speed",2,1,40));
			params.add(acceleration.set("acceleration",1,0.01,10));
			params.add(cornerThreshold.set("corner threshold",90,0,180));
			params.add(dotMaxPoints.set("dot max points", 2, 0, 100));
			
		}
        void setLabel(string _label) {
            label = _label;
            params.setName(label);
        } 
		
        
		ofParameter<float> speed;
		ofParameter<float> acceleration;
		ofParameter<float> cornerThreshold;
		ofParameter<int> dotMaxPoints;
		
		ofParameterGroup params;
        private :
        string label;

	};

}

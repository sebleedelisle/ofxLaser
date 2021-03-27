//
//  ofxLaserRenderProfile.hpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 13/11/2017.
//
//
#pragma once
#include "ofMain.h"
//#include "ofxGui.h"

namespace ofxLaser {

	class RenderProfile {
		public :
		RenderProfile() {
			
			ofLog(OF_LOG_WARNING, "RenderProfile default constructor called");
		}
		
		RenderProfile(string label) {
			

			speed = 2;
			acceleration = 4;
			cornerThreshold = 125;
			
			params.setName(label);
			params.add(speed.set("speed",2,1,40));
			params.add(acceleration.set("acceleration",1,0.01,10));
			params.add(cornerThreshold.set("corner threshold",90,0,180));
			params.add(dotMaxPoints.set("dot max points", 2, 0, 100));
			
		}
		
		
//		ofParameter<int> preBlankPoints;
//		ofParameter<int> preOnPoints;
//		ofParameter<int> postOnPoints;
//		ofParameter<int> postBlankPoints;
		ofParameter<float> speed;
		ofParameter<float> acceleration;
		ofParameter<float> cornerThreshold;
		ofParameter<int> dotMaxPoints;
		
		ofParameterGroup params;

	};

}

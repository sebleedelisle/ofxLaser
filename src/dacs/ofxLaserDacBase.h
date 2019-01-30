//
//  ofxLaserDacBase.hpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#pragma once
#include "ofxLaserPoint.h"

namespace ofxLaser {

	class DacBase {
	public:
		DacBase() {};
		
		virtual bool sendFrame(const vector<Point>& points) { return true; };
		virtual bool sendPoints(const vector<Point>& points) { return true; };
		virtual bool setPointsPerSecond(uint32_t pps) { return true; };
		virtual string getLabel(){return "";};
		
		virtual ofColor getStatusColour(){return ofColor::white; };
	
		virtual const vector<ofParameter<int>*>& getDisplayData() { return displayData;};
		virtual void resetDisplayData(){};
		virtual void reset() {};
		
	protected :
	
		vector<ofParameter<int>*> displayData;
		bool resetFlag = false;

	};

}

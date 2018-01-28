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
		
		virtual bool sendFrame(const vector<Point>& points){};
        virtual bool sendPoints(const vector<Point>& points){};
		virtual bool setPointsPerSecond(uint32_t pps){};

	};

}

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
        ~DacBase() {}; 
		
		virtual bool sendFrame(const vector<Point>& points)  = 0;
		virtual bool sendPoints(const vector<Point>& points)  = 0;
		virtual bool setPointsPerSecond(uint32_t pps)  = 0;
		virtual string getId() = 0;
        
		virtual ofColor getStatusColour() = 0;
	
		virtual const vector<ofAbstractParameter*>& getDisplayData();
		//virtual void resetDisplayData(){};
		virtual void reset() = 0;
        virtual void setArmed(bool armed);
		virtual void close() = 0;

        

		
	protected :
	
		vector<ofAbstractParameter*> displayData;
		bool resetFlag = false;
        bool armed = false;

	};

}

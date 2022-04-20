//
//  ofxLaserDacBase.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#pragma once
#include "ofxLaserPoint.h"

#define OFXLASER_DACSTATUS_GOOD 0
#define OFXLASER_DACSTATUS_WARNING 1
#define OFXLASER_DACSTATUS_ERROR 2


namespace ofxLaser {

	class DacBase {
	public:
		DacBase() {};
        ~DacBase() {}; 
		
		virtual bool sendFrame(const vector<Point>& points)  = 0;
		virtual bool sendPoints(const vector<Point>& points)  = 0;
		virtual bool setPointsPerSecond(uint32_t pps)  = 0;
        virtual bool setColourShift(float shiftSeconds) = 0;
		virtual string getId() = 0;
        
		//virtual ofColor getStatusColour() = 0;
        virtual int getStatus() = 0; 
	
		virtual const vector<ofAbstractParameter*>& getDisplayData();
		//virtual void resetDisplayData(){};
		virtual void reset() = 0;
        virtual void setArmed(bool armed);
		virtual void close() = 0;
        virtual bool isReadyForFrame(int maxLatencyMS){
            return true;
        }
        int maxLatencyMS; 

        string alias = "";
        
        bool colourShiftImplemented = false;
		
	protected :
	
		vector<ofAbstractParameter*> displayData;
		bool resetFlag = false;
        bool armed = false;
        bool frameMode = true;
        float colourShift = 0;
        
	};

}

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
#define OFXLASER_DACSTATUS_NO_DAC 3


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
        
//        virtual string getAlias() {
//            if(alias!="") return alias;
//            else return getId();
//        }
//        virtual void setAlias(string _alias) {
//            alias = _alias;
//        }
//
		//virtual ofColor getStatusColour() = 0;
        virtual int getStatus() = 0;
        bool hasStatusChanged() {
            int currentstatus =getStatus();
            if(lastStatus!=currentstatus) {
                lastStatus = currentstatus;
                return true;
            } else {
                return false;
            }
        }
	
		virtual const vector<ofAbstractParameter*>& getDisplayData();
		//virtual void resetDisplayData(){};
		virtual void reset() = 0;
        virtual void setArmed(bool armed);
		virtual void close() = 0;
        virtual bool isReadyForFrame(int maxLatencyMS){
            return true;
        }
        
        void logNotice(const string& msg) {
            if(logging) {
                ofLogNotice() << msg;
            }
        }
        
        
        int maxLatencyMS;
        // additional buffer time to allow for calculations
        // only used to know if the frame is ready
        int calculationTimeMS = 10;
        // minimum buffer to send at once
        int minPacketDataSize = 256;
        
        bool colourShiftImplemented = false;
		
        bool verbose = false;
        bool logging = false;
        
	protected :
	
		vector<ofAbstractParameter*> displayData;
		bool resetFlag = false;
        bool armed = false;
        bool frameMode = true;
        //string alias = "";
        
        float colourShift = 0;
        int lastStatus = OFXLASER_DACSTATUS_NO_DAC;
        

	};

}

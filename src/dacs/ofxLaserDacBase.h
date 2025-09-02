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
		//virtual bool sendPoints(const vector<Point>& points)  = 0;
		virtual bool setPointsPerSecond(uint32_t pps)  = 0;
        virtual uint32_t getPointsPerSecond()  = 0;
        virtual bool setColourShift(float shiftSeconds) = 0;
        virtual string getType() = 0; 
        virtual string getFullId() {
            return getType() + " " + getRawId();
        }
        virtual string getRawId() = 0;
        
        // whether the DAC reports battery level (like LaserCube)
        virtual bool canReportBatteryLevel(){ return hasBatteryLevel; };
        // whether the DAC reports temperature
        virtual bool canReportTemperatureLevel(){ return hasTemperature;};
        
        virtual float getTemperatureC() { return temperatureC; }
        virtual float getBatteryLevel() { return batteryLevel; }
        virtual bool getIsPluggedIn() { return isPluggedIn; }
        
        
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
        
        virtual void logNotice(const string& msg) {
            if(logging) {
                ofLogNotice() << msg;
            }
        }
        
        
        std::atomic<int> maxLatencyMS;
        // additional buffer time to allow for calculations
        // only used to know if the frame is ready
        int calculationTimeMS = 10;
        // minimum buffer to send at once
        int minPacketDataSize = 128;
        int maxPointRate = 60000; 
        
        bool colourShiftImplemented = false;
		
        bool verbose = false;
        bool logging = true;
        
        int blankPointsAfterReArmRemaining = 0;
        float blankTimeAfterReArm = 0.1; // 0.2 seconds of blank after rearm
        
        
	protected :
	
		vector<ofAbstractParameter*> displayData;
		std::atomic<bool> resetFlag = false;
        bool armed = false;
       // bool frameMode = true;
        
        std::atomic<float> colourShift = 0;
        int lastStatus = OFXLASER_DACSTATUS_NO_DAC;
        bool hasBatteryLevel = false;
        bool hasTemperature = false;
        std::atomic<float> temperatureC = 0;
        std::atomic<float> batteryLevel = 0; // 0 to 1
        std::atomic<bool> isPluggedIn = false; // 0 to 1
	};

}

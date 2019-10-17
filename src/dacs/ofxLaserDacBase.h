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
	
		virtual const vector<ofAbstractParameter*>& getDisplayData() { return displayData;};
		virtual void resetDisplayData(){};
		virtual void reset() {};


		static uint16_t bytesToUInt16(unsigned char* byteaddress) {
			return (uint16_t)(*(byteaddress+1)<<8)|*byteaddress;
			
		}
		static uint16_t bytesToInt16(unsigned char* byteaddress) {
			uint16_t i = *(signed char *)(byteaddress);
			i *= 1 << CHAR_BIT;
			i |= (*byteaddress+1);
			return i;
			
		}
		static uint32_t bytesToUInt32(unsigned char* byteaddress){
			return (uint32_t)(*(byteaddress+3)<<24)|(*(byteaddress+2)<<16)|(*(byteaddress+1)<<8)|*byteaddress;
			
		}
		static void writeUInt16ToBytes(uint16_t& n, unsigned char* byteaddress){
			*(byteaddress+1) = n>>8;
			*byteaddress = n&0xff;
		}
		static void writeInt16ToBytes(int16_t& n, unsigned char* byteaddress){
			*(byteaddress+1) = n>>8;
			*byteaddress = n&0xff;
		}
		static void writeUInt32ToBytes(uint32_t& n, unsigned char* byteaddress){
			*(byteaddress+3) = (n>>24) & 0xff;
			*(byteaddress+2) = (n>>16) & 0xff;
			*(byteaddress+1) = (n>>8) & 0xff;
			*byteaddress = n&0xff;
			
		}

		
	protected :
	
		vector<ofAbstractParameter*> displayData;
		bool resetFlag = false;

	};

}

//
//  ofxLaserDacIDN.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 13/04/2018.
//
//

#pragma once
#include "ofMain.h"
#include "ofxLaserDacBase.h"
#include "ofxNetwork.h"

#define IDN_MIN -32768
#define IDN_MAX 32767

namespace ofxLaser {
	
class IDN_point {
	public :
	
	char* getSerialised() {
		
		serialized[0] = (uint8_t) (x >> 8);
		serialized[1] = (uint8_t) (x);
		serialized[2] = (uint8_t) (y >> 8);
		serialized[3] = (uint8_t) (y);
		serialized[4] = r;
		serialized[5] = g;
		serialized[6] = b;
		
		return (char*) &serialized;
	}
	
	char serialized[7];
	
	uint16_t x;
	uint16_t y;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

	
class DacIDN : public DacBase, ofThread {
	
	public:
	void setup(string ip);
	
	bool sendFrame(const vector<Point>& points);
	bool sendPoints(const vector<Point>& points);
	bool setPointsPerSecond(uint32_t pps);
	
	string getId(){
		return "IDN";
	}
	
	// TODO return relevant colour 
	ofColor getStatusColour(){
		return connected ? ofColor::green :  ofColor::red;
	}
	
	void close();
	
	protected:

	private:

	void threadedFunction();
	
	void sendFrameToDac();

	ofxUDPManager udpConnection;

	uint32_t pps;
	bool connected;
	bool newFrameIsBuffered;
	uint64_t lastFrameTime;
	uint64_t lastFrameDuration;
	
	vector<IDN_point> pointsToSend;
	deque<IDN_point> bufferedPoints;
	uint16_t counter ;

	const bool verbose = false; 

};

}

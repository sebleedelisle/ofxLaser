//
//  ofxLaserDacBase.hpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserDacBase.h"
#include "ofxNetwork.h"
#include "LaserdockDeviceManager.h"
#include "LaserdockDevice.h"
#include <libusb.h>


#define LASERDOCK_MIN 0
#define LASERDOCK_MAX 4095

namespace ofxLaser {

class DacLaserdock : public DacBase, ofThread{
	public:
	
	DacLaserdock() : lddmanager(LaserdockDeviceManager::getInstance()) {};
	~DacLaserdock();
	
	void setup();
	
	bool sendFrame(const vector<Point>& points) ;
	bool sendPoints(const vector<Point>& points) ;
	bool setPointsPerSecond(uint32_t pps);
	string getLabel(){return "Laserdock";};
	
	// TODO return relevant colour
	ofColor getStatusColour(){
		return ofColor::green;
	}
	
	bool addPoint(const LaserdockSample &point );
	// simple object pooling system
	LaserdockSample* getLaserdockSample();
	
	
	private:
	void threadedFunction();

	LaserdockDeviceManager &lddmanager;
	LaserdockDevice * device;
	
	LaserdockSample sendpoint, lastpoint;
	deque<LaserdockSample*> bufferedPoints;
	vector<LaserdockSample*> sparePoints;
	vector<LaserdockSample> framePoints;
	
	uint32_t pps, newPPS;
	
	bool frameMode = true; 
	bool replayFrames = true;
	bool isReplaying = false;
	bool connected = false; 

};

}

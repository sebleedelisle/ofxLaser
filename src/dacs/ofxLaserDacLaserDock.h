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

class DacLaserDock : public DacBase, ofThread{
	public:
	
	DacLaserDock() : lddmanager(LaserdockDeviceManager::getInstance()) {};
	~DacLaserDock();
	
	void setup();
	
	bool sendFrame(const vector<Point>& points) ;
	bool sendPoints(const vector<Point>& points) ;
	bool setPointsPerSecond(uint32_t pps);
	string getLabel(){return "LaserDock";};
	
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
	
	bool frameMode = true; 
	bool replayFrames = true;
	bool isReplaying = false;

};

}

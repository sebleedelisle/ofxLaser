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
#include "HeliosDac.h"
#include "ofxLaserDacHeliosManager.h"

#define HELIOS_MIN 0
#define HELIOS_MAX 4095

namespace ofxLaser {

class DacHelios : public DacBase, ofThread{
	public:
	
	DacHelios() : heliosManager(DacHeliosManager::getInstance()) {};
	~DacHelios();
	
	void setup(string serial="");
	bool connectToDevice(string serial="");
	
	bool sendFrame(const vector<Point>& points) ;
	bool sendPoints(const vector<Point>& points) ;
	bool setPointsPerSecond(uint32_t pps);
	
	string getLabel(){return "Helios";};
	
	// TODO return relevant colour
	ofColor getStatusColour(){
		return connected ? ofColor::green :  ofColor::red;
	}
	
	bool addPoint(const HeliosPoint &point );
	// simple object pooling system
	HeliosPoint* getHeliosPoint();
	
	ofParameter<int> pointBufferDisplay;
	ofParameter<string> deviceName; 
	
	private:
	void threadedFunction();

	void setConnected(bool state);
	
	
	HeliosPoint sendpoint, lastpoint;
	deque<HeliosPoint*> bufferedPoints;
	vector<HeliosPoint*> sparePoints;
	vector<HeliosPoint> framePoints;
	
	uint32_t pps, newPPS;
	
	bool frameMode = true; 
	bool replayFrames = true;
	bool isReplaying = false;
	bool connected = false;
	
	DacHeliosManager &heliosManager;
	int deviceNumber; 
	
	
};

}

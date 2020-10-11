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
    void reset();
    void setActive(bool active);
	void close(); 
	
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
	
	uint32_t pps = 30000, newPPS = 30000;
	
	bool frameMode = true; 
	bool replayFrames = true;
	bool isReplaying = false;
	bool connected = false;
    bool newArmed = false;  // as in the PPS system, this knows
                            // if armed status has changed and sends
                            // signal to DAC
	
	DacHeliosManager &heliosManager;
	//int deviceNumber;
	string deviceId = "";
	HeliosDacDevice* dacDevice = nullptr; 
	
	
};

}

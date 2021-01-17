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

class DacHeliosFrame {
	
	public :
	DacHeliosFrame() {
		samples = (HeliosPoint *)calloc(sizeof(HeliosPoint), maxSamples);
	}
	~DacHeliosFrame() {
		free(samples);
	}
	
	void addPoint(ofxLaser::Point& p) {
		// TODO check size
		HeliosPoint& s = samples[numSamples];
		numSamples++;
		s.x = ofMap(p.x,0,800, HELIOS_MIN, HELIOS_MAX);
		s.y = ofMap(p.y,800,0, HELIOS_MIN, HELIOS_MAX); // Y is UP
		s.r = roundf(p.r);
		s.g = roundf(p.g);
		s.b = roundf(p.b);
		s.i = 255;
		
	}
	
	
	HeliosPoint * samples;
	int numSamples = 0;
	const int maxSamples = 100000;
	
};


class DacHelios : public DacBase, ofThread{
	public:
	
	DacHelios();
	~DacHelios();
	
	void setup(string serial="");
	const vector<ofAbstractParameter*>& getDisplayData();
		
	bool connectToDevice(string serial="");
    void reset();
    void setActive(bool active);
	void close();
	
	bool sendFrame(const vector<Point>& points) ;
	bool sendPoints(const vector<Point>& points) ;
	bool setPointsPerSecond(uint32_t pps);
	
	string getLabel(){return "Helios";};
	

	ofColor getStatusColour(){
		return connected ? ofColor::green :  ofColor::red;
	}
	
	
	ofParameter<int> pointBufferDisplay;
	ofParameter<string> deviceName;
	
	private:
	void threadedFunction();

	void setConnected(bool state);
	
	
	
	ofThreadChannel<DacHeliosFrame*> framesChannel;
		
	uint32_t pps;
	std::atomic<uint32_t>  newPPS;
	
	std::atomic<bool> frameMode;
	std::atomic<bool> connected ;
    std::atomic<bool> newArmed ;  // as in the PPS system, this knows
                            // if armed status has changed and sends
                            // signal to DAC
	
	DacHeliosManager &heliosManager;
	
	// ONLY to be accessed within thread
	string deviceId;
	HeliosDacDevice* dacDevice;
	
	
};

}

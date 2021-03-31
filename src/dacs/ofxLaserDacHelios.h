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
//#include "ofxLaserDacHeliosManager.h"

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
	void reset() {
		numSamples = 0;
	} 
	
	bool addPoint(const ofxLaser::Point& p) {
		// TODO check size
        if(numSamples==maxSamples) return false;
		HeliosPoint& s = samples[numSamples];
		numSamples++;
		s.x = ofMap(p.x,0,800, HELIOS_MIN, HELIOS_MAX);
		s.y = ofMap(p.y,800,0, HELIOS_MIN, HELIOS_MAX); // Y is UP
		s.r = roundf(p.r);
		s.g = roundf(p.g);
		s.b = roundf(p.b);
		s.i = 255;
        return true; 
	}
	
	
	HeliosPoint * samples;
	int numSamples = 0;
	const int maxSamples = 100000;
	
};


class DacHelios : public DacBase, ofThread{
	public:
	
	DacHelios();
	~DacHelios();
	
    
    OF_DEPRECATED_MSG("DACs are no longer set up in code, do it within the app instead",  bool setup());
   
	bool setup(libusb_device* usbdevice);
	const vector<ofAbstractParameter*>& getDisplayData() override;
		
	//bool connectToDevice(string serial="");
    void reset() override;
    void setArmed(bool armed) override;
	void close() override;
	
	bool sendFrame(const vector<Point>& points) override ;
	bool sendPoints(const vector<Point>& points)  override;
	bool setPointsPerSecond(uint32_t pps) override;
	
	DacHeliosFrame* getFrame();
	DacHeliosFrame* releaseFrame(DacHeliosFrame* frame);
	ofThreadChannel<DacHeliosFrame*> spareFrames;
	
	//string getLabel(){return "Helios";};
    string getLabel() override{return "Helios " + ofToString(serialNumber);};
    
	

	ofColor getStatusColour() override {
		return connected ? ofColor::green :  ofColor::red;
	}
	
	
	ofParameter<int> pointBufferDisplay;
	ofParameter<string> deviceName;
    
    string serialNumber;
    
	private:
	void threadedFunction() override;

	void setConnected(bool state);
	
	/// TEMP
	ofxLaser::Point lastPoint;
	
	ofThreadChannel<DacHeliosFrame*> framesChannel;
		
	uint32_t pps;
	std::atomic<uint32_t>  newPPS;
	
	std::atomic<bool> frameMode;
	std::atomic<bool> connected ;
    std::atomic<bool> newArmed ;  // as in the PPS system, this knows
                            // if armed status has changed and sends
                            // signal to DAC
	
	//DacHeliosManager &heliosManager;
	
   
	// ONLY to be accessed within thread
	string deviceId;
	HeliosDacDevice* dacDevice;
	
	
};

}

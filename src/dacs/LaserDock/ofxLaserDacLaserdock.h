//
//  ofxLaserDacLaserdock.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 03/03/2019.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserDacThreadedBase.h"
#include "ofxLaserDacLaserDockByteStream.h"
#include "LaserdockDevice.h"
#include "libusb.h"


#define LASERDOCK_MIN 0
#define LASERDOCK_MAX 4095

namespace ofxLaser {

class DacLaserdock : public DacThreadedBase{
	public:
	
    DacLaserdock(){
        colourShiftImplemented = true; // eventually we can get rid of this
    };
    ~DacLaserdock();

    bool setup(libusb_device* usbdevice);
    OF_DEPRECATED_MSG("DACs are no longer set up in code, do it within the app instead",  bool setup());
   
    void reset() override;
    void close() override;
    virtual int getMaxPointBufferSize() override {
        return 4096;
    }
    
	//bool sendFrame(const vector<Point>& points) override ;
	//bool sendPoints(const vector<Point>& points) override ;
	bool setPointsPerSecond(uint32_t pps) override;
    //virtual bool setColourShift(float shift) override { return true; }; // TODO implement here in the DAC
	
	string getId() override {return "Laserdock " + ofToString(serialNumber);};
	
    int getStatus() override {
        return connected ? OFXLASER_DACSTATUS_GOOD :  OFXLASER_DACSTATUS_ERROR;
    }
    
	//bool addPoint(const LaserdockSample &point );
	// simple object pooling system
	//LaserdockSample* getLaserdockSample();
	
//	ofParameter<int> pointBufferDisplay;
	ofParameter<string> serialNumber;
	
    bool sendPointsToDac(); 
    
    bool verbose = false; 
    
	private:
	void threadedFunction() override;

	void setConnected(bool state);
	
	LaserdockDevice * dacDevice = nullptr;
	
	//LaserdockSample sendpoint, lastpoint;
	//deque<LaserdockSample*> bufferedPoints;
	//vector<LaserdockSample*> sparePoints;
	//vector<LaserdockSample> framePoints;
	
    DacLaserdockByteStream dacCommand; 
    LaserdockSample lastPointSent;
    LaserdockSample sendPoint; // I think used as a spare?
 
    
//	uint32_t pps, newPPS;
	uint32_t maxPPS; 
	
	//bool frameMode = true;
	//bool replayFrames = true;
	//bool isReplaying = false;
	bool connected = false;
    
	
	
};

}

//
//  ofxLaserDacLaserdock.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 03/03/2019.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserDacBaseThreaded.h"
#include "ofxNetwork.h" // if you take this out it no longer compiles on windows and I have no idea why
#include "ByteBuffer.h"
//#include "ofxLaserDacLaserDockByteStream.h"
#include "LaserdockDevice.h"
#include "libusb.h"


#define LASERDOCK_MIN 0
#define LASERDOCK_MAX 4095

namespace ofxLaser {

class DacLaserdockByteStream : public ByteBuffer {
 
    public :
    
    void clear() override ;
    void addPoint(LaserdockSample& p);
     void logData();
    
    int numPointsExpected = 0;
    int numPoints = 0;
    
} ;


class DacLaserDock : public DacBaseThreaded{
	public:
	
    DacLaserDock(){
        colourShiftImplemented = true; // eventually we can get rid of this
    };
    ~DacLaserDock();

    bool setup(libusb_device* usbdevice);
    OF_DEPRECATED_MSG("DACs are no longer set up in code, do it within the app instead",  bool setup());
   
    void reset() override;
    void close() override;
    virtual int getMaxPointBufferSize() override {
        return 4096;
    }
    
	bool setPointsPerSecond(uint32_t pps) override;
	
	string getId() override {return "Laserdock " + ofToString(serialNumber);};
	
    int getStatus() override {
        return connected ? OFXLASER_DACSTATUS_GOOD :  OFXLASER_DACSTATUS_ERROR;
    }
    
    ofParameter<string> serialNumber;
	
    bool sendPointsToDac(); 
    
    bool verbose = false; 
    
	private:
	void threadedFunction() override;

	void setConnected(bool state);
	
	LaserdockDevice * dacDevice = nullptr;
	
    DacLaserdockByteStream dacCommand; 
    LaserdockSample lastPointSent;
    LaserdockSample sendPoint; // I think used as a spare?
 
	uint32_t maxPPS; 
	
	bool connected = false;
    

};

}

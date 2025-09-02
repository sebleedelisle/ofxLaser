//
//  ofxLaserDacHelios.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/05/2020.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserDacBase.h"
#include "HeliosDac.h"
#include "UsbDeviceHandleHelios.h"

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
        if(numSamples>=maxSamples) return false;
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
	const int maxSamples = HELIOS_MAX_POINTS;
	
};

// wrapper for Gitle's class, but doesn't close the usb

class HeliosDacSafe : public HeliosDacDevice {
public:
    
    HeliosDacSafe(UsbDeviceHandleHelios* safeHandle)
        : HeliosDacDevice(safeHandle->getHandle()) {}

    void SetClosed() override {
        // No-op: we don’t own the handle
    }

    ~HeliosDacSafe() {
        closed = true;
        // Avoid double-close
    }
};

class DacHelios : public DacBase, ofThread{
	public:
	
	DacHelios();
	~DacHelios();
	
    
    OF_DEPRECATED_MSG("DACs are no longer set up in code, do it within the app instead",  bool setup());
   
	bool setup(std::unique_ptr<UsbDeviceHandleHelios> handle);
	const vector<ofAbstractParameter*>& getDisplayData() override;
		
	//bool connectToDevice(string serial="");
    void reset() override;

	void close() override;
	
	bool sendFrame(const vector<Point>& points) override ;
	//bool sendPoints(const vector<Point>& points)  override;
	bool setPointsPerSecond(uint32_t pps) override;
    uint32_t getPointsPerSecond() override; 
    virtual bool setColourShift(float shiftseconds) override { return true; }; // TODO implement here in DAC
    
    
    libusb_device* getDevice() const {
        return usbHandle ? usbHandle->getDevice() : nullptr;
    }
    
    std::shared_ptr<DacHeliosFrame> getFrame();
    void deleteFrame(std::shared_ptr<DacHeliosFrame> frame);
    
	ofThreadChannel<std::shared_ptr<DacHeliosFrame>> spareFramesChannel;
    vector<std::shared_ptr<DacHeliosFrame>>spareFrames; 
    DacHeliosFrame blankFrame; 
	
    
    std::unique_ptr<UsbDeviceHandleHelios> usbHandle;

    virtual string getType() override { return "Helios"; };
    
    string getRawId() override{return ofToString(dacName);};

	int getStatus() override {
		return connected ? OFXLASER_DACSTATUS_GOOD :  OFXLASER_DACSTATUS_ERROR;
	}
	
	
	ofParameter<int> pointBufferDisplay;
	ofParameter<string> deviceName;
    
    string dacName;
    HeliosDacSafe* dacDevice;
    
	private:
	void threadedFunction() override;

	void setConnected(bool state);
	
	/// TEMP
	ofxLaser::Point lastPoint;
	
	ofThreadChannel<std::shared_ptr<DacHeliosFrame>> framesChannel;
		
    std::atomic<uint32_t> pps;
	std::atomic<uint32_t> newPPS;
	
	//std::atomic<bool> frameMode;
	std::atomic<bool> connected ;
    std::atomic<bool> armedStatus ;  // as in the PPS system, this knows
                            // if armed status has changed and sends
                            // signal to DAC
	
	//DacHeliosManager &heliosManager;
	
   
	
};

}

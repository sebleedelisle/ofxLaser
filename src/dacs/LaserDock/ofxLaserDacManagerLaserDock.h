//
//  ofxLaserDacManagerBase.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#pragma once
#include "ofxLaserDacManagerBase.h"
#include "ofxLaserDacLaserDock.h"

#include <libusb.h>
#include <optional>

#define LASERDOCK_VIN 0x1fc9
#define LASERDOCK_PIN 0x04d8

namespace ofxLaser {

class DacManagerLaserDock : public DacManagerBase{
    
    public :
    DacManagerLaserDock();
    ~DacManagerLaserDock();
    virtual vector<DacData> updateDacList() override;
    virtual std::shared_ptr<DacBase> getAndConnectToDac(const string& id) override;
     virtual string getType() override {
        return "LaserCubeUSB";
    }
    virtual void exit() override;

    protected :

    // checks the usbdevice to see if it's a laserdock, and if it is,
    // it returns the serial number. If not it returns an empty string.
    
    std::optional<std::string> getLaserdockSerialNumber(libusb_device* usbdevice);

    libusb_context* ctx = nullptr;
    
    
};
}



//
//  ofxLaserDacManagerBase.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#pragma once
#include "ofxLaserDacManagerBase.h"
#include "ofxLaserDacHelios.h"
#include <libusb.h>
#include <optional>

#define HELIOS_VID    0x1209
#define HELIOS_PID    0xE500

namespace ofxLaser {

class DacManagerHelios : public DacManagerBase{
    
    public :
    DacManagerHelios();
    ~DacManagerHelios();
    virtual vector<DacData> updateDacList() override;
    virtual std::shared_ptr<DacBase>  getAndConnectToDac(const string& id) override;
    
    virtual string getType() override {
        return "Helios";
    }
    virtual void exit() override;
    
    
    
    protected :

    // checks the usbdevice to see if it's a laserdock, and if it is,
    // it returns the serial number. If not it returns an empty string.
    
    std::unique_ptr<UsbDeviceHandleHelios> getHandleIfDeviceIsHelios(libusb_device* usbdevice);
    std::optional<std::string> getHeliosNameForUsbHandle(std::unique_ptr<UsbDeviceHandleHelios>& handle); 

    libusb_context* ctx = nullptr;
    
    
};
}



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

#define HELIOS_VID    0x1209
#define HELIOS_PID    0xE500

namespace ofxLaser {

class DacManagerHelios : public DacManagerBase{
    
    public :
    DacManagerHelios();
    ~DacManagerHelios();
    virtual vector<DacData> updateDacList() override;
    virtual DacBase* getAndConnectToDac(const string& id) override;
    virtual bool disconnectAndDeleteDac(const string& id) override;
    virtual string getType() override {
        return "Helios";
    }
    virtual void exit() override;

    protected :

    // checks the usbdevice to see if it's a laserdock, and if it is,
    // it returns the serial number. If not it returns an empty string.
    
    string getHeliosSerialNumber(libusb_device* usbdevice);

    
    
    
};
}



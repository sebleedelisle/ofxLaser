//
//  ofxLaserDacManagerBase.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#pragma once
#include "ofxLaserDacManagerBase.h"
#include "ofxLaserDacLaserdock.h"
#include <libusb.h>

#define LASERDOCK_VIN 0x1fc9
#define LASERDOCK_PIN 0x04d8

namespace ofxLaser {

class DacManagerLaserdock : public DacManagerBase{
    
    public :
    DacManagerLaserdock();
    ~DacManagerLaserdock();
    virtual vector<DacData> updateDacList() override;
    virtual DacBase* getAndConnectToDac(const string& id) override;
    virtual bool disconnectAndDeleteDac(const string& id) override;
    virtual string getType() override {
        return "Laserdock";
    }
    virtual void exit() override;

    protected :

    // checks the usbdevice to see if it's a laserdock, and if it is,
    // it returns the serial number. If not it returns an empty string.
    
    string getLaserdockSerialNumber(libusb_device* usbdevice);

    
    
    
};
}



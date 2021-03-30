//
//  ofxLaserDacDetectorBase.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//
#pragma once
#include "ofxLaserDacDetectorBase.h"
//#include "LaserdockDeviceManager.h"
#include "ofxLaserDacLaserdock.h"
#include <libusb.h>

#define LASERDOCK_VIN 0x1fc9
#define LASERDOCK_PIN 0x04d8

namespace ofxLaser {

class DacDetectorLaserdock : public DacDetectorBase{
    
    public :
    DacDetectorLaserdock();
    ~DacDetectorLaserdock();
    virtual vector<DacData> updateDacList() override;
    virtual DacBase* getAndConnectToDac(const string& id) override;
    virtual bool disconnectAndDeleteDac(const string& id) override;
    virtual string getType() override {
        return "Laserdock";
    }

    protected :
//
//    bool isLaserdock(libusb_device *device) {
//        struct libusb_device_descriptor device_descriptor;
//        int result = libusb_get_device_descriptor(device, &device_descriptor);
//        if (result < 0) {
//            printf("Failed to get device descriptor!");
//        }
//
//        ofLogNotice("Found laserdock : "+ofToString(device_descriptor.iSerialNumber));
//        if ((device_descriptor.idVendor==LASERDOCK_VIN)  && (device_descriptor.idProduct==LASERDOCK_PIN))
//            return true;
//
//        return false;
//    }
    
    // checks the usbdevice to see if it's a laserdock, and if it is,
    // it returns the serial number. If not it returns an empty string.
    
    string getLaserDockSerialNumber(libusb_device* usbdevice) {
        // make a descriptor object to store the data in
        struct libusb_device_descriptor devicedescriptor;
        // get the descriptor
        
        string returnstring = "";
        
        int result = libusb_get_device_descriptor(usbdevice, &devicedescriptor);
        if (result < 0) {
            ofLogError("DacDetectorLaserdock failed to get device descriptor for USB device - error code " + ofToString(result));
            // skip to the next one
            return returnstring;
        }
        
        if ((devicedescriptor.idVendor==LASERDOCK_VIN)  && (devicedescriptor.idProduct==LASERDOCK_PIN)) {
            unsigned char idstring[256];
            struct libusb_device_handle *devhandlecontrol;
            
            ofLogNotice("Found laserdock : "+ofToString(devicedescriptor.iSerialNumber));
            
            // open the device
            result = libusb_open(usbdevice, &devhandlecontrol);
            if(result!=0) {
                ofLogError("DacDetectorLaserdock failed to open USB device - error code " + ofToString(result));
                return returnstring;
            }
            
            // let's get the serial number from the device
            result = libusb_get_string_descriptor_ascii(devhandlecontrol, devicedescriptor.iSerialNumber, idstring, sizeof(idstring));
            if (result > 0) {
                // add to list...
                returnstring = (char*)idstring;
                ofLogNotice("Serial Number: " + returnstring);
                
                //DacData(string _type, string _id, string _address="", ofxLaser::Projector* projector = nullptr){
                //DacData data(getType(), serialnumber);
               // daclist.push_back(data);
            } else {
                ofLogError("DacDetectorLaserdock failed to get serial number - error code " + ofToString(result));
            }
            libusb_close(devhandlecontrol);
            
          
        }
        return returnstring; 

        
        
    }

    
    
    
};
}



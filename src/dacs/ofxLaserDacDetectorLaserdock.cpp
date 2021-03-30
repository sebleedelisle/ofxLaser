//
//  ofxLaserDacDetectorBase.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#include "ofxLaserDacDetectorLaserdock.h"

using namespace ofxLaser;

DacDetectorLaserdock :: DacDetectorLaserdock()  {
    int rc;
    rc = libusb_init(NULL);
    if (rc < 0)
    {
        ofLogError("Error initializing libusb: "+ofToString(rc));
        //return false;
    }
    
}
DacDetectorLaserdock :: ~DacDetectorLaserdock()  {

    // TODO wait for all DACs threads to stop
    
    libusb_exit(NULL);
}

vector<DacData> DacDetectorLaserdock :: updateDacList(){
    
    vector<DacData> daclist;
    libusb_device **libusb_device_list;
    ssize_t cnt = libusb_get_device_list(NULL, &libusb_device_list);
    ssize_t i = 0;

    if (cnt < 0) {
        // LIBUSB ERROR
        ofLogError("ofxDacDetectorLaserdock :: getDacList() - USB error code " + ofToString(cnt));
        libusb_free_device_list(libusb_device_list, 1); // probably not necessary
        return daclist;
    }

    for (i = 0; i < cnt; i++) {
        
        // get the device from the array
        libusb_device *usbdevice = libusb_device_list[i];
        
        // checks to see if the device is a laserdock, and if so, returns the serial number.
        string serialnumber = getLaserDockSerialNumber(usbdevice);
        
        if(serialnumber!="") {
            DacData data(getType(), serialnumber);
            daclist.push_back(data);
        }
            
        
        
        
    }

    libusb_free_device_list(libusb_device_list, cnt);
    return daclist;
    
}



DacBase* DacDetectorLaserdock :: getAndConnectToDac(const string& id){
    
    // returns a dac - if failed returns nullptr.
    
    // get USB device and make a LaserDockDevice from it
    DacLaserdock* dac = nullptr;
    
    libusb_device **libusb_device_list;
    ssize_t cnt = libusb_get_device_list(NULL, &libusb_device_list);
    ssize_t i = 0;

    if (cnt < 0) {
        fprintf(stderr, "Error finding USB device\n");
        libusb_free_device_list(libusb_device_list, 1); // probably not necessary
        return nullptr;
    }

    for (i = 0; i < cnt; i++) {
        libusb_device *libusb_device = libusb_device_list[i];
        
        if (getLaserDockSerialNumber(libusb_device) == id) {
            // check serial number, if it matches, then make a dac with it!
            dac = new DacLaserdock();
            if(!dac->setup(libusb_device)){
                delete dac;
                dac = nullptr;
                
            }

        }
    }

    libusb_free_device_list(libusb_device_list, cnt);
    
    
    //DacLaserdock* dac = new DacLaserdock();
   // dac->setup(dacdata.id);
    // TODO store dac in list
    
    return dac;
}

bool DacDetectorLaserdock :: disconnectAndDeleteDac(const string& dacdata){
    
    
    
}

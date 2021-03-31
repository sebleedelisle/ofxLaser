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
        libusb_free_device_list(libusb_device_list, 1);
        return daclist;
    }

    for (i = 0; i < cnt; i++) {
        
        // get the device from the array
        libusb_device *usbdevice = libusb_device_list[i];
        
        // checks to see if the device is a laserdock, and if so, returns the serial number.
        string serialnumber = getLaserdockSerialNumber(usbdevice);
        
		// if we found one, add it to the list! 
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
    
    DacLaserdock* dac = (DacLaserdock*) getDacById(id);
    
    if(dac!=nullptr) {
        ofLogNotice("DacDetectorLaserdock :: getAndConnectToDac(...) - Already a dac made with id "+ofToString(id));
        return dac;
    }
    
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
        
        if (getLaserdockSerialNumber(libusb_device) == id) {
            // check serial number, if it matches, then make a dac with it!
            dac = new DacLaserdock();
            if(!dac->setup(libusb_device)){
                delete dac;
                dac = nullptr;
                
            } else {
                // STORE DAC!
                dacsById[id] = dac;
            }

        }
    }

    libusb_free_device_list(libusb_device_list, cnt);
    
    
    
    return dac;
}

bool DacDetectorLaserdock :: disconnectAndDeleteDac(const string& id){
    
    if ( dacsById.count(id) == 0 ) {
        ofLogError("DacDetectorLaserdock::disconnectAndDeleteDac("+id+") - dac not found");
        return false;
    }
    DacLaserdock* dac = (DacLaserdock*) dacsById.at(id);
    dac->close();
    auto it=dacsById.find(id);
    dacsById.erase(it);
    delete dac; 
    
}


string DacDetectorLaserdock :: getLaserdockSerialNumber(libusb_device* usbdevice) {
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

void DacDetectorLaserdock :: exit() {
    
}

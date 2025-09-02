//
//  ofxLaserDacManagerBase.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#include "ofxLaserDacManagerLaserDock.h"

using namespace ofxLaser;

DacManagerLaserDock :: DacManagerLaserDock()  {
    int rc;
    rc = libusb_init(&ctx);
    if (rc < 0)
    {
        ofLogError("Error initializing libusb: "+ofToString(rc));
        //return false;
    }
    
}
DacManagerLaserDock :: ~DacManagerLaserDock()  {
    ofLogNotice("DacManagerLaserDock :: ~DacManagerLaserDock()");

    for (auto& [id, dac] : dacsById) {
        if (dac) {
            dac->close();
        }
    }
    
    if(ctx) {
        libusb_exit(ctx);
        ctx = nullptr;
    }
}

vector<DacData> DacManagerLaserDock::updateDacList() {
    vector<DacData> daclist;
  
    libusb_device **libusb_device_list = nullptr;
    ssize_t cnt = libusb_get_device_list(ctx, &libusb_device_list);

    if (cnt < 0 || libusb_device_list==nullptr) {
        ofLogError("DacManagerLaserDock::updateDacList - libusb_get_device_list() failed. Error code: " + ofToString(cnt));
        return daclist; // nothing to free
    }

    for (ssize_t i = 0; i < cnt; i++) {
        libusb_device *usbdevice = libusb_device_list[i];
        auto maybeSerial = getLaserdockSerialNumber(usbdevice);
        if (maybeSerial) {
            daclist.emplace_back(getType(), *maybeSerial);
        }
       
    }

    libusb_free_device_list(libusb_device_list, 1);
    return daclist;
}

std::shared_ptr<DacBase> DacManagerLaserDock :: getAndConnectToDac(const string& id){
    
    // returns a dac - if failed returns nullptr.
    
    
    std::shared_ptr<DacBase> dac =  getDacById(id);
    
    if(dac) {
        ofLogNotice("DacManagerLaserDock :: getAndConnectToDac(...) - Already a dac made with id "+ofToString(id));
        return dac;
    }
    
    libusb_device **libusb_device_list;
    ssize_t cnt = libusb_get_device_list(ctx, &libusb_device_list);
    ssize_t i = 0;

    if (cnt < 0 || libusb_device_list == nullptr) {
        fprintf(stderr, "Error finding USB device\n");
        //libusb_free_device_list(libusb_device_list, 1); // probably not necessary
        return nullptr;
    }

    for (i = 0; i < cnt; i++) {
        libusb_device *libusb_device = libusb_device_list[i];
        
        if (getLaserdockSerialNumber(libusb_device) == id) {
            // check serial number, if it matches, then make a dac with it!
            auto hdac = std::make_shared<DacLaserDock>();
            if(!hdac->setup(libusb_device)){
                //delete hdac;
                
            } else {
                // STORE DAC!
                dacsById.emplace(id, hdac);
                dac = hdac;
            }
            break;

        }
    }

    libusb_free_device_list(libusb_device_list, (int)cnt);
    
    
    
    return dac;
    
    
}

std::optional<std::string> DacManagerLaserDock::getLaserdockSerialNumber(libusb_device* usbdevice) {
    libusb_device_descriptor devicedescriptor;
    int result = libusb_get_device_descriptor(usbdevice, &devicedescriptor);
    if (result < 0) {
        ofLogError("DacManagerLaserDock: failed to get device descriptor. Error: " + ofToString(result));
        return std::nullopt;
    }

    // Check if this is a Laserdock device
    if (devicedescriptor.idVendor != LASERDOCK_VIN || devicedescriptor.idProduct != LASERDOCK_PIN) {
        return std::nullopt;
    }

    if (devicedescriptor.iSerialNumber == 0) {
        ofLogError("DacManagerLaserDock: device has no serial number string index.");
        return std::nullopt;
    }

    struct UsbGuard {
        libusb_device_handle* handle;
        ~UsbGuard() { if (handle) libusb_close(handle); }
    };
    
    libusb_device_handle* devhandlecontrol = nullptr;
    result = libusb_open(usbdevice, &devhandlecontrol);
    if (result != 0 || !devhandlecontrol) {
        ofLogError("DacManagerLaserDock: failed to open USB device. Error: " + ofToString(result));
        return std::nullopt;
    }
    UsbGuard guard{devhandlecontrol};

    unsigned char idstring[256] = {0};
    result = libusb_get_string_descriptor_ascii(devhandlecontrol, devicedescriptor.iSerialNumber, idstring, sizeof(idstring));
    std::optional<std::string> serial;

    if (result > 0) {
        serial = std::string(reinterpret_cast<char*>(idstring), result);
        ofLogNotice("DacManagerLaserDock: Found Laserdock with serial: " + *serial);
    } else {
        ofLogError("DacManagerLaserDock: failed to get serial number. Error: " + ofToString(result));
    }

    return serial;
}

void DacManagerLaserDock :: exit() {
    
}

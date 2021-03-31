//
//  ofxLaserDacDetectorBase.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#include "ofxLaserDacDetectorHelios.h"

using namespace ofxLaser;

DacDetectorHelios :: DacDetectorHelios()  {
    int rc;
    rc = libusb_init(NULL);
    if (rc < 0)
    {
        ofLogError("Error initializing libusb: "+ofToString(rc));
        //return false;
    }
    
}
DacDetectorHelios :: ~DacDetectorHelios()  {

    // TODO wait for all DACs threads to stop
    
    libusb_exit(NULL);
}

vector<DacData> DacDetectorHelios :: updateDacList(){
    
    vector<DacData> daclist;
    libusb_device **libusb_device_list;
    ssize_t cnt = libusb_get_device_list(NULL, &libusb_device_list);
    ssize_t i = 0;

    if (cnt < 0) {
        // LIBUSB ERROR
        ofLogError("ofxDacDetectorHelios :: getDacList() - USB error code " + ofToString(cnt));
        libusb_free_device_list(libusb_device_list, 1); // probably not necessary
        return daclist;
    }

    for (i = 0; i < cnt; i++) {
        
        // get the device from the array
        libusb_device *usbdevice = libusb_device_list[i];
        
        // checks to see if the device is a laserdock, and if so, returns the serial number.
        string serialnumber = getHeliosSerialNumber(usbdevice);
        
        if(serialnumber!="") {
            DacData data(getType(), serialnumber);
            daclist.push_back(data);
        }
            
    }

    libusb_free_device_list(libusb_device_list, cnt);
    return daclist;
    
}


DacBase* DacDetectorHelios :: getAndConnectToDac(const string& id){
    
    // returns a dac - if failed returns nullptr.
    
    DacHelios* dac = (DacHelios*) getDacById(id);
    
    if(dac!=nullptr) {
        ofLogNotice("DacDetectorHelios :: getAndConnectToDac(...) - Already a dac made with id "+ofToString(id));
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
        
        if (getHeliosSerialNumber(libusb_device) == id) {
            // check serial number, if it matches, then make a dac with it!
            dac = new DacHelios();
            if(!dac->setup(libusb_device)){ // *************************************
                delete dac;
                dac = nullptr;
                
            } else {
                // STORE DAC!
                dacsById[id] = dac;
            }
            break; 

        }
    }

    libusb_free_device_list(libusb_device_list, cnt);
    
    
    
    return dac;
}

bool DacDetectorHelios :: disconnectAndDeleteDac(const string& id){
    
    if ( dacsById.count(id) == 0 ) {
        ofLogError("DacDetectorHelios::disconnectAndDeleteDac("+id+") - dac not found");
        return false;
    }
    DacHelios* dac = (DacHelios*) dacsById.at(id);
    dac->close();
    auto it=dacsById.find(id);
    dacsById.erase(it);
    delete dac;
    return true; 
    
}


string DacDetectorHelios :: getHeliosSerialNumber(libusb_device* usbdevice) {
    // make a descriptor object to store the data in
    struct libusb_device_descriptor devicedescriptor;
    // get the descriptor
    
    string returnstring = "";
    
    int result = libusb_get_device_descriptor(usbdevice, &devicedescriptor);
    if (result < 0) {
        ofLogError("DacDetectorHelios failed to get device descriptor for USB device - error code " + ofToString(result));
        // skip to the next one
        return "";
    }
    
    if ((devicedescriptor.idVendor==HELIOS_VID)  && (devicedescriptor.idProduct==HELIOS_PID)) {
        unsigned char idstring[256];
        struct libusb_device_handle *devhandlecontrol;
        
        ofLogNotice("Found HELIOS! : "+ofToString(devicedescriptor.iSerialNumber));
        
        // open the device
        result = libusb_open(usbdevice, &devhandlecontrol);
        if(result!=0) {
            ofLogError("DacDetectorHelios failed to open USB device - error code " + ofToString(result));
            return "";
        }
        
        result = libusb_claim_interface(devhandlecontrol, 0);
        cout << "...libusb_claim_interface : " << result << endl;
        // seems to return LIBUSB_ERROR_ACCESS if it's busy
        if (result < 0) {
            libusb_close(devhandlecontrol);
            return "";
        }
        
        result = libusb_set_interface_alt_setting(devhandlecontrol, 0, 1);
        cout << "...libusb_set_interface_alt_setting : " << result << endl;
        if (result < 0) {
            libusb_close(devhandlecontrol);
            return "";
        }
        
        HeliosDacDevice* dac = new HeliosDacDevice(devhandlecontrol);
        cout << "...new dac device " << dac->nameStr << endl;
        if(dac->nameStr.empty()) {
            ofLogError("new dac name is wrong!");
        }
        returnstring = dac->nameStr;
        libusb_release_interface(devhandlecontrol,0);
        ofLogNotice("FOUND HELIOS NAME : "+returnstring);
        // should close down the dac and also clean up the devhandle
        delete dac;
       
      
    }
    return returnstring;

    
    
}

void DacDetectorHelios :: exit() {
    
}

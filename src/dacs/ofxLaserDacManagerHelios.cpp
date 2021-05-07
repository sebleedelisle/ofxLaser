//
//  ofxLaserDacManagerBase.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#include "ofxLaserDacManagerHelios.h"

using namespace ofxLaser;

DacManagerHelios :: DacManagerHelios()  {
    int rc;
    rc = libusb_init(NULL);
    if (rc < 0)
    {
        ofLogError("Error initializing libusb: "+ofToString(rc));
        //return false;
    }
    
}
DacManagerHelios :: ~DacManagerHelios()  {

    // TODO wait for all DACs threads to stop
    
    libusb_exit(NULL);
}

vector<DacData> DacManagerHelios :: updateDacList(){
    
    vector<DacData> daclist;
    
  
    libusb_device **libusb_device_list;
    ssize_t cnt = libusb_get_device_list(NULL, &libusb_device_list);
    ssize_t i = 0;

   
    // add data for dacs we already know about
    for(auto& dacpair : dacsById) {
        DacHelios* heliosdac = (DacHelios*) dacpair.second;
        DacData data(getType(), heliosdac->dacName);
        daclist.push_back(data);

    }
    
    if (cnt < 0) {
        // LIBUSB ERROR
        ofLogError("ofxDacManagerHelios :: getDacList() - USB error code " + ofToString(cnt));
        libusb_free_device_list(libusb_device_list, 1); // probably not necessary
        return daclist;
    }
    

    for (i = 0; i < cnt; i++) {
        
        // get the device from the array
        libusb_device *usbdevice = libusb_device_list[i];
        bool dacalreadyinuse = false;
        for(auto dacpair : dacsById) {
            DacHelios* dac = (DacHelios*)dacpair.second; 
            //dac->dacDevice;
            if(dac->usbDevice == usbdevice) {
                cout << "found dac already in use " << dac->dacName << endl;
                DacData data(getType(), dac->dacName);
                daclist.push_back(data);
                dacalreadyinuse = true;
                break;
            }
        }
        
        if(!dacalreadyinuse) {
            // checks to see if the device is a laserdock, and if so, returns the serial number.
            string serialnumber;
           // if(dacsById.size()==0)
                serialnumber = getHeliosSerialNumber(usbdevice);
            
            if(serialnumber!="") {
                DacData data(getType(), serialnumber);
                daclist.push_back(data);
            }
        }
    }

    
    
    libusb_free_device_list(libusb_device_list, (int)cnt);

    return daclist;
    
}


DacBase* DacManagerHelios :: getAndConnectToDac(const string& id){
    
    // returns a dac - if failed returns nullptr.
    
    DacHelios* dac = (DacHelios*) getDacById(id);
    
    if(dac!=nullptr) {
        ofLogNotice("DacManagerHelios :: getAndConnectToDac(...) - Already a dac made with id "+ofToString(id));
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

    libusb_free_device_list(libusb_device_list, (int)cnt);
    
    
    
    return dac;
}

bool DacManagerHelios :: disconnectAndDeleteDac(const string& id){
    
    if ( dacsById.count(id) == 0 ) {
        ofLogError("DacManagerHelios::disconnectAndDeleteDac("+id+") - dac not found");
        return false;
    }
    DacHelios* dac = (DacHelios*) dacsById.at(id);
    dac->close();
    auto it=dacsById.find(id);
    dacsById.erase(it);
    delete dac;
    return true; 
    
}


string DacManagerHelios :: getHeliosSerialNumber(libusb_device* usbdevice) {
    // make a descriptor object to store the data in
    struct libusb_device_descriptor devicedescriptor;
    // get the descriptor
    
    string returnstring = "";
    
    int result = libusb_get_device_descriptor(usbdevice, &devicedescriptor);
    if (result < 0) {
        ofLogError("DacManagerHelios :: getHeliosSerialNumber failed to get device descriptor for USB device - error code " + ofToString(result));
        // skip to the next one
        return "";
    }
    
    if ((devicedescriptor.idVendor==HELIOS_VID)  && (devicedescriptor.idProduct==HELIOS_PID)) {
        unsigned char idstring[256];
        struct libusb_device_handle *usbHandle;
        
     //   ofLogNotice("Found HELIOS! : "+ofToString(devicedescriptor.iSerialNumber));
        
        // open the device
        result = libusb_open(usbdevice, &usbHandle);
        if(result!=0) {
            ofLogError("DacManagerHelios :: getHeliosSerialNumber failed to open USB device - error code " + ofToString(result));
            return "";
        }
        
        
        
        result = libusb_claim_interface(usbHandle, 0);
     //   cout << "...libusb_claim_interface : " << result << endl;
        // seems to return LIBUSB_ERROR_ACCESS if it's busy
        if (result < 0) {
			ofLogError("DacManagerHelios :: getHeliosSerialNumber failed to claim interface - error code " + ofToString(result));

            libusb_close(usbHandle);
            return "";
        }
       
        result = libusb_set_interface_alt_setting(usbHandle, 0, 1);
      // cout << "...libusb_set_interface_alt_setting : " << result << endl;
        //if ((result < 0) ||(dacsById.size()>0)) {
        if (result < 0) {
			ofLogError("DacManagerHelios :: getHeliosSerialNumber libusb_set_interface_alt_setting failed - error code " + ofToString(result));

            libusb_release_interface(usbHandle, 0);
            libusb_close(usbHandle);
            return "";
        }

        // TO GET THE NAME :
		// Do we need to tell the Helios the SDK number and stuff?
		// Send control command 0x05 (which presumably asks for the name
		int actualLength = 0;
		std::uint8_t ctrlCommand[2] = { 0x05, 0 };
		
		// note that the last param is the timeout in MS
		int transferResult = libusb_interrupt_transfer(usbHandle, EP_INT_OUT, ctrlCommand, 2, &actualLength, 16);

		// do an interrupt transfer to get the name
		if (transferResult == LIBUSB_SUCCESS) {
			std::uint8_t receiveBytes[32];
			int transferResult = libusb_interrupt_transfer(usbHandle, EP_INT_IN, receiveBytes, 32, &actualLength, 32);
			if(transferResult == LIBUSB_SUCCESS) {
				// check that the first byte is 0x85 (presumably some
				// kind of confirmation value
				if(receiveBytes[0] == 0x85) {
					// terminate the string with a null
					receiveBytes[31] = '\0';
					// and copy all but the first character into a
					// new string
					char name[32];
					memcpy(name, &receiveBytes[1], 31);
					returnstring = name;
				} else {
					ofLogError("DacManagerHelios Helios device didn't return valid name");
				}
			} else {
				ofLogError("DacManagerHelios Helios device didn't give us its name properly");
			}
			
		} else {
			ofLogError("DacManagerHelios Helios device USB read error "+ofToString(transferResult));
		}

		
		
        //returnstring = dacdevice->nameStr;
        int res = libusb_release_interface(usbHandle,0);
		if (res < 0) {
			ofLogError("DacManagerHelios libusb_release_interface failed " + ofToString(res)); 
		}

		libusb_close(usbHandle);
        ofLogNotice("FOUND HELIOS NAME : "+returnstring);
        // should close down the dac and also clean up the devhandle
        //delete dacdevice;
       
      
    }
    return returnstring;

    
    
}

void DacManagerHelios :: exit() {
    
}

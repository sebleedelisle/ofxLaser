//
//  ofxLaserDacHeliosManager.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 03/10/2020.
//

#include "ofxLaserDacHeliosManager.h"

using namespace ofxLaser;


DacHeliosManager::DacHeliosManager() {
	
	ofLogNotice("DacHeliosManager constructor");
    //NB it doesn't matter how many times you call libusb_init - if there's
    // already a default context it'll reuse it and not initialise it again
	int result = libusb_init(NULL);
	if (result < 0) {
		ofLogError("DacHeliosManager failed to init libusb");
        throw; 
    } else {
	
        libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL);
        updateUsbDeviceList();
       
    }
	
}
DacHeliosManager::~DacHeliosManager() {
	ofLogNotice("DacHeliosManager destructor");
	//for(HeliosDacDevice* dac :
	ofLogNotice("...deleting " + ofToString(usedDevices.size()) + " devices...");
	
	for(HeliosDacDevice* dac : usedDevices) {
	//
		dac->SetClosed(); // also closes USB connection and waits for thread to finish
		
	}
	ofLogNotice("...deleting " + ofToString(availableDevices.size()) + " devices...");
	
	for(HeliosDacDevice* dac : availableDevices) {
		dac->SetClosed();
		delete dac; // also closes USB connection and waits for thread to finish
		
	}
	
	// how to wait until threads have finish?
	libusb_exit(NULL);
}

void DacHeliosManager::updateUsbDeviceList() {
	ofLogNotice("DacHeliosManager::updateUsbDeviceList");
	libusb_device** devs;
	
	
	ssize_t numusbdevices = libusb_get_device_list(NULL, &devs);
	
	// if numusbdevices<0 then we have some error... shouldn't matter though
	
	for (int i = 0; i < numusbdevices; i++){
		
		struct libusb_device_descriptor devDesc;
		
		int result = libusb_get_device_descriptor(devs[i], &devDesc);
		if (result < 0)
			continue;

	
		if(isHeliosDevice(devDesc)) {
			
			
//			cout << "bLength \t: " << devDesc.bLength << endl;
//			cout << "bDescriptorType \t: " << devDesc.bDescriptorType<< endl;
//			cout << "bcdUSB \t: " << devDesc.bcdUSB<< endl;
//
//			cout << "bDeviceClass \t: " << devDesc.bDeviceClass<< endl;
//			cout << "bDeviceSubClass \t: " << devDesc.bDeviceSubClass<< endl;
//
//			cout << "bDeviceProtocol \t: " << devDesc.bDeviceProtocol<< endl;
//			cout << "bMaxPacketSize0 \t: " << devDesc.bMaxPacketSize0<< endl;
//
//			cout << "idVendor \t: " << devDesc.idVendor<< endl;
//			cout << "idProduct \t: " << devDesc.idProduct<< endl;
//			cout << "bcdDevice \t: " << devDesc.bcdDevice<< endl;
//
//			cout << "iManufacturer \t: " << devDesc.iManufacturer<< endl;
//			cout << "iProduct \t: " << devDesc.iProduct<< endl;
//
//			cout << "iSerialNumber \t: " << devDesc.iSerialNumber<< endl;
//
//			cout << "bNumConfigurations \t: " << devDesc.bNumConfigurations<< endl << endl;;
			
			// note that handles are unique, even for duplicate devices
			libusb_device_handle* devHandle;
			result = libusb_open(devs[i], &devHandle);
			cout << "...libusb_open result : " << result << endl;
			
			//libusb_get_device()
			
			if (result < 0) {
				// do something
				continue; // on to next USB device
			} else {
				// do we already have a DAC with this handle?
				
			}
			
			result = libusb_claim_interface(devHandle, 0);
			cout << "...libusb_claim_interface : " << result << endl;
			// seems to return LIBUSB_ERROR_ACCESS if it's busy
			if (result < 0) {
				// do something
				continue; // on to next USB device
			}
			
			result = libusb_set_interface_alt_setting(devHandle, 0, 1);
			cout << "...libusb_set_interface_alt_setting : " << result << endl;
			if (result < 0) {
				// do something
				continue; // on to next USB device
			}
			
			HeliosDacDevice* dac = new HeliosDacDevice(devHandle);
			cout << "...new dac device " << dac->nameStr << endl;
			if(dac->nameStr.empty()) {
				ofLogError("new dac name is wrong!");
			}
			
			//string dacname=dac->GetName();
			//deviceByName[dac->name] = dac;
			availableDevices.push_back(dac); 
			
		}
		
	}
	
	libusb_free_device_list(devs, 1);
	
	ofLogNotice("Available devices  : "+ofToString(availableDevices.size()));
	ofLogNotice("Used devices  : "+ofToString(usedDevices.size()));

	
}

bool DacHeliosManager::deleteDevice(HeliosDacDevice* deviceToDelete){
	ofLogNotice("DacHeliosManager::deleteDevice " + deviceToDelete->nameStr);
	
	for(size_t i = 0; i<usedDevices.size(); i++) {
		HeliosDacDevice* device = usedDevices.at(i);
		if(deviceToDelete == device) {
			ofLogNotice("...found device to delete! "+device->nameStr);
			usedDevices.erase(usedDevices.begin()+i);
			delete device;
			return true;
		}
	}
	ofLogError("...device not found! "+deviceToDelete->nameStr);
	
	return false;
	
	
}

HeliosDacDevice*  DacHeliosManager::getDacDeviceForName(string name) {
	ofLogNotice("DacHeliosManager::getDacDeviceForName " + name);
    
	 
	 
	for(size_t i = 0; i<availableDevices.size(); i++) {
		HeliosDacDevice* device = availableDevices.at(i);
		//if(device->nameStr == "")
		ofLogNotice(ofToString(i) + " : device "+ device->nameStr + " " + device->GetName());
		if(name.empty() || (name.compare(device->GetName())==0)) {
			// if we found a device, pull it out of the vector
			ofLogNotice("... found device "+ device->nameStr);
			availableDevices.erase(availableDevices.begin()+i);
			usedDevices.push_back(device);
			return device;
			
		}
			
		
	}
	// if we had no luck, update the list for next time...
	
	 
	return nullptr;
	
	
}

bool DacHeliosManager::isHeliosDevice(libusb_device_descriptor& devDesc) {
	return ((devDesc.idProduct == HELIOS_PID) && (devDesc.idVendor == HELIOS_VID));
}



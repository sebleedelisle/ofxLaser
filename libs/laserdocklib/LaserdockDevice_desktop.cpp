//
// Created by Guoping Huang on 8/8/16.
//

#include "LaserdockDevice.h"
#include "LaserdockDevice_p.h"

#include <libusb.h>


void LaserdockDevicePrivate::initialize() {
    int r = 0;

    r = libusb_open(this->usbdevice, &this->devh_ctl);
    if (r != 0) return;
    r = libusb_open(this->usbdevice, &this->devh_data);
    if (r != 0) return;
	
	
	//---------------------------------------
	
	struct libusb_device_descriptor desc;
	unsigned char mystring[256];
	int ret;
	ret = libusb_get_device_descriptor(this->usbdevice, &desc);
	if (desc.iSerialNumber) {
		//ret = libusb_get_string_descriptor_ascii(&this->devh_data, desc.iSerialNumber, string, sizeof(string));
		ret = libusb_get_string_descriptor_ascii(this->devh_data, desc.iSerialNumber, mystring, sizeof(mystring));
		this->serial_number = (char*)mystring;
		//if (ret > 0) printf("Serial Number: %s\n", this->serial_number.c_str());
		ret = libusb_get_string_descriptor_ascii(this->devh_ctl, desc.iSerialNumber, mystring, sizeof(mystring));
		this->serial_number = (char*)mystring;
		//if (ret > 0)	printf("Serial Number: %s\n", this->serial_number.c_str());
	}
	
	
	//-------------------------------------
	
	
    r = libusb_claim_interface(this->devh_ctl, 0);
    if (r != 0) return;

    r = libusb_claim_interface(this->devh_data, 1);
    if (r != 0) return;

    r = libusb_set_interface_alt_setting(this->devh_data, 1, 1);
    if (r != 0) return;

    this->status = LaserdockDevice::Status::INITIALIZED;
}


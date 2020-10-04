//
// Created by Guoping Huang on 9/6/16.
//

#ifndef LASERDOCKLIB_LASERDOCKDEVICEMANAGERPRIVATE_H
#define LASERDOCKLIB_LASERDOCKDEVICEMANAGERPRIVATE_H

#include <memory>
#include <vector>

class LaserdockDeviceManager;

class libusb_device;
class libusb_context;

class LaserdockDeviceManagerPrivate {

public:
    LaserdockDeviceManagerPrivate(LaserdockDeviceManager *q_ptr);
	~LaserdockDeviceManagerPrivate();

    bool initialize_usb();
    bool is_laserdock(libusb_device * device) const;

    std::vector<std::unique_ptr<LaserdockDevice>> get_devices();

private:
    LaserdockDeviceManager * q;
    libusb_context *m_libusb_ctx;
};


#endif //LASERDOCKLIB_LASERDOCKDEVICEMANAGER_H

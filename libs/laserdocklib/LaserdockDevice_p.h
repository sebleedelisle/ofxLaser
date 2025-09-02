//
// Created by Guoping Huang on 8/8/16.
//

#ifndef LASERDOCKLIB_LASERDOCKDEVICEPRIVATE_H
#define LASERDOCKLIB_LASERDOCKDEVICEPRIVATE_H

#include <string>

#ifdef ANDROID
class _jobject;
typedef _jobject* jobject;
#endif

class LaserdockDevice;
class libusb_device;

class LaserdockDevicePrivate {
public:
    struct libusb_device_handle *devh_ctl;
    struct libusb_device_handle *devh_data;
    libusb_device * usbdevice;
	std::string serial_number;
    bool flipx;
    bool flipy;
    LaserdockDevice::Status status;

#ifdef ANDROID
    jobject m_jobject;
    LaserdockDevicePrivate(libusb_device * device, jobject jobj, LaserdockDevice * q_ptr);
#endif

    LaserdockDevicePrivate(libusb_device * device, LaserdockDevice * q_ptr);
    virtual ~LaserdockDevicePrivate();

    void initialize();
    void release();
    void print() const;

private:
    LaserdockDevice * q;
};



#endif //LASERDOCKLIB_LASERDOCKDEVICE_H

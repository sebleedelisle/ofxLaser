#include "LaserdockDeviceManager.h"
#include "LaserdockDeviceManager_p.h"

#include <libusb.h>

#include "LaserdockDevice.h"

std::vector<std::unique_ptr<LaserdockDevice> > LaserdockDeviceManagerPrivate::get_devices() {
    std::vector<std::unique_ptr<LaserdockDevice>> laserdockDevices;

    libusb_device **libusb_device_list;
    ssize_t cnt = libusb_get_device_list(NULL, &libusb_device_list);
    ssize_t i = 0;

    if (cnt < 0) {
        fprintf(stderr, "Error finding USB device\n");
        libusb_free_device_list(libusb_device_list, 1); // probably not necessary
        return laserdockDevices;
    }

    for (i = 0; i < cnt; i++) {
        libusb_device *libusb_device = libusb_device_list[i];
        if (is_laserdock(libusb_device)) {
            std::unique_ptr<LaserdockDevice> d(new LaserdockDevice(libusb_device));
            if(d->status() == LaserdockDevice::Status::INITIALIZED)
                laserdockDevices.push_back(std::move(d));
        }
    }

    libusb_free_device_list(libusb_device_list, cnt);

    return laserdockDevices;
}

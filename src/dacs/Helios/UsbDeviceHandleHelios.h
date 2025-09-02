#pragma once

#include <libusb.h>
#include <cstdio>
#include <stdexcept>

class UsbDeviceHandleHelios {
public:
    UsbDeviceHandleHelios(libusb_device* device)
            : device(device), handle(nullptr)
        {
            iface0 = 0; 
            int r = libusb_open(device, &handle);
            if (r != 0 || !handle) {
                throw std::runtime_error("Failed to open USB device: " + std::string(libusb_error_name(r)));
            }

            claimInterface(iface0);

        }

        ~UsbDeviceHandleHelios() {
            
            releaseInterface(iface0);

            if (handle) {
                libusb_close(handle);
            }
        }

    
    libusb_device_handle* getHandle() const {
        return handle;
    }
    
    libusb_device* getDevice() const { return device; }
    
private:
    libusb_device* device;
    libusb_device_handle* handle;
    int iface0, iface1;

    void claimInterface(int iface) {
        int r = libusb_claim_interface(handle, iface);
        if (r != 0) {
            throw std::runtime_error("Failed to claim interface " + std::to_string(iface) + ": " + libusb_error_name(r));
        }
        r = libusb_set_interface_alt_setting(handle, 0, 1); // critical for Helios
        if (r != 0) {
            throw std::runtime_error("Failed to set the alt " + std::to_string(iface) + ": " + libusb_error_name(r));
        }
    }

    void releaseInterface(int iface) {
        if (handle) {
            int r = libusb_release_interface(handle, iface);
            if (r != 0) {
                fprintf(stderr, "Warning: failed to release interface %d: %s\n", iface, libusb_error_name(r));
            }
        }
    }
};

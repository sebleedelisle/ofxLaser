//
//  ofxLaserDacManagerBase.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#include "ofxLaserDacManagerHelios.h"
#include <chrono>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace {

std::optional<std::string> getUsbPortPath(libusb_device* device) {
    if (!device) {
        return std::nullopt;
    }

    uint8_t ports[16];
    int depth = libusb_get_port_numbers(device, ports, sizeof(ports));
    if (depth <= 0) {
        return std::nullopt;
    }

    std::string path;
    path.reserve(depth * 3);
    for (int i = 0; i < depth; i++) {
        if (i > 0) {
            path += "-";
        }
        path += std::to_string(ports[i]);
    }
    return path;
}

}

using namespace ofxLaser;

DacManagerHelios :: DacManagerHelios()  {
    
    int rc = libusb_init(&ctx);
    if ( rc< 0) {
        ofLogError("Error initializing libusb: "+ofToString(rc));
        //return false;
    }
    
}
DacManagerHelios :: ~DacManagerHelios()  {

    // TODO wait for all DACs threads to stop
    if (ctx) {
        libusb_exit(ctx);
        ctx = nullptr;
    }
}

vector<DacData> DacManagerHelios :: updateDacList(){
    
    vector<DacData> daclist;
    std::unordered_map<std::string, std::shared_ptr<DacHelios>> dacsByPortPath;
       
    // add data for dacs we already know about
    for(auto& dacpair : dacsById) {
        std::shared_ptr<DacHelios> heliosdac = std::dynamic_pointer_cast<DacHelios>(dacpair.second);
        DacData data(getType(), heliosdac->dacName);
        daclist.push_back(data);

        std::optional<std::string> portPath = getUsbPortPath(heliosdac->getDevice());
        if (portPath) {
            dacsByPortPath.emplace(*portPath, heliosdac);
        }

    }
    
    libusb_device **libusb_device_list;
    ssize_t cnt = libusb_get_device_list(ctx, &libusb_device_list);
    ssize_t i = 0;
    
    if (cnt < 0 || libusb_device_list == nullptr) {
        // LIBUSB ERROR
        ofLogError("ofxDacManagerHelios :: getDacList() - USB error code " + ofToString(cnt));
        return daclist;
    }
    

    for (i = 0; i < cnt; i++) {
        
        // get the device from the array
        libusb_device *usbdevice = libusb_device_list[i];
        std::optional<std::string> portPath = getUsbPortPath(usbdevice);
        if (portPath && dacsByPortPath.count(*portPath) > 0) {
            continue; // already known and connected
        }

        // checks to see if the device is a Helios, and if so, returns the serial number.
        std::unique_ptr<UsbDeviceHandleHelios> handle = getHandleIfDeviceIsHelios(usbdevice);
        std::optional<std::string> dacname = getHeliosNameForUsbHandle(handle);
        if(handle && dacname) {
            daclist.emplace_back(getType(), *dacname);
        }
    }

    libusb_free_device_list(libusb_device_list, (int)cnt);

    return daclist;
    
}


std::shared_ptr<DacBase> DacManagerHelios :: getAndConnectToDac(const string& id){
    
    // if we already have a dac, return that one!
    
    if(std::shared_ptr<DacBase> existingDac =  getDacById(id)) {
        ofLogNotice("DacManagerHelios :: getAndConnectToDac(...) - Already a dac made with id "+ofToString(id));
        return existingDac;
    }
    
    std::shared_ptr<DacBase> dac;
    
    libusb_device **libusb_device_list;
    ssize_t cnt = libusb_get_device_list(ctx, &libusb_device_list);
    ssize_t i = 0;

    if (cnt < 0) {
        fprintf(stderr, "Error finding USB device\n");
          return nullptr;
    }

    std::unordered_set<std::string> existingPortPaths;
    for (auto& dacpair : dacsById) {
        std::shared_ptr<DacHelios> dac = std::dynamic_pointer_cast<DacHelios>(dacpair.second);
        std::optional<std::string> portPath = getUsbPortPath(dac->getDevice());
        if (portPath) {
            existingPortPaths.insert(*portPath);
        }
    }

    for (i = 0; i < cnt; i++) {
        libusb_device *device = libusb_device_list[i];

        std::optional<std::string> portPath = getUsbPortPath(device);
        if (portPath && existingPortPaths.count(*portPath) > 0) {
            continue;
        }
        
        std::unique_ptr<UsbDeviceHandleHelios> handle = getHandleIfDeviceIsHelios(device);
        std::optional<std::string> dacname = getHeliosNameForUsbHandle(handle);
        if(handle && dacname && (*dacname==id)) {
            std::shared_ptr<DacHelios>  hdac = std::make_shared<DacHelios>();
            if(hdac->setup(std::move(handle))){
                // STORE DAC!
                dacsById.emplace(id, hdac);
                dac = hdac;
            } else {
                // function itself should log errors
            }
            break;
        }
    }

    libusb_free_device_list(libusb_device_list, (int)cnt);

    return dac;
}

std::unique_ptr<UsbDeviceHandleHelios> DacManagerHelios::getHandleIfDeviceIsHelios(libusb_device* usbdevice) {
    
    libusb_device_descriptor devicedescriptor;
    int result = libusb_get_device_descriptor(usbdevice, &devicedescriptor);
    if (result < 0) {
        ofLogError("Helios: Failed to get device descriptor. Code: " + ofToString(result));
        return nullptr;
    }
    
    // Only handle devices with the correct VID/PID
    if (devicedescriptor.idVendor != HELIOS_VID || devicedescriptor.idProduct != HELIOS_PID) {
        return nullptr;
    }
    try {
        return std::make_unique<UsbDeviceHandleHelios>(usbdevice);
    } catch (const std::exception& e) {
        ofLogError("Helios") << "Failed to open/claim device: " << e.what();
        return nullptr;
    }

}


std::optional<std::string> DacManagerHelios::getHeliosNameForUsbHandle(std::unique_ptr<UsbDeviceHandleHelios>& handle){

    if (!handle || !handle->getHandle()) {
        //ofLogError("Helios") << "Invalid USB handle";
        return std::nullopt;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Drain any lingering interrupt IN transfers before the first command.
    {
        std::uint8_t drainBuffer[32];
        int actualLength = 0;
        while (libusb_interrupt_transfer(handle->getHandle(), EP_INT_IN, drainBuffer, sizeof(drainBuffer), &actualLength, 5) == LIBUSB_SUCCESS) {
        }
    }

    // Match SDK init sequence: firmware query then SDK version.
    {
        std::uint8_t fwCommand[2] = { 0x04, 0 };
        bool gotFirmware = false;
        for (int i = 0; (i < 2) && !gotFirmware; i++) {
            int actualLength = 0;
            int result = libusb_interrupt_transfer(handle->getHandle(), EP_INT_OUT, fwCommand, 2, &actualLength, 32);
            if (result == LIBUSB_SUCCESS && actualLength == 2) {
                for (int j = 0; (j < 3) && !gotFirmware; j++) {
                    std::uint8_t fwResponse[32];
                    result = libusb_interrupt_transfer(handle->getHandle(), EP_INT_IN, fwResponse, sizeof(fwResponse), &actualLength, 64);
                    if (result == LIBUSB_SUCCESS && fwResponse[0] == 0x84) {
                        gotFirmware = true;
                    }
                }
            }
            if (!gotFirmware) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
    }

    {
        std::uint8_t sdkVersion[2] = { 0x07, HELIOS_SDK_VERSION };
        for (int i = 0; i < 2; i++) {
            int actualLength = 0;
            int result = libusb_interrupt_transfer(handle->getHandle(), EP_INT_OUT, sdkVersion, 2, &actualLength, 32);
            if (result == LIBUSB_SUCCESS && actualLength == 2) {
                break;
            }
            if (result == LIBUSB_ERROR_NO_DEVICE) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }

    // Send control command to ask for the DAC name
    std::uint8_t ctrlCommand[2] = { 0x05, 0 };
    int actualLength = 0;
    int result = LIBUSB_ERROR_OTHER;
    for (int i = 0; i < 5; i++) { // retry command if necessary
        result = libusb_interrupt_transfer(handle->getHandle(), EP_INT_OUT, ctrlCommand, 2, &actualLength, 32);
        if (result == LIBUSB_SUCCESS) {
            break;
        }
        if (result == LIBUSB_ERROR_NO_DEVICE) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    if (result != LIBUSB_SUCCESS) {
        ofLogError("Helios: Failed to send name request. Code: " + ofToString(result));
        return std::nullopt;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    // Receive the DAC name response
    std::uint8_t receiveBytes[32] = {0};
    for (int i = 0; i < 5; i++) { // retry response if necessary
        result = libusb_interrupt_transfer(handle->getHandle(), EP_INT_IN, receiveBytes, 32, &actualLength, 100);
        if (result == LIBUSB_SUCCESS) {
            break;
        }
        if (result == LIBUSB_ERROR_NO_DEVICE) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    if (result != LIBUSB_SUCCESS) {
        ofLogError("Helios: Failed to receive name. Code: " + ofToString(result));
        return std::nullopt;
    }

    // Check for valid name response
    if (actualLength >= 2 && receiveBytes[0] == 0x85) {
        int nameLength = std::min(actualLength - 1, 31);
        char name[32];
        memcpy(name, &receiveBytes[1], nameLength);
        name[nameLength] = '\0';
        return std::string(name);
    } else {
        ofLogError("Helios: Invalid name response.");
        return std::nullopt;
    }
    
}



void DacManagerHelios :: exit() {
    
}

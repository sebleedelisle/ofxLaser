//
// Created by Guoping Huang on 8/8/16.
//
// Simplified and fortified by Seb Lee-Delisle on 24/05/25
//

#include "LaserdockDevice.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <libusb.h>

#include "UsbDeviceHandleHelios.h"


/// ---------------------------- anonymous namespace ----------------------------

namespace {

    bool read_uint8(libusb_device_handle *handle, uint8_t command, uint8_t *value){
        int rv = 0;
        int transferred = 0;
        unsigned char packet[64]; packet[0] = command;
        int length = 1;

        rv = libusb_bulk_transfer(handle, (1 | LIBUSB_ENDPOINT_OUT), packet, length, &transferred, 0);
        if(rv != 0 || transferred != length)
            return false;

        rv = libusb_bulk_transfer(handle, (1 | LIBUSB_ENDPOINT_IN), packet, 64, &transferred, 0);
        if(rv != 0 || transferred != 64 || packet[1] != 0)
        {
            return false;
        }
        *value = packet[2];

        return true;
    }

    bool send_uint8(libusb_device_handle *handle, uint8_t command, uint8_t value){

        int rv = 0;
        int transferred = 0;
        unsigned char packet[64]; packet[0] = command; packet[1] = value;
        int length = 2;

        rv = libusb_bulk_transfer(handle, (1 | LIBUSB_ENDPOINT_OUT), packet, length, &transferred, 0);
        if(rv != 0 || transferred != length)
            return false;

        rv = libusb_bulk_transfer(handle, (1 | LIBUSB_ENDPOINT_IN), packet, 64, &transferred, 0);
        if(rv != 0 || transferred != 64 || packet[1] != 0)
        {
            return false;
        }

        return true;
    }

    bool read_uint32(libusb_device_handle *handle, uint8_t command, uint32_t *value){
        int rv = 0;
        int transferred = 0;
        unsigned char packet[64]; packet[0] = command;
        int length = 1;

        rv = libusb_bulk_transfer(handle, (1 | LIBUSB_ENDPOINT_OUT), packet, length, &transferred, 0);
        if(rv != 0 || transferred != length)
            return false;

        rv = libusb_bulk_transfer(handle, (1 | LIBUSB_ENDPOINT_IN), packet, 64, &transferred, 0);
        if(rv != 0 || transferred != 64 || packet[1] != 0)
        {
            return false;
        }

        memcpy(value, packet + 2, sizeof(uint32_t));
//#ifdef BIG_ENDIAN
//        *value  = __builtin_bswap32(*value);
//#endif
        return true;
    }

    bool send_uint32(libusb_device_handle *handle, uint8_t command, uint32_t value){
//#ifdef BIG_ENDIAN
//        value  = __builtin_bswap32(value);
//#endif
        int rv = 0;
        int transferred = 0;
        unsigned char packet[64]; packet[0] = command;
        int length = 1 + sizeof(uint32_t);
        memcpy(packet + 1, &value, sizeof(uint32_t));

        rv = libusb_bulk_transfer(handle, (1 | LIBUSB_ENDPOINT_OUT), packet, length, &transferred, 0);
        if(rv != 0 || transferred != length)
            return false;

        rv = libusb_bulk_transfer(handle, (1 | LIBUSB_ENDPOINT_IN), packet, 64, &transferred, 0);
        if(rv != 0 || transferred != 64 || packet[1] != 0)
        {
            return false;
        }

        return true;
    }

    bool send_raw(libusb_device_handle *handle, uint8_t* request, uint32_t rlen, uint8_t* response){
        int rv = 0;
        int transferred = 0;
        unsigned char packet[64];
        int length = rlen;
        memcpy(packet, request, length);

        rv = libusb_bulk_transfer(handle, (1 | LIBUSB_ENDPOINT_OUT), packet, length, &transferred, 0);
        if(rv != 0 || transferred != length)
            return false;

        rv = libusb_bulk_transfer(handle, (1 | LIBUSB_ENDPOINT_IN), response, 64, &transferred, 0);
        if(rv != 0 || transferred != 64 || packet[1] != 0)
        {
            return false;
        }

        return true;
    }
}


/// ---------------------------- LaserdockDevice ----------------------------

LaserdockDevice::LaserdockDevice(libusb_device* device)
    : usbDevice(device)
{
    try {
        usbHandle = std::make_unique<UsbDeviceHandleLaserDock>(device); // claims both interfaces
        initialize();
        deviceStatus = INITIALIZED;
    } catch (const std::exception& e) {
        fprintf(stderr, "[Laserdock] Error initializing device: %s\n", e.what());
        usbHandle.reset();
        deviceStatus = UNKNOWN;
    }
}

LaserdockDevice::~LaserdockDevice() {
    
//    if (usbHandle) {
//        libusb_release_interface(usbHandle, 0);
//        libusb_release_interface(usbHandle, 1);
//        libusb_close(usbHandle);
//    }
}


void LaserdockDevice::initialize() {
//    int r = libusb_open(usbDevice, &usbHandle);
//    if (r != 0 || !usbHandle) {
//        fprintf(stderr, "[Laserdock] Failed to open device: %s\n", libusb_error_name(r));
//        deviceStatus = UNKNOWN;
//        return;
//    }
//
//    r = libusb_claim_interface(usbHandle, 0);
//    if (r != 0) {
//        fprintf(stderr, "[Laserdock] Failed to claim interface 0: %s\n", libusb_error_name(r));
//        libusb_close(usbHandle);
//        usbHandle = nullptr;
//        return;
//    }
//
//    r = libusb_claim_interface(usbHandle, 1);
//    if (r != 0) {
//        fprintf(stderr, "[Laserdock] Failed to claim interface 1: %s\n", libusb_error_name(r));
//        libusb_release_interface(usbHandle, 0);
//        libusb_close(usbHandle);
//        usbHandle = nullptr;
//        return;
//    }
//
//    libusb_set_interface_alt_setting(usbHandle, 1, 1);

    libusb_device_descriptor desc;
    unsigned char str[256] = {};
    if (libusb_get_device_descriptor(usbDevice, &desc) == 0 && desc.iSerialNumber) {
        if (libusb_get_string_descriptor_ascii(usbHandle->getHandle(), desc.iSerialNumber, str, sizeof(str)) > 0) {
            serialNumber = reinterpret_cast<char*>(str);
        }
    }

    deviceStatus = INITIALIZED;
}



bool LaserdockDevice::enable_output() {
    if(!usbHandle) return false;
    return send_uint8(usbHandle->getHandle(), 0x80, 0x01);
}



bool LaserdockDevice::usb_send(unsigned char * data, int length){
    //printf("sending usb, numbytes %d.\n", numbytes);
    if(!usbHandle) return false;
    int r, actual;
    r = libusb_bulk_transfer(usbHandle->getHandle(), (1 | LIBUSB_ENDPOINT_OUT), data, length, &actual, 0);

    if(r != 0 || length != actual)
        return false;

    return true;
}


unsigned char *LaserdockDevice::usb_get(unsigned char * data, int length){

    int r, actual;

    r = libusb_bulk_transfer(usbHandle->getHandle(), (1 | LIBUSB_ENDPOINT_OUT), data, length, &actual, 0);
    if(r != 0 || actual != length)
        return NULL;

    unsigned char * response = (unsigned char *)calloc(64, 1);

    r = libusb_bulk_transfer(usbHandle->getHandle(), (1 | LIBUSB_ENDPOINT_IN), response, 64, &actual, 0);

    if(r != 0 || actual != 64 || response[1] != 0)
    {
        printf("Read Error: %d, %d\n",  r, actual);
        //free(response);
        return NULL;
    }

    return response;
}

bool LaserdockDevice::get_output(bool *enabled) {
    uint8_t enabled8 =0;
    bool success =  read_uint8(usbHandle->getHandle(), 0x81, &enabled8);
    *enabled = (enabled8 == 1)? true : false;
    return success;
}

bool LaserdockDevice::disable_output() {
    return send_uint8(usbHandle->getHandle(), 0x80, 0x00);
}

bool LaserdockDevice::dac_rate(uint32_t *rate) {
    return read_uint32(usbHandle->getHandle(), 0X83, rate);
}

bool LaserdockDevice::set_dac_rate(uint32_t rate) {
    return send_uint32(usbHandle->getHandle(), 0x82, rate);
}

bool LaserdockDevice::max_dac_rate(uint32_t *rate) {
    return read_uint32(usbHandle->getHandle(), 0X84, rate);
}

bool LaserdockDevice::min_dac_value(uint32_t *value) {
    return read_uint32(usbHandle->getHandle(), 0x87, value);
}

bool LaserdockDevice::max_dac_value(uint32_t *value) {
    return read_uint32(usbHandle->getHandle(), 0x88, value);
}


bool LaserdockDevice::sample_element_count(uint32_t *count) {
    return read_uint32(usbHandle->getHandle(), 0X85, count);
}

bool LaserdockDevice::iso_packet_sample_count(uint32_t *count) {
    return read_uint32(usbHandle->getHandle(), 0x86, count);
}

bool LaserdockDevice::bulk_packet_sample_count(uint32_t *count) {
    return read_uint32(usbHandle->getHandle(), 0x8E, count);
}

bool LaserdockDevice::version_major_number(uint32_t *major) {
    return read_uint32(usbHandle->getHandle(), 0X8B, major);
}

bool LaserdockDevice::version_minor_number(uint32_t *minor) {
    return read_uint32(usbHandle->getHandle(), 0X8C, minor);
}


bool LaserdockDevice::ringbuffer_sample_count(uint32_t *count) {
    return read_uint32(usbHandle->getHandle(), 0X89, count);
}

bool LaserdockDevice::ringbuffer_empty_sample_count(uint32_t *count) {
    return read_uint32(usbHandle->getHandle(), 0X8A, count);
}

bool LaserdockDevice::send(unsigned char *data, uint32_t length) {

    int timeout_strikes = 3;

    int rv = 0; int transferred = 0;
    do {
        rv = libusb_bulk_transfer(usbHandle->getHandle(), (3 | LIBUSB_ENDPOINT_OUT), (unsigned char*) data, length,
                                 &transferred, 0);
        if(rv==LIBUSB_ERROR_TIMEOUT){
            timeout_strikes--;
        }
    } while ( rv == LIBUSB_ERROR_TIMEOUT && timeout_strikes != 0);

    if (rv < 0) {
        return false;
    }

    return true;
}

bool LaserdockDevice::send_samples(LaserdockSample *samples, uint32_t count) {
    return this->send((unsigned char *) samples, sizeof(LaserdockSample)*count);
}

bool LaserdockDevice::clear_ringbuffer() {
    return send_uint8(usbHandle->getHandle(), 0x8D, 0);
}


bool LaserdockDevice::runner_mode_enable(bool v) {
    uint8_t request[] = {0xC0, 0x01, v? (uint8_t)0x01: (uint8_t)0x00};
    uint32_t rlen = 3;
    uint8_t response[64];
    bool r =  send_raw(usbHandle->getHandle(), request, rlen, response);
    return r;
}

bool LaserdockDevice::runner_mode_run(bool v) {
    uint8_t request[] = {0xC0, 0x09, v? (uint8_t)0x01: (uint8_t)0x00};
    uint32_t rlen = 3;
    uint8_t response[64];
    bool r =  send_raw(usbHandle->getHandle(), request, rlen, response);
    return r;
}

bool LaserdockDevice::runner_mode_load(LaserdockSample *samples, uint16_t position, uint16_t count) {
    // not sure when this gets deallocated!
    uint8_t * request =(uint8_t *) calloc(sizeof(uint8_t), 64);

    request[0] = 0xC0;
    request[1] = 0x08;

    uint16_t * pos = (uint16_t *)(request+2);
    uint16_t * cnt = (uint16_t *)(request+4);
    *pos = position;
    *cnt = count;

    memcpy(request+6, samples,  sizeof(LaserdockSample) * count);

    uint32_t rlen = 64;
    uint8_t response[64];
    bool r =  send_raw(usbHandle->getHandle(), request, rlen, response);

    free(request);
    return r;
}


uint16_t float_to_laserdock_xy(float var)
{
    uint16_t val = (4095 * (var + 1.0)/2.0);
    return val;
}


uint16_t laserdock_sample_flip(uint16_t value){
    return 4095 - value;
}

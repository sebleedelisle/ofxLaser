//
// Created by Guoping Huang on 8/8/16.
//

#include "LaserdockDevice.h"
#include "LaserdockDevice_p.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include <libusb.h>

/// ---------------------------- anonymouse namespace ----------------------------

namespace {

    bool guint8(libusb_device_handle *handle, uint8_t command, uint8_t *value){
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

    bool suint8(libusb_device_handle *handle, uint8_t command, uint8_t value){

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

    bool guint32(libusb_device_handle *handle, uint8_t command, uint32_t *value){
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

    bool suint32(libusb_device_handle *handle, uint8_t command, uint32_t value){
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

    bool sendraw(libusb_device_handle *handle, uint8_t* request, uint32_t rlen, uint8_t* response){
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

LaserdockDevice::LaserdockDevice(libusb_device * usbdevice)
    : d(new LaserdockDevicePrivate(usbdevice, this))
{
    d->initialize();
}

LaserdockDevice::~LaserdockDevice() {
}

bool LaserdockDevice::enable_output() {
    return suint8(d->devh_ctl, 0x80, 0x01);
}

LaserdockDevice::Status LaserdockDevice::status() const {
    return d->status;
}


bool LaserdockDevice::usb_send(unsigned char * data, int length){
    //printf("sending usb, numbytes %d.\n", numbytes);

    int r, actual;
    r = libusb_bulk_transfer(d->devh_ctl, (1 | LIBUSB_ENDPOINT_OUT), data, length, &actual, 0);

    if(r != 0 || length != actual)
        return false;

    return true;
}


unsigned char *LaserdockDevice::usb_get(unsigned char * data, int length){

    int r, actual;

    r = libusb_bulk_transfer(d->devh_ctl, (1 | LIBUSB_ENDPOINT_OUT), data, length, &actual, 0);
    if(r != 0 || actual != length)
        return NULL;

    unsigned char * response = (unsigned char *)calloc(64, 1);

    r = libusb_bulk_transfer(d->devh_ctl, (1 | LIBUSB_ENDPOINT_IN), response, 64, &actual, 0);

    if(r != 0 || actual != 64 || response[1] != 0)
    {
        printf("Read Error: %d, %d\n",  r, actual);
        //free(response);
        return NULL;
    }

    return response;
}

void LaserdockDevice::print() const
{
    d->print();
}


bool LaserdockDevice::get_output(bool *enabled) {
    uint8_t enabled8;
    bool success =  guint8(d->devh_ctl, 0x81, &enabled8);
    *enabled = (enabled8 == 1)? true : false;
    return success;
}

bool LaserdockDevice::disable_output() {
    return suint8(d->devh_ctl, 0x80, 0x00);
}

bool LaserdockDevice::dac_rate(uint32_t *rate) {
    return guint32(d->devh_ctl, 0X83, rate);
}

bool LaserdockDevice::set_dac_rate(uint32_t rate) {
    return suint32(d->devh_ctl, 0x82, rate);
}

bool LaserdockDevice::max_dac_rate(uint32_t *rate) {
    return guint32(d->devh_ctl, 0X84, rate);
}

bool LaserdockDevice::min_dac_value(uint32_t *value) {
    return guint32(d->devh_ctl, 0x87, value);
}

bool LaserdockDevice::max_dac_value(uint32_t *value) {
    return guint32(d->devh_ctl, 0x88, value);
}


bool LaserdockDevice::sample_element_count(uint32_t *count) {
    return guint32(d->devh_ctl, 0X85, count);
}

bool LaserdockDevice::iso_packet_sample_count(uint32_t *count) {
    return guint32(d->devh_ctl, 0x86, count);
}

bool LaserdockDevice::bulk_packet_sample_count(uint32_t *count) {
    return guint32(d->devh_ctl, 0x8E, count);
}

bool LaserdockDevice::version_major_number(uint32_t *major) {
    return guint32(d->devh_ctl, 0X8B, major);
}

bool LaserdockDevice::version_minor_number(uint32_t *minor) {
    return guint32(d->devh_ctl, 0X8C, minor);
}


bool LaserdockDevice::ringbuffer_sample_count(uint32_t *count) {
    return guint32(d->devh_ctl, 0X89, count);
}

bool LaserdockDevice::ringbuffer_empty_sample_count(uint32_t *count) {
    return guint32(d->devh_ctl, 0X8A, count);
}

bool LaserdockDevice::send(unsigned char *data, uint32_t length) {
    if(d->flipx || d->flipy){
        LaserdockSample * samples = (LaserdockSample * ) data;
        int count = length/sizeof(LaserdockSample);
        for(int i=0; i < count; i++){
            LaserdockSample * sample = samples+i;
            if(d->flipx){
                sample->x = laserdock_sample_flip(sample->x);
            }

            if(d->flipy){
                sample->y = laserdock_sample_flip(sample->y);
            }
        }
    }

    int timeout_strikes = 3;

    int rv = 0; int transferred = 0;
    do {
        rv = libusb_bulk_transfer(d->devh_data, (3 | LIBUSB_ENDPOINT_OUT), (unsigned char*) data, length,
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
    return suint8(d->devh_ctl, 0x8D, 0);
}

bool LaserdockDevice::flixpX() {
    return d->flipx;
}

bool LaserdockDevice::flixpY() {
    return d->flipy;
}

void LaserdockDevice::setFlipX(bool flip) {
    d->flipx = flip;
}

void LaserdockDevice::setFlipY(bool flip) {
    d->flipy = flip;
}

bool LaserdockDevice::runner_mode_enable(bool v) {
    uint8_t request[] = {0xC0, 0x01, v? (uint8_t)0x01: (uint8_t)0x00};
    uint32_t rlen = 4;
    uint8_t response[64];
    bool r =  sendraw(d->devh_ctl, request, rlen, response);
    return r;
}

bool LaserdockDevice::runner_mode_run(bool v) {
    uint8_t request[] = {0xC0, 0x09, v? (uint8_t)0x01: (uint8_t)0x00};
    uint32_t rlen = 4;
    uint8_t response[64];
    bool r =  sendraw(d->devh_ctl, request, rlen, response);
    return r;
}

bool LaserdockDevice::runner_mode_load(LaserdockSample *samples, uint16_t position, uint16_t count) {
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
    bool r =  sendraw(d->devh_ctl, request, rlen, response);

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

/// ---------------------------- LaserdockDevicePrivate ----------------------------

LaserdockDevicePrivate::LaserdockDevicePrivate(libusb_device *device, LaserdockDevice *q_ptr) :
    q(q_ptr) ,
    usbdevice(device),
    devh_ctl(NULL),
    devh_data(NULL),
    status(LaserdockDevice::Status::UNKNOWN),
    flipx(true),
    flipy(false)
{
}



LaserdockDevicePrivate::~LaserdockDevicePrivate(){
    this->release();
    // TODO: add device close for android with UsbDevice
    libusb_close(this->devh_ctl);
    libusb_close(this->devh_data);
}


void LaserdockDevicePrivate::release(){
    //        int r = 0;
    //        r = libusb_release_interface(this->devh_ctl, 0);
    //        if (r != 0)
    //            fprintf(stderr, "Error releasing interface 0\n");
    //        r = libusb_release_interface(this->devh_ctl, 1);
    //        if (r != 0)
    //            fprintf(stderr, "Error releasing interface 1\n");
}

void LaserdockDevicePrivate::print() const
{
    struct libusb_device_descriptor device_descriptor;

    // Get USB device descriptor
    int result = libusb_get_device_descriptor(usbdevice, &device_descriptor);
    if (result < 0) {
        printf("Failed to get device descriptor!");
    }

    //  print our devices
    printf("0x%04x 0x%04x", device_descriptor.idVendor, device_descriptor.idProduct);

    // Print the device manufacturer string
    char manufacturer[256] = " ";
    if (device_descriptor.iManufacturer) {
        libusb_get_string_descriptor_ascii(devh_ctl, device_descriptor.iManufacturer,
                                           (unsigned char *)manufacturer, sizeof(manufacturer));
        printf(" %s\n", manufacturer);
    }
}


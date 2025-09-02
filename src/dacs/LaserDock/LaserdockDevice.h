//
// Created by Guoping Huang on 8/8/16.
//
// Simplified and fortified by Seb Lee-Delisle on 24/05/25
//
#pragma once

#include <string>
#include <memory>
#include <libusb.h>
#include "UsbDeviceHandleLaserDock.h"

struct LaserdockSample {
    uint16_t rg = 0; // lower byte = red, upper = green
    uint16_t b = 0;  // lower byte = blue
    uint16_t x = 2048;
    uint16_t y = 2048;
};


class LaserdockDevice {
public:
    enum Status { UNKNOWN, INITIALIZED };

    explicit LaserdockDevice(libusb_device* device);
    ~LaserdockDevice();

    Status status() const { return deviceStatus; }
    std::string serial_number() const { return serialNumber; }

    bool enable_output();
    bool disable_output();
    bool get_output(bool* enabled);

    bool dac_rate(uint32_t* rate);
    bool set_dac_rate(uint32_t rate);
    bool max_dac_rate(uint32_t* rate);
    bool min_dac_value(uint32_t* value);
    bool max_dac_value(uint32_t* value);

    bool sample_element_count(uint32_t* count);
    bool iso_packet_sample_count(uint32_t* count);
    bool bulk_packet_sample_count(uint32_t* count);

    bool version_major_number(uint32_t* major);
    bool version_minor_number(uint32_t* minor);

    bool clear_ringbuffer();
    bool ringbuffer_sample_count(uint32_t* count);
    bool ringbuffer_empty_sample_count(uint32_t* count);

    bool runner_mode_enable(bool enable);
    bool runner_mode_run(bool run);
    bool runner_mode_load(LaserdockSample* samples, uint16_t position, uint16_t count);

    bool send(unsigned char* data, uint32_t length);
    bool send_samples(LaserdockSample* samples, uint32_t count);
    bool usb_send(unsigned char* data, int length);
    unsigned char* usb_get(unsigned char* data, int length);

    void print() const;

private:
    void initialize();

    libusb_device* usbDevice = nullptr;
    std::unique_ptr<UsbDeviceHandleLaserDock> usbHandle;
    std::string serialNumber;
    Status deviceStatus = UNKNOWN;

};

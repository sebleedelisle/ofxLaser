//
// Created by Guoping Huang on 8/8/16.
//

#ifndef LASERDOCKLIB_LASERDOCKDEVICE_H
#define LASERDOCKLIB_LASERDOCKDEVICE_H

#include <memory>

#ifdef _WIN32
#define LASERDOCKLIB_EXPORT __declspec(dllexport)
#else
#define LASERDOCKLIB_EXPORT
#endif

uint16_t LASERDOCKLIB_EXPORT float_to_laserdock_xy(float var);
uint16_t LASERDOCKLIB_EXPORT laserdock_sample_flip(uint16_t);

struct LaserdockSample
{
    uint16_t rg;      //lower byte is red, top byte is green
    uint16_t b;       //lower byte is blue
    uint16_t x;
    uint16_t y;
};

#ifdef ANDROID
class _jobject;
typedef _jobject* jobject;
#endif

class libusb_device;
class LaserdockDevicePrivate;

class LASERDOCKLIB_EXPORT LaserdockDevice {

public:

    enum Status { UNKNOWN, INITIALIZED };

    explicit LaserdockDevice(libusb_device *usbdevice);
#ifdef ANDROID
    explicit LaserdockDevice(libusb_device *usbdevice, jobject obj);
#endif
    virtual ~LaserdockDevice();

    Status status() const;

    bool enable_output();
    bool disable_output();
    bool get_output(bool * enabled);

    bool dac_rate(uint32_t *rate);
    bool set_dac_rate(uint32_t rate);
    bool max_dac_rate(uint32_t *rate);
    bool min_dac_value(uint32_t *value);
    bool max_dac_value(uint32_t *value);

    bool sample_element_count(uint32_t *count);
    bool iso_packet_sample_count(uint32_t *count);
    bool bulk_packet_sample_count(uint32_t *count);

    bool version_major_number(uint32_t *major);
    bool version_minor_number(uint32_t *minor);

    bool clear_ringbuffer();
    bool ringbuffer_sample_count(uint32_t *count);
    bool ringbuffer_empty_sample_count(uint32_t *count);

    bool runner_mode_enable(bool);
    bool runner_mode_run(bool);
    bool runner_mode_load(LaserdockSample *samples, uint16_t position, uint16_t count);

    bool send(unsigned char * data, uint32_t length);
    bool send_samples(LaserdockSample * samples, uint32_t count);

    bool flixpX();
    bool flixpY();

    void setFlipX(bool);
    void setFlipY(bool);

    bool usb_send(unsigned char *data, int length);
    unsigned char *usb_get(unsigned char * data, int length);

    void print() const;

private:
    std::unique_ptr<LaserdockDevicePrivate> d;
};

#endif //LASERDOCKLIB_LASERDOCKDEVICE_H

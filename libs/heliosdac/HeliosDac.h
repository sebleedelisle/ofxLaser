/*
SDK for Helios Laser DAC class, HEADER
By Gitle Mikkelsen
gitlem@gmail.com

Dependencies:
Libusb 1.0 (GNU Lesser General Public License, see libusb.h)

Standard: C++14
git repo: https://github.com/Grix/helios_dac.git

BASIC USAGE:
1.	Call OpenDevices() to open devices, returns number of available devices.
2.	To send a frame to the DAC, first call GetStatus(). If the function returns ready (1), 
	then you can call WriteFrame(). The status should be polled until it returns ready. 
	It can and sometimes will fail to return ready on the first try.
3.  To stop output, use Stop(). To restart output you must send a new frame as described above.
4.	When the DAC is no longer needed, destroy the instance (destructors will free everything and close the connection)

The DAC is double-buffered. When it receives its first frame, it starts outputting it. When a second frame is sent to 
the DAC while the first frame is being played, the second frame is stored in the DACs memory until the first frame 
finishes playback, at which point the second, buffered frame will start playing. If the DAC finished playback of a frame
without having received and buffered a second frame, it will by default loop the first frame until a new frame is
received (but the flag HELIOS_FLAG_SINGLE_MODE will make it stop playback instead).
The GetStatus() function actually checks whether or not the buffer on the DAC is empty or full. If it is full, the DAC
cannot receive a new frame until the currently playing frame finishes, freeing up the buffer.
*/

#pragma once

#include "libusb.h"
#include <cstring>
#include <cstdint>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <chrono>
#include <ofMain.h>

#define HELIOS_SDK_VERSION	6

#define HELIOS_MAX_POINTS	0x1000
#define HELIOS_MAX_RATE		0xFFFF
#define HELIOS_MIN_RATE		7

#define HELIOS_SUCCESS		1	

// Functions return negative values if something went wrong	
// Attempted to perform an action before calling OpenDevices()
#define HELIOS_ERROR_NOT_INITIALIZED	-1
// Attempted to perform an action with an invalid device number
#define HELIOS_ERROR_INVALID_DEVNUM		-2
// WriteFrame() called with null pointer to points
#define HELIOS_ERROR_NULL_POINTS		-3
// WriteFrame() called with a frame containing too many points
#define HELIOS_ERROR_TOO_MANY_POINTS	-4
// WriteFrame() called with pps higher than maximum allowed
#define HELIOS_ERROR_PPS_TOO_HIGH		-5
// WriteFrame() called with pps lower than minimum allowed
#define HELIOS_ERROR_PPS_TOO_LOW		-6

// Errors from the HeliosDacDevice class begin at -1000
// Attempted to perform an operation on a closed DAC device
#define HELIOS_ERROR_DEVICE_CLOSED			-1000
// Attempted to send a new frame with HELIOS_FLAGS_DONT_BLOCK before previous DoFrame() completed
#define HELIOS_ERROR_DEVICE_FRAME_READY		-1001
// Operation failed because SendControl() failed (if operation failed because of libusb_interrupt_transfer failure, the error code will be a libusb error instead)
#define HELIOS_ERROR_DEVICE_SEND_CONTROL	-1002
// Received an unexpected result from a call to SendControl()
#define HELIOS_ERROR_DEVICE_RESULT			-1003
// Attempted to call SendControl() with a null buffer pointer
#define HELIOS_ERROR_DEVICE_NULL_BUFFER		-1004
// Attempted to call SendControl() with a control signal that is too long
#define HELIOS_ERROR_DEVICE_SIGNAL_TOO_LONG	-1005

// Errors from libusb are the libusb error code added to -5000. See libusb.h for libusb error codes.
#define HELIOS_ERROR_LIBUSB_BASE		-5000
	
#define HELIOS_FLAGS_DEFAULT			0
#define HELIOS_FLAGS_START_IMMEDIATELY	(1 << 0)
#define HELIOS_FLAGS_SINGLE_MODE		(1 << 1)
#define HELIOS_FLAGS_DONT_BLOCK			(1 << 2)

//usb properties
#define HELIOS_VID	0x1209
#define HELIOS_PID	0xE500
#define EP_BULK_OUT	0x02
#define EP_BULK_IN	0x81
#define EP_INT_OUT	0x06
#define EP_INT_IN	0x83

#ifdef _DEBUG
#define LIBUSB_LOG_LEVEL LIBUSB_LOG_LEVEL_WARNING
#else
#define LIBUSB_LOG_LEVEL LIBUSB_LOG_LEVEL_NONE
#endif

//point data structure
typedef struct
{
	std::uint16_t x; //12 bit (from 0 to 0xFFF)
	std::uint16_t y; //12 bit (from 0 to 0xFFF)
	std::uint8_t r;	//8 bit	(from 0 to 0xFF)
	std::uint8_t g;	//8 bit (from 0 to 0xFF)
	std::uint8_t b;	//8 bit (from 0 to 0xFF)
	std::uint8_t i;	//8 bit (from 0 to 0xFF)
} HeliosPoint;



class HeliosDacDevice //individual dac, interal use
{
public:

	HeliosDacDevice(libusb_device_handle*);
	~HeliosDacDevice();
	int SendFrame(unsigned int pps, std::uint8_t flags, HeliosPoint* points, unsigned int numOfPoints);
	int GetStatus();
	int GetFirmwareVersion();
	int GetName(char* name);
	string GetName();
	int SetName(char* name);
	int SetShutter(bool level);
	int Stop();
	int EraseFirmware();
	bool isClosed() { return closed; };
	virtual void SetClosed();

	string nameStr = "";
    
protected :
    
    bool closed = true;
    
private:

	int DoFrame();
	
	int SendControl(std::uint8_t* buffer, unsigned int bufferSize);

	struct libusb_transfer* interruptTransfer = NULL;
	struct libusb_device_handle* usbHandle;

	bool frameReady = false;
	int firmwareVersion = 0;
	char name[32];

	std::uint8_t* frameBuffer;
	unsigned int frameBufferSize;
	int frameResult = -1;

};


//
//class HeliosDac
//{
//public:
//
//	HeliosDac();
//	~HeliosDac();
//
//	//unless otherwise specified, functions return HELIOS_SUCCESS if OK, and HELIOS_ERROR if something went wrong.
//
//	//initializes drivers, opens connection to all devices.
//	//Returns number of available devices.
//	//NB: To re-scan for newly connected DACs after this function has once been called before, you must first call CloseDevices()
//	int OpenDevices();
//
//	//closes and frees all devices
//	int CloseDevices();
//	
//	//sets debug log level in libusb
//	int SetLibusbDebugLogLevel(int logLevel);
//
//	//writes and outputs a frame to the speficied dac
//	//devNum: dac number (0 to n where n+1 is the return value from OpenDevices() )
//	//pps: rate of output in points per second
//	//flags: (default is 0)
//	//	Bit 0 (LSB) = if 1, start output immediately, instead of waiting for current frame (if there is one) to finish playing
//	//	Bit 1 = if 1, play frame only once, instead of repeating until another frame is written
//	//  Bit 2 = if 1, don't let WriteFrame() block execution while waiting for the transfer to finish 
//	//			(NB: then the function might return 1 even if it fails)
//	//	Bit 3-7 = reserved
//	//points: pointer to point data. See point structure declaration earlier in this document
//	//numOfPoints: number of points in the frame
//	int WriteFrame(unsigned int devNum, unsigned int pps, std::uint8_t flags, HeliosPoint* points, unsigned int numOfPoints);
//
//	//Gets status of DAC, 1 means DAC is ready to receive frame, 0 means it is not
//	int GetStatus(unsigned int devNum);
//
//	//Returns firmware version of DAC
//	int GetFirmwareVersion(unsigned int devNum);
//
//	//Gets name of DAC (populates name with max 32 characters)
//	int GetName(unsigned int devNum, char* name);
//
//	//Sets name of DAC (name must be max 31 characters incl. null terminator)
//	int SetName(unsigned int devNum, char* name);
//
//	//Stops output of DAC until new frame is written (NB: blocks for 100ms)
//	int Stop(unsigned int devNum);
//
//	//Sets shutter level of DAC
//	int SetShutter(unsigned int devNum, bool level);
//
//	//Erase the firmware of the DAC, allowing it to be updated by accessing the SAM-BA bootloader
//	int EraseFirmware(unsigned int devNum);
//
//private:
//
//
//	std::vector<std::unique_ptr<HeliosDacDevice>> deviceList;
//
//	bool inited = false;
//};
//
//

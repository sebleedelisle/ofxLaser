/*
SDK for Helios Laser DAC class, SOURCE
By Gitle Mikkelsen
gitlem@gmail.com

Dependencies:
Libusb 1.0 (GNU Lesser General Public License, see libusb.h)

Standard: C++14
git repo: https://github.com/Grix/helios_dac.git

See header HeliosDac.h for function and usage documentation
 
RIPPED APART BY Seb Lee-Delisle
with apologies to Gitle.
 
*/


#include "HeliosDac.h"


/// -----------------------------------------------------------------------
/// HELIOSDACDEVICE START (one instance for each connected DAC)
/// -----------------------------------------------------------------------

HeliosDacDevice::HeliosDacDevice(libusb_device_handle* handle)
{
	
	ofLogNotice("HeliosDacDevice constructor");
	
	closed = true;
	usbHandle = handle;
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//std::lock_guard<std::mutex>lock(frameLock);

	int actualLength = 0;

	//catch any lingering transfers
	std::uint8_t ctrlBuffer0[32];
	while (libusb_interrupt_transfer(usbHandle, EP_INT_IN, ctrlBuffer0, 32, &actualLength, 5) == LIBUSB_SUCCESS);

	//get firmware version
	firmwareVersion = 0;
	bool repeat = true;
	for (int i = 0; ((i < 2) && repeat); i++) //retry command if necessary
	{
		std::uint8_t ctrlBuffer[2] = { 0x04, 0 };
		int transferResult = libusb_interrupt_transfer(usbHandle, EP_INT_OUT, ctrlBuffer, 2, &actualLength, 32);
		if ((transferResult == LIBUSB_SUCCESS) && (actualLength == 2))
		{
			for (int j = 0; ((j < 3) && repeat); j++) //retry response getting if necessary
			{
				std::uint8_t ctrlBuffer2[32];
				transferResult = libusb_interrupt_transfer(usbHandle, EP_INT_IN, ctrlBuffer2, 32, &actualLength, 32);
				if (transferResult == LIBUSB_SUCCESS)
				{
					if (ctrlBuffer2[0] == 0x84)
					{
						firmwareVersion = ((ctrlBuffer2[1] << 0) |
							(ctrlBuffer2[2] << 8) |
							(ctrlBuffer2[3] << 16) |
							(ctrlBuffer2[4] << 24));
						repeat = false;
					}
				}
			}
		}
	}


	//send sdk firmware version
	repeat = true;
	for (int i = 0; ((i < 2) && repeat); i++) //retry command if necessary
	{
		std::uint8_t ctrlBuffer3[2] = { 0x07, HELIOS_SDK_VERSION };
		int transferResult = libusb_interrupt_transfer(usbHandle, EP_INT_OUT, ctrlBuffer3, 2, &actualLength, 32);
		if ((transferResult == LIBUSB_SUCCESS) && (actualLength == 2))
			repeat = false;
	}

	frameBuffer = new std::uint8_t[HELIOS_MAX_POINTS * 7 + 5];

	closed = false;
	
	nameStr = GetName();
}

HeliosDacDevice::~HeliosDacDevice()
{
	ofLogNotice("HeliosDacDevice destructor");
	SetClosed();
	
	delete frameBuffer;
}

void HeliosDacDevice::SetClosed(){
	if(closed) return;
	libusb_close(usbHandle);
	closed = true;
}

//sends a raw frame buffer (implemented as bulk transfer) to a dac device
//returns 1 if success
int HeliosDacDevice::SendFrame(unsigned int pps, std::uint8_t flags, HeliosPoint* points, unsigned int numOfPoints)
{
	if (closed)
		return HELIOS_ERROR_DEVICE_CLOSED;

	if (frameReady)
		return HELIOS_ERROR_DEVICE_FRAME_READY;

	unsigned int bufPos = 0;
	
	//this is a bug workaround, the mcu won't correctly receive transfers with these sizes
	unsigned int ppsActual = pps;
	unsigned int numOfPointsActual = numOfPoints;
	if ((((int)numOfPoints-45) % 64) == 0)
	{
		//cout << "HelioDacDevice horrific hack engaged! " << endl; 
		numOfPointsActual--;
		//adjust pps to keep the same frame duration even with one less point
		ppsActual = (unsigned int)((pps * (double)numOfPointsActual / (double)numOfPoints) + 0.5); 
	}

	//prepare frame buffer
	for (unsigned int i = 0; i < numOfPointsActual; i++)
	{
		HeliosPoint& p = points[i];
		//cout << p.x << " " << p.y << " " << (int)p.r << " " << (int)p.g << " " << (int)p.b << " " << endl;
		
		frameBuffer[bufPos++] = (points[i].x >> 4);
		frameBuffer[bufPos++] = ((points[i].x & 0x0F) << 4) | (points[i].y >> 8);
		frameBuffer[bufPos++] = (points[i].y & 0xFF);
		frameBuffer[bufPos++] = points[i].r;
		frameBuffer[bufPos++] = points[i].g;
		frameBuffer[bufPos++] = points[i].b;
		frameBuffer[bufPos++] = 0xff;//points[i].i;
	}
	frameBuffer[bufPos++] = (ppsActual & 0xFF);
	frameBuffer[bufPos++] = (ppsActual >> 8);
	frameBuffer[bufPos++] = (numOfPointsActual & 0xFF);
	frameBuffer[bufPos++] = (numOfPointsActual >> 8);
	frameBuffer[bufPos++] = flags;

	frameBufferSize = bufPos;

	if ((flags & HELIOS_FLAGS_DONT_BLOCK) != 0)
	{
		frameReady = true;
		return HELIOS_SUCCESS;
	}
	else
	{
		return DoFrame();
	}
}

//sends frame to DAC
int HeliosDacDevice::DoFrame()
{
	if (closed)
		return HELIOS_ERROR_DEVICE_CLOSED;

	int actualLength = 0;
	int transferResult = libusb_bulk_transfer(usbHandle, EP_BULK_OUT, frameBuffer, frameBufferSize, &actualLength, 8 + (frameBufferSize >> 5));

	if (transferResult == LIBUSB_SUCCESS)
		return HELIOS_SUCCESS;
	else {
		cout << "libusb error : " << transferResult << " while sending " <<frameBufferSize << " samples" << endl;
		return HELIOS_ERROR_LIBUSB_BASE + transferResult;
	}
}

//continually running thread, when a frame is ready, it is sent to the DAC
//only used if HELIOS_FLAGS_DONT_BLOCK is used with writeframe


//Gets firmware version of DAC
int HeliosDacDevice::GetFirmwareVersion()
{
	if (closed)
		return HELIOS_ERROR_DEVICE_CLOSED;

	return firmwareVersion;
}

//Gets name of DAC
int HeliosDacDevice::GetName(char* dacName)
{
	if (closed)
	{
		return HELIOS_ERROR_DEVICE_CLOSED;
	}
	
	int errorCode;

	
	for (int i = 0; (i < 2); i++) //retry command if necessary
	{
		int actualLength = 0;
		std::uint8_t ctrlBuffer4[2] = { 0x05, 0 };
		if (SendControl(ctrlBuffer4, 2) == HELIOS_SUCCESS)
		{
			std::uint8_t ctrlBuffer5[32];
			int transferResult = libusb_interrupt_transfer(usbHandle, EP_INT_IN, ctrlBuffer5, 32, &actualLength, 32);

			if (transferResult == LIBUSB_SUCCESS)
			{
				if (ctrlBuffer5[0] == 0x85)
				{
					ctrlBuffer5[31] = '\0';
					memcpy(name, &ctrlBuffer5[1], 31);
					memcpy(dacName, &ctrlBuffer5[1], 31);
					return HELIOS_SUCCESS;
				}
				else
				{
					errorCode = HELIOS_ERROR_DEVICE_RESULT;
				}
			}
			else
			{
				errorCode = HELIOS_ERROR_LIBUSB_BASE + transferResult;
			}
		}
		else
		{
			errorCode = HELIOS_ERROR_DEVICE_SEND_CONTROL;
		}
	}

	return errorCode;
}

string HeliosDacDevice :: GetName() {
	char dacName[32];
	
	int result = GetName(dacName);
	
	if(result==HELIOS_SUCCESS) {
		string deviceNameStr(dacName);
		return deviceNameStr;
	} else {
		return "";
	}

}


//Gets status of DAC, 1 means DAC is ready to receive frame, 0 means it's not
int HeliosDacDevice::GetStatus()
{
	if (closed)
		return HELIOS_ERROR_DEVICE_CLOSED;

	int errorCode;

	
	int actualLength = 0;
	std::uint8_t ctrlBuffer[2] = { 0x03, 0 };
	if (SendControl(ctrlBuffer, 2) == HELIOS_SUCCESS)
	{
		std::uint8_t ctrlBuffer2[32];
		int transferResult = libusb_interrupt_transfer(usbHandle, EP_INT_IN, ctrlBuffer2, 32, &actualLength, 16);
		if (transferResult == LIBUSB_SUCCESS)
		{
			if (ctrlBuffer2[0] == 0x83) //STATUS ID
			{
				if (ctrlBuffer2[1] == 0)
					return 0;
				else
					return 1;
			}
			else
			{
				errorCode = HELIOS_ERROR_DEVICE_RESULT;
			}
		}
		else
		{
			errorCode = HELIOS_ERROR_LIBUSB_BASE + transferResult;
			if(transferResult == LIBUSB_ERROR_NO_DEVICE) {
				closed = true;
			}
		}
	}
	else
	{
		errorCode = HELIOS_ERROR_DEVICE_SEND_CONTROL;
	}

	return errorCode;
}

//Set shutter level of DAC
int HeliosDacDevice::SetShutter(bool level)
{
	if (closed)
		return HELIOS_ERROR_DEVICE_CLOSED;

	
	std::uint8_t txBuffer[2] = { 0x02, level };
	if (SendControl(txBuffer, 2) == HELIOS_SUCCESS)
		return HELIOS_SUCCESS;
	else
		return HELIOS_ERROR_DEVICE_SEND_CONTROL;
}

//Stops output of DAC
int HeliosDacDevice::Stop()
{
	if (closed)
		return HELIOS_ERROR_DEVICE_CLOSED;


	std::uint8_t txBuffer[2] = { 0x01, 0 };
	if (SendControl(txBuffer, 2) == HELIOS_SUCCESS)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		return HELIOS_SUCCESS;
	}
	else
		return HELIOS_ERROR_DEVICE_SEND_CONTROL;
}

//Sets persistent name of DAC
int HeliosDacDevice::SetName(char* name)
{
	if (closed)
		return HELIOS_ERROR_DEVICE_CLOSED;

	
	std::uint8_t txBuffer[32] = { 0x06 };
	memcpy(&txBuffer[1], name, 30);
	txBuffer[31] = '\0';
	if (SendControl(txBuffer, 32) == HELIOS_SUCCESS)
		return HELIOS_SUCCESS;
	else
		return HELIOS_ERROR_DEVICE_SEND_CONTROL;
}

//Erases the firmware of the DAC, allowing it to be updated
int HeliosDacDevice::EraseFirmware()
{
	if (closed)
		return HELIOS_ERROR_DEVICE_CLOSED;

	std::uint8_t txBuffer[2] = { 0xDE, 0 };
	if (SendControl(txBuffer, 2) == HELIOS_SUCCESS)
	{
		closed = true;
		return HELIOS_SUCCESS;
	}
	else
		return HELIOS_ERROR_DEVICE_SEND_CONTROL;
}

//sends a raw control signal (implemented as interrupt transfer) to a dac device
//returns 1 if successful
int HeliosDacDevice::SendControl(std::uint8_t* bufferAddress, unsigned int length)
{
	if (bufferAddress == NULL)
		return HELIOS_ERROR_DEVICE_NULL_BUFFER;

	if (length > 32)
		return HELIOS_ERROR_DEVICE_SIGNAL_TOO_LONG;

	int actualLength = 0;
	int transferResult = libusb_interrupt_transfer(usbHandle, EP_INT_OUT, bufferAddress, length, &actualLength, 16);

	if (transferResult == LIBUSB_SUCCESS) {
		return HELIOS_SUCCESS;
	} else if(transferResult == LIBUSB_ERROR_NO_DEVICE) {
		closed = true;
		return HELIOS_ERROR_DEVICE_CLOSED; 
	} else {
		return HELIOS_ERROR_LIBUSB_BASE + transferResult;
	}
}

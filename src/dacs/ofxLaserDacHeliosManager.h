//
//  ofxLaserDacHeliosManager.h
//  example_HeliosTest
//
//  Created by Seb Lee-Delisle on 29/05/2020.
//

#pragma once
#include "HeliosDac.h"
#include "ofMain.h"
#include "libusb.h"

namespace ofxLaser {
	class DacHeliosManager {
	
		public :
		DacHeliosManager();
		~DacHeliosManager();
		
		void updateUsbDeviceList();
		bool isHeliosDevice(libusb_device_descriptor& devdesc);
		
		HeliosDacDevice* getDacDeviceForName(string name);
		
		bool deleteDevice(HeliosDacDevice* device);
		
//		bool initialised = false;
		
		DacHeliosManager(const DacHeliosManager&) = delete;
		DacHeliosManager& operator=(const DacHeliosManager &) = delete;
		DacHeliosManager(DacHeliosManager &&) = delete;
		DacHeliosManager & operator=(DacHeliosManager &&) = delete;
		
		static DacHeliosManager& getInstance() {
			static DacHeliosManager dacHeliosManager;
			return dacHeliosManager;
		}
		int numDevices = 0;
		//vector<bool> inUse;
		
		//map<string, HeliosDacDevice*> deviceByName;
		vector<HeliosDacDevice*> availableDevices;
		vector<HeliosDacDevice*> usedDevices;
		//map<string, bool> inUse;
	};
	
};

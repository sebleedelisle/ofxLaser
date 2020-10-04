//
//  ofxLaserDacHeliosManager.h
//  example_HeliosTest
//
//  Created by Seb Lee-Delisle on 29/05/2020.
//

#pragma once
#include "HeliosDac.h"
#include "ofMain.h"

namespace ofxLaser {
	class DacHeliosManager {
	
		
		DacHeliosManager();
		~DacHeliosManager();
		
		private :
		
		
		bool initialised = false;
		
		HeliosDac heliosDacDevice;
		
		public :

		// returns DAC id string
        string connectToDevice(string name);

		bool deviceDisconnected(string devId);
		

		bool doesIdExist(string devId);
		int getDeviceNumForId(string devId);

		
        int getStatus(string devId);

        int setShutter(string devId, bool shutterActive);
        
        int writeFrame(string devId, unsigned int pps, std::uint8_t flags, HeliosPoint* points, unsigned int numOfPoints);

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
		
		map<string, int> deviceNumById;
		map<string, bool> inUse;
	};
	
};

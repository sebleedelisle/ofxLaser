//
//  ofxLaserDacHeliosManager.h
//  example_HeliosTest
//
//  Created by Seb Lee-Delisle on 29/05/2020.
//

#pragma once

namespace ofxLaser {
	class DacHeliosManager {
	
		private :
		DacHeliosManager(){};
		
		bool initialised = false;
		
		HeliosDac heliosDacDevice;
		
		public :

		int connectToDevice(string name) {
			if(!initialised) {
				numDevices = heliosDacDevice.OpenDevices();
				inUse.resize(numDevices, false);
				initialised = true;
			}
			
			if(numDevices<=0) {
				return -1;
			}
			
			for(int i = 0; i<numDevices; i++) {
				char devicename[32];
				heliosDacDevice.GetName(i, devicename);
				string deviceNameStr(devicename);
				ofLogNotice(ofToString(i)+ " " + deviceNameStr); 
				if((!inUse[i]) && ((name.empty()) ||(name == deviceNameStr))) {
					inUse[i] = true;
					return i;
				}
				
				
			}
			return -1;
			
		}
		
		int getStatus(int devNum) {
			return heliosDacDevice.GetStatus(devNum);
		}
		int writeFrame(unsigned int devNum, unsigned int pps, std::uint8_t flags, HeliosPoint* points, unsigned int numOfPoints) {
			return heliosDacDevice.WriteFrame(devNum, pps, flags, points, numOfPoints);
		}

		DacHeliosManager(const DacHeliosManager&) = delete;
		DacHeliosManager& operator=(const DacHeliosManager &) = delete;
		DacHeliosManager(DacHeliosManager &&) = delete;
		DacHeliosManager & operator=(DacHeliosManager &&) = delete;
		
		static DacHeliosManager& getInstance() {
			static DacHeliosManager dacHeliosManager;
			return dacHeliosManager;
		}
		int numDevices = 0;
		vector<bool> inUse;
	};
	
};

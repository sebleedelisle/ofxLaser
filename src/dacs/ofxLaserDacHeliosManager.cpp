//
//  ofxLaserDacHeliosManager.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 03/10/2020.
//

#include "ofxLaserDacHeliosManager.h"

using namespace ofxLaser;


DacHeliosManager::DacHeliosManager() {
	
}
DacHeliosManager::~DacHeliosManager() {
	deviceNumById.clear(); 
	
}


string DacHeliosManager::connectToDevice(string name) {
	
	ofLogNotice("DacHeliosManager::connectToDevice " + name);
	
    //if(!initialised) {
        numDevices = heliosDacDevice.OpenDevices();
        //inUse.resize(numDevices, false);
    
		for(int i = 0; i<numDevices; i++) {
			char devicename[32];
			heliosDacDevice.GetName(i, devicename);
			string deviceNameStr(devicename);
			
			//if(lock()) {
				deviceNumById[deviceNameStr] = i;
				inUse[deviceNameStr] = false;
				//unlock();
			//}
		}
	//	initialised = true;
		
	//} else {
		// check if it's changed... 
		
		
	//}
    
    if(numDevices<=0) {
        return "";
    }
	
	// is the device name in the map? If yes and it's available, use it!
	
	if((!name.empty() && (deviceNumById.find(name)!=deviceNumById.end())) ) {
		
		if(inUse[name]) {
			ofLogNotice("Device already in use : "+name);
			return "";
		} else {
			inUse[name] = true;
			return name;
		}
		
	} else if(name.empty()) {
		
		for( auto & useddata : inUse ) {
			bool & isused = useddata.second;
			const string& devname = useddata.first;
			if(!isused) {
				isused = true;
				return devname;
			}
			
		}
		
	} else {
		ofLogNotice("Error - Helios name " + name + " not found. ");
		return "";
		
	}
		
}

bool DacHeliosManager::deviceDisconnected(string devId) {
	if(doesIdExist(devId)){
		//if(lock()) {
			deviceNumById.erase(devId);
			inUse.erase(devId);
			heliosDacDevice.CloseDevices();
		//	unlock();
			return true;
		//}
	}
	return false; 
	
}

int DacHeliosManager::getStatus(string devId) {
	int num = getDeviceNumForId(devId);
	
	if(num>=0) {
		 return heliosDacDevice.GetStatus(num);
	} else {
		return HELIOS_ERROR_INVALID_DEVNUM;
	}
	
}


int DacHeliosManager::setShutter(string devId, bool shutterActive) {
	
   int num = getDeviceNumForId(devId);
   
   if(num>=0) {
		return heliosDacDevice.SetShutter(num, shutterActive);
   } else {
	   return HELIOS_ERROR_INVALID_DEVNUM;
   }
}

int DacHeliosManager::writeFrame(string devId, unsigned int pps, std::uint8_t flags, HeliosPoint* points, unsigned int numOfPoints) {
	int num = getDeviceNumForId(devId);
	
	if(num>=0) {
		 return heliosDacDevice.WriteFrame(num, pps, flags, points, numOfPoints);;
	} else {
		return HELIOS_ERROR_INVALID_DEVNUM;
	}
	
}

bool DacHeliosManager::doesIdExist(string devId){

	if(deviceNumById.find(devId)!=deviceNumById.end()) {
		//unlock();
		return true;
	} else {
		return false;
	}
}

int DacHeliosManager::getDeviceNumForId(string devId) {
	if(deviceNumById.find(devId)!=deviceNumById.end()) {
		int num = deviceNumById[devId];
		//unlock();
		
		return num;
	} else {
		return -1;
	}
	
}


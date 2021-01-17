//
//  ofxLaserDacBase.cpp
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#include "ofxLaserDacHelios.h"

using namespace ofxLaser;

DacHelios:: DacHelios() : heliosManager(DacHeliosManager::getInstance()) {
	pps = 30000;
	newPPS = 30000;
	
	frameMode = true;
	
	connected = false;
    newArmed = false;  // as in the PPS system, this knows
                            // if armed status has changed and sends
                            // signal to DAC
	deviceId = "";
	dacDevice = nullptr;
	
}
DacHelios:: ~DacHelios() {
	ofLogNotice("DacHelios destructor");
	//stopThread();
	close();
	
}

void DacHelios::close() {
	
	if(isThreadRunning()) {
		waitForThread(true); // also stops the thread
		if(lock()) {
			if(dacDevice!=nullptr) {
				dacDevice->SetClosed();
				dacDevice = nullptr;
			}
			unlock();
		}
		delete dacDevice;
	}
}


void DacHelios::setup(string name) {
	
	deviceName = name; // the parameter which shows the name
	
	ofLogNotice("DacHelios::setup "+name);
	
	connectToDevice(deviceName);

	pointBufferDisplay.set("Point Buffer", 0,0,1799);
	displayData.push_back(&deviceName);
	displayData.push_back(&pointBufferDisplay);
	
	startThread();

}

const vector<ofAbstractParameter*>& DacHelios :: getDisplayData() {
	if(lock()) {
	
		//pointBufferDisplay += (response.status.buffer_fullness-pointBufferDisplay)*1;
		
		//int pointssincelastmessage = (float)((ofGetElapsedTimeMicros()-lastMessageTimeMicros)*response.status.point_rate)/1000000.0f;
		
		//pointBufferDisplay = bufferedPoints.size();
																 
		//latencyDisplay += (latencyMicros - latencyDisplay)*0.1;
		//reconnectCount = prepareSendCount;
	
		unlock();
	}
	
	return displayData;
}

bool DacHelios::connectToDevice(string name) {
	
	dacDevice = heliosManager.getDacDeviceForName(name);
	if(dacDevice!=nullptr) {
		deviceId = deviceName = dacDevice->nameStr;
		if(dacDevice->isClosed()) { // not sure why it should be but hey
			ofLogError("HeliosDac device is closed before it's used!"+ name);
			heliosManager.deleteDevice(dacDevice);
			connected = false;
			
			dacDevice = nullptr;
		} else {
			connected = true;
		}
	} else {
		connected = false;
	}

	return connected;
	
}

bool DacHelios:: sendFrame(const vector<Point>& points){
	if(!connected) return false;
	// get frame object

	DacHeliosFrame* frame = new DacHeliosFrame();
	
	// add all points into the frame object
	frameMode = true;
	for(ofxLaser::Point p : points) {
		frame->addPoint(p);
	}
	// add the frame object to the frame channel
	framesChannel.send(frame);
	
	return true;
	
}


bool DacHelios::sendPoints(const vector<Point>& points) {
	return false;
	
	//	if(bufferedPoints.size()>pps*0.5) {
//		return false;
//	}
//	frameMode = false;
//	HeliosPoint p1;
//	if(lock()) {
//		frameMode = false;
//
//		for(size_t i= 0; i<points.size(); i++) {
//			const Point& p2 = points[i];
//			p1.x = ofMap(p2.x,0,800, HELIOS_MIN, HELIOS_MAX);
//			p1.y = ofMap(p2.y,800,0, HELIOS_MIN, HELIOS_MAX); // Y is UP
//			p1.r = roundf(p2.r);
//			p1.g = roundf(p2.g);
//			p1.b = roundf(p2.b);
//			p1.i = 255;
//			addPoint(p1);
//		}
//		unlock();
//	}
//	return true;
};

bool DacHelios::setPointsPerSecond(uint32_t newpps) {
	ofLog(OF_LOG_NOTICE, "setPointsPerSecond " + ofToString(newpps));
	while(!lock());
	newPPS = newpps;
	unlock();
	return true;
	
	
};

void DacHelios :: setActive(bool active){
    newArmed = active;
    
}
void DacHelios :: reset() {
    if(lock()) {
        resetFlag = true;
        unlock();
    }
}


void DacHelios :: threadedFunction(){
	
	//const uint32_t samples_per_packet = 1024;
    const int minBuffer = 500; // TODO make this dependent on pointrate

	//HeliosPoint * samples = (HeliosPoint *)calloc(sizeof(HeliosPoint), samples_per_packet);
	DacHeliosFrame* currentFrame = nullptr;
	DacHeliosFrame* nextFrame = nullptr;
	DacHeliosFrame* newFrame = nullptr;
	
	while(isThreadRunning()) {
	
		if(connected && (newPPS!=pps)) {
			pps = newPPS;
			
		}
        if(connected && (newArmed!=armed)) {
            if((dacDevice!=nullptr) && (dacDevice->SetShutter(newArmed)==HELIOS_SUCCESS)) {
                armed = newArmed;
            }
        }
       
        
		if(connected && isThreadRunning()) {
            
			// is there a new frame?
			// if so, store it
			bool dacReady = false;
			int status = 0;
				
			while(!dacReady)  {
				if(framesChannel.tryReceive(newFrame)) {
					if(nextFrame!=nullptr) {
						delete nextFrame;
					}
					nextFrame = newFrame;
				}
				
				if(dacDevice!=nullptr) status = dacDevice->GetStatus();
				
				dacReady = (status == 1);
				
				if(!dacReady) {
					if(status>1)
						ofLog(OF_LOG_NOTICE, "heliosDac.getStatus : "+ ofToString(status));
					sleep(1);
				}
			
				
			}
			// if we got to here then the dac is ready but we might not have a
			// currentFrame yet...
			if(nextFrame!=nullptr) {
				if(currentFrame!=nullptr) {
					delete currentFrame;
				}
				currentFrame = nextFrame;
				nextFrame = nullptr;
			}
			if(currentFrame!=nullptr) {
				int result =  0;
				if(dacDevice!=nullptr) result = dacDevice->SendFrame(pps, HELIOS_FLAGS_SINGLE_MODE, currentFrame->samples, currentFrame->numSamples);
				//ofLog(OF_LOG_NOTICE, "heliosDac.WriteFrame : "+ ofToString(result));
				if(result <=-5000){ // then we have a USB connection error
					ofLog(OF_LOG_NOTICE, "heliosDac.WriteFrame failed. LIBUSB error : "+ ofToString(result));
					if(result!=-5007) setConnected(false); // time out error
				} else if(result!=HELIOS_SUCCESS) {
					ofLog(OF_LOG_NOTICE, "heliosDac.WriteFrame failed. Error : "+ ofToString(result));
					setConnected(false);
				} else {
					setConnected(true);
				}
			}
		} else  {
            
            // DO RECONNECT
            //resetFlag = false;
            
			// TODO : handle reconnection better
			// try to reconnect!
			
			if(isThreadRunning()) {
				if(lock()) {
					if(connectToDevice(deviceName)) {
						setConnected(true);
					// wait half a second?
						unlock();
					} else {
						unlock();
						for(size_t i = 0; (i<10)&&(isThreadRunning()); i++ ) {
							sleep(50);
						}
					}
				}
				yield();
				
			}
			
		}
	}

	//free(samples);
}



void DacHelios :: setConnected(bool state) {
	if(connected!=state) {
		
		while(!lock()){};
		connected = state;
		if(!connected) {
			//heliosManager.deviceDisconnected(deviceName);
			armed = !newArmed;
			heliosManager.deleteDevice(dacDevice);
			dacDevice = nullptr;
		}
		ofLogNotice("Helios Dac changed connection state : "+ofToString(connected));
		unlock();
		
	}
}

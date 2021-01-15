//
//  ofxLaserDacBase.cpp
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#include "ofxLaserDacHelios.h"

using namespace ofxLaser;


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
//
//	deviceId = name;
//	string newname = heliosManager.connectToDevice(name);
//
//	if(name.empty()) deviceId = newname;
//
//	if(newname=="") setConnected(false);
//	else {
//		setConnected(true);
//		deviceName = deviceId;
//		ofLogNotice("Connected to HeliosDac : "+deviceId);
//	}
	return connected;
	
}

bool DacHelios:: sendFrame(const vector<Point>& points){
	if(!connected) return false;
	
	// TODO we can probably simplify the replaying
	// system as the Helios will automatically replay frames
	
	int maxBufferSize = 1000;
	HeliosPoint& p1 = sendpoint;
	// if we already have too many points in the buffer,
	// then it means that we need to skip this frame
	
	if(isReplaying || (bufferedPoints.size()<maxBufferSize)) {
		
		framePoints.resize(points.size());
		
		if(lock()) {
			frameMode = true;
			for(size_t i= 0; i<points.size(); i++) {
				
				const Point& p2 = points[i];
				p1.x = ofMap(p2.x,0,800, HELIOS_MIN, HELIOS_MAX);
				p1.y = ofMap(p2.y,800,0, HELIOS_MIN, HELIOS_MAX); // Y is UP
				p1.r = roundf(p2.r);
				p1.g = roundf(p2.g);
				p1.b = roundf(p2.b);
				p1.i = 255;
				addPoint(p1);
				
				framePoints[i] = p1;
			}
			isReplaying = false;
			unlock();
		}
		return true;
	} else {
		// we've skipped this frame... TODO - dropped frame count?
		return false;
	}
}

inline bool DacHelios :: addPoint(const HeliosPoint &point ){
	HeliosPoint* p = getHeliosPoint();
	*p = point; // copy assignment hopefully!
	bufferedPoints.push_back(p);
	return true;
}


HeliosPoint*  DacHelios :: getHeliosPoint() {
	HeliosPoint* p;
	if(sparePoints.size()==0) {
		p= new HeliosPoint();
	} else {
		p = sparePoints.back();
		sparePoints.pop_back();
	}
	
	p->x =
	p->y =
	p->r =
	p->g =
	p->b =
	p->i = 0;
	
	return p;
}


bool DacHelios::sendPoints(const vector<Point>& points) {
	if(bufferedPoints.size()>pps*0.5) {
		return false;
	}
	frameMode = false; 
	HeliosPoint p1;
	if(lock()) {
		frameMode = false;

		for(size_t i= 0; i<points.size(); i++) {
			const Point& p2 = points[i];
			p1.x = ofMap(p2.x,0,800, HELIOS_MIN, HELIOS_MAX);
			p1.y = ofMap(p2.y,800,0, HELIOS_MIN, HELIOS_MAX); // Y is UP
			p1.r = roundf(p2.r);
			p1.g = roundf(p2.g);
			p1.b = roundf(p2.b);
			p1.i = 255;
			addPoint(p1);
		}
		unlock();
	}
	return true;
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
	
	const uint32_t samples_per_packet = 1024;

	HeliosPoint * samples = (HeliosPoint *)calloc(sizeof(HeliosPoint), samples_per_packet);
		
	int _index = 0;
	
	while(isThreadRunning()) {
	
		int samplecount = samples_per_packet;

		
		// 1 : transfer the points into a format that we can
		//     send to the DAC
		
		while(!lock()){};

		// if we're out of points, let's replay the frame!
		if(bufferedPoints.size()<samples_per_packet) {
			if(frameMode && replayFrames) {
				for(size_t i= 0; i<framePoints.size(); i++) {
					addPoint(framePoints[i]);
				}
				isReplaying = true;
			} else {
				// Minimum 10 points per frame
				while(bufferedPoints.size()<10) { // add blank points to fill the space
					// clear the point so we don't just project strong beams
					lastpoint.r = lastpoint.g = lastpoint.b = 0;
					addPoint(lastpoint);
				}
			}
		}
		
		HeliosPoint& p = sendpoint;
		for(int i = 0; i<samples_per_packet; i++) {

			if(bufferedPoints.size()>0) {
				p = *bufferedPoints[0];
				sparePoints.push_back(bufferedPoints[0]);
				bufferedPoints.pop_front();
				lastpoint = p;
			} else {
				//p = lastpoint;
				//p.r = p.g = p.b =0;
				samplecount = i;
				break;
			}
			samples[i] = p;
		}
		if(connected && (newPPS!=pps)) {
			pps = newPPS;
			
		}
        if(connected && (newArmed!=armed)) {
            if(dacDevice->SetShutter(newArmed)==HELIOS_SUCCESS) {
                armed = newArmed;
            }
        }
       
		unlock();

        
		if(connected && isThreadRunning()) {
            
            // getStatus is used to determine if the
            // dac is ready for more points or not
			bool dacReady = false;
			int status = 0;
			if(isThreadRunning()) do {
				if(dacDevice!=nullptr) status = dacDevice->GetStatus();
				dacReady = (status != 0); // 0 means buffer full, 1 means ready
				//ofLog(OF_LOG_NOTICE, "heliosDac.getStatus : "+ ofToString(status));
				//yield();
				if(!dacReady) sleep(1);
			} while((!dacReady) && isThreadRunning());
            
//			if(status!=1) {
//				ofLog(OF_LOG_NOTICE, "heliosDac.getStatus : "+ ofToString(status));
//			}
			//if(!isThreadRunning()) break;
			int result =  0;
			if(dacDevice!=nullptr) result = dacDevice->SendFrame(pps, HELIOS_FLAGS_SINGLE_MODE, &samples[0], samplecount);
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

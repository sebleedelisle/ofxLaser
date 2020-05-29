//
//  ofxLaserDacBase.cpp
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#include "ofxLaserDacHelios.h"

using namespace ofxLaser;


DacHelios:: ~DacHelios() {
	stopThread();
	waitForThread(); 
	
}


void DacHelios::setup(string name) {
	
	deviceName = name;
	
	connectToDevice(deviceName);

	pointBufferDisplay.set("Point Buffer", 0,0,1799);
	displayData.push_back(&deviceName);
	displayData.push_back(&pointBufferDisplay);
	
	startThread();

}


bool DacHelios::connectToDevice(string name) {
	// to do - check name
	// note - this is  a bit different from
	// LaserDock as the name is always the same
	// unless you change it
	//
	// TODO store the used devices somehow to make
	// sure we keep track of which are being used.
//	int numDevs = heliosDac.OpenDevices();
//	char devicename[32];
//	if(numDevs>0) {
//		for(int i = 0; i<numDevs; i++) {
//			heliosDac.GetName(i,devicename);
//			ofLogNotice("Helios "+ofToString(i)+ " : " + devicename);
//		}
//		setConnected(true);
//	} else {
//		setConnected(false);
//	}
//
	deviceNumber = heliosManager.connectToDevice(name);
	if(deviceNumber<0) setConnected(false);
	else setConnected(true);
	

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
			for(int i = 0; i<points.size(); i++) {
				
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

		for(int i = 0; i<points.size(); i++) {
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

void DacHelios :: threadedFunction(){
	
	const uint32_t samples_per_packet = 256;

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
				for(int i = 0; i<framePoints.size(); i++) {
					addPoint(framePoints[i]);
				}
				isReplaying = true;
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
			//device->set_dac_rate(pps);
		}
		unlock();

		if(connected) {
			while((heliosManager.getStatus(deviceNumber)!=1) && isThreadRunning()) yield();
			if(!heliosManager.writeFrame(deviceNumber,pps, HELIOS_FLAGS_SINGLE_MODE, &samples[0], samplecount)){
				ofLog(OF_LOG_NOTICE, "heliosDac.WriteFrame failed");
				setConnected(false);
			} else {
				setConnected(true);
			}
		} else {
//			// try to reconnect!
//			if(lock()){
//				if(!connectToDevice(serialNumber)) {
//					unlock();
//					// wait half a second?
//					sleep(500);
//				} else {
//					unlock();
//				}
//			};
			
		}
	}
	
	free(samples);
}



void DacHelios :: setConnected(bool state) {
	if(connected!=state) {
		
		while(!lock()){};
		connected = state;
		unlock();
		
	}
}

//
//  ofxLaserDacBase.cpp
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#include "ofxLaserDacLaserdock.h"

typedef bool (LaserdockDevice::*ReadMethodPtr)(uint32_t *);

void print_uint32(string name, LaserdockDevice *d, ReadMethodPtr method){
	uint32_t count = 0;
	bool successful = (d->*method)(&count);
	
	if(!successful){
		cout << "Failed reading " << name << endl;
		return;
	}
	
	cout << name << ": " << count << endl;
}


using namespace ofxLaser;

DacLaserdock:: ~DacLaserdock() {
	stopThread();
	waitForThread(); 
	
}


void DacLaserdock::setup(string serial) {
	
	serialNumber = serial;
	
	connectToDevice(serial);

	pointBufferDisplay.set("Point Buffer", 0,0,1799);
	displayData.push_back(&serialNumber);
	displayData.push_back(&pointBufferDisplay);
	
	startThread();

}


bool DacLaserdock::connectToDevice(string serial) {
	
	std::vector<std::unique_ptr<LaserdockDevice> > devices = lddmanager.get_laserdock_devices();
	LaserdockDevice* newdevice = nullptr;
	if(devices.size()==0) {
		
	} else if(serial == "") {
		newdevice = devices[0].release();
	} else {
		for(int i = 0; i<devices.size(); i++) {
			if(devices[i]->serial_number() == serial) {
				newdevice = devices[i].release();
				break;
			}
		}
	}
	if(newdevice == nullptr) {
		//ofLogNotice("DacLaserdock : couldn't connect to DAC " + ofToString(serial));
		
		return false;
	}
	
	// NEW DEVICE ACQUIRED!
	if(device!=nullptr) {
		// make sure device is released if we already have one
		delete device;
	}
	
	device = newdevice;
	
	connected = true;
	
	newPPS = pps;
	//pps = 0;
	
//	cout << "Device Status:" << device->status() << endl;
//	print_uint32("Firmware major version", device, &LaserdockDevice::version_major_number);
//	print_uint32("Firmware minor version", device, &LaserdockDevice::version_minor_number);
//	print_uint32("Max Dac Rate", device, &LaserdockDevice::max_dac_rate);
//	print_uint32("Min Dac Value", device, &LaserdockDevice::min_dac_value);
//	print_uint32("Max Dac Value", device, &LaserdockDevice::max_dac_value);
//	device->set_dac_rate(1000);
//	print_uint32("Current Dac Rate", device, &LaserdockDevice::dac_rate);
//	device->set_dac_rate(30000);
//	print_uint32("Current Dac Rate", device, &LaserdockDevice::dac_rate);
//
//	print_uint32("Sample Element Count", device, &LaserdockDevice::sample_element_count);
//	print_uint32("ISO packket sample count", device, &LaserdockDevice::iso_packet_sample_count);
//	print_uint32("Bulky packet sample count", device, &LaserdockDevice::bulk_packet_sample_count);
//	print_uint32("Ringbuffer sample count", device, &LaserdockDevice::ringbuffer_sample_count);
//	print_uint32("Ringbuffer empty sample count", device, &LaserdockDevice::ringbuffer_empty_sample_count);
//
//	cout << "Serial number: " << device->serial_number() << endl;
//
//	cout << "Clearing ringbuffer: " << device->clear_ringbuffer() << endl;
	
	bool enabled = false ;
	
	if(!device->enable_output()){
		cout << "Failed enabling output state" << endl;
	}
	
	if(!device->get_output(&enabled)){
		cout << "Failed reading output state" << endl;
	} else
	{
		cout << "Output Enabled/Disabled: " << enabled << endl;
	}
	
	LaserdockDevice &d = *device;
	d.runner_mode_enable(1);
	d.runner_mode_enable(0);
	d.runner_mode_run(1);
	d.runner_mode_run(0);
	
	LaserdockSample * samples = (LaserdockSample*) calloc(sizeof(LaserdockSample), 7);
	memset(samples, 0xFF, sizeof(LaserdockSample) * 7);
	d.runner_mode_load(samples, 0, 7);

	serialNumber.setName("Serial");
	serialNumber.set(device->serial_number());
	ofLogNotice("DacLaserdock : connecting to : " + ofToString(serialNumber));
	
	device->set_dac_rate(pps);
	
	return true;
}

bool DacLaserdock:: sendFrame(const vector<Point>& points){
	if(!connected) return false;
	
	//	ofLog(OF_LOG_NOTICE, "point create count  : " + ofToString(dac_point::createCount));
	//	ofLog(OF_LOG_NOTICE, "point destroy count : " + ofToString(dac_point::destroyCount));
	
	int maxBufferSize = 1000;
	LaserdockSample& p1 = sendpoint;
	// if we already have too many points in the buffer,
	// then it means that we need to skip this frame
	
	if(isReplaying || (bufferedPoints.size()<maxBufferSize)) {
		
		framePoints.resize(points.size());
		
		if(lock()) {
			frameMode = true;
			for(int i = 0; i<points.size(); i++) {
				
				const Point& p2 = points[i];
				p1.x = ofMap(p2.x,0,800, LASERDOCK_MIN, LASERDOCK_MAX);
				p1.y = ofMap(p2.y,800,0, LASERDOCK_MIN, LASERDOCK_MAX); // Y is UP
				p1.rg = (int)roundf(p2.r) | ((int)roundf(p2.g)<<8);
				p1.b = roundf(p2.b);
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

inline bool DacLaserdock :: addPoint(const LaserdockSample &point ){
	LaserdockSample* p = getLaserdockSample();
	*p = point; // copy assignment hopefully!
	bufferedPoints.push_back(p);
	return true;
}


LaserdockSample*  DacLaserdock :: getLaserdockSample() {
	LaserdockSample* p;
	if(sparePoints.size()==0) {
		p= new LaserdockSample();
	} else {
		p = sparePoints.back();
		sparePoints.pop_back();
	}
	
	p->x =
	p->y =
	p->rg =
	p->b = 0;
	
	return p;
}


bool DacLaserdock::sendPoints(const vector<Point>& points) {
	if(bufferedPoints.size()>pps*0.5) {
		return false;
	}
	frameMode = false; 
	LaserdockSample p1;
	if(lock()) {
		frameMode = false;

		for(int i = 0; i<points.size(); i++) {
			const Point& p2 = points[i];
			p1.x = ofMap(p2.x,0,800, LASERDOCK_MIN, LASERDOCK_MAX);
			p1.y = ofMap(p2.y,800,0, LASERDOCK_MIN, LASERDOCK_MAX); // Y is UP in ilda specs
			p1.rg = (int)roundf(p2.r) | ((int)roundf(p2.g)<<8);
			p1.b = roundf(p2.b);
			addPoint(p1);
		}
		unlock();
	}
	return true;
};

bool DacLaserdock::setPointsPerSecond(uint32_t newpps) {
	ofLog(OF_LOG_NOTICE, "setPointsPerSecond " + ofToString(newpps));
	while(!lock());
	newPPS = newpps;
	unlock();
	return true;
	
	
};

void DacLaserdock :: threadedFunction(){
	
	const uint32_t samples_per_packet = 64;
	const uint32_t circle_steps = 300;
	LaserdockSample * samples = (LaserdockSample *)calloc(sizeof(LaserdockSample), samples_per_packet);
		
	int _index = 0;
	
	while(isThreadRunning()) {
	
		int count = samples_per_packet;

		
		
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
		
		LaserdockSample& p = sendpoint;
		for(int i = 0; i<samples_per_packet; i++) {

			if(bufferedPoints.size()>0) {
				p = *bufferedPoints[0];
				sparePoints.push_back(bufferedPoints[0]);
				bufferedPoints.pop_front();
				lastpoint = p;
			} else {
				p = lastpoint;
				p.rg = p.b =0;
				
			}
			samples[i] = p;
		}
		if(connected && (newPPS!=pps)) {
			pps = newPPS;
			device->set_dac_rate(pps);
		}
		unlock();

		if(connected) {
			if(!device->send_samples(samples,samples_per_packet)){
				ofLog(OF_LOG_NOTICE, "send_samples failed");
				setConnected(false);
			} else {
				setConnected(true);
			}
		} else {
			// try to reconnect!
			if(lock()){
				if(!connectToDevice(serialNumber)) {
					unlock();
					// wait half a second?
					sleep(500);
				} else {
					unlock();
				}
			};
			
		}
	}
	
	free(samples);
}



void DacLaserdock :: setConnected(bool state) {
	if(connected!=state) {
		
		while(!lock()){};
		connected = state;
		unlock();
		
	}
}

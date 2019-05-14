//
//  ofxLaserDacBase.cpp
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#include "ofxLaserDacLaserDock.h"

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

DacLaserDock:: ~DacLaserDock() {
	stopThread();
	waitForThread(); 
	
}


void DacLaserDock::setup() {
	
	device =  lddmanager.get_next_available_device();

	cout << "Device Status:" << device->status() << endl;
	print_uint32("Firmware major version", device, &LaserdockDevice::version_major_number);
	print_uint32("Firmware minor version", device, &LaserdockDevice::version_minor_number);
	print_uint32("Max Dac Rate", device, &LaserdockDevice::max_dac_rate);
	print_uint32("Min Dac Value", device, &LaserdockDevice::min_dac_value);
	print_uint32("Max Dac Value", device, &LaserdockDevice::max_dac_value);
	device->set_dac_rate(1000);
	print_uint32("Current Dac Rate", device, &LaserdockDevice::dac_rate);
	device->set_dac_rate(30000);
	print_uint32("Current Dac Rate", device, &LaserdockDevice::dac_rate);

	print_uint32("Sample Element Count", device, &LaserdockDevice::sample_element_count);
	print_uint32("ISO packket sample count", device, &LaserdockDevice::iso_packet_sample_count);
	print_uint32("Bulky packet sample count", device, &LaserdockDevice::bulk_packet_sample_count);
	print_uint32("Ringbuffer sample count", device, &LaserdockDevice::ringbuffer_sample_count);
	print_uint32("Ringbuffer empty sample count", device, &LaserdockDevice::ringbuffer_empty_sample_count);

	cout << "Clearing ringbuffer: " << device->clear_ringbuffer() << endl;
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
	
//	if(!device->disable_output()){
//		cout << "Failed disabling output state" << endl;
//	}
//
//	if(!device->get_output(&enabled)){
//		cout << "Failed reading output state" << endl;
//	} else
//	{
//		cout << "Output Enabled/Disabled: " << enabled << endl;
//	}

	startThread();

}

bool DacLaserDock:: sendFrame(const vector<Point>& points){
	
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
				p1.y = ofMap(p2.y,800,0, LASERDOCK_MIN, LASERDOCK_MAX); // Y is UP in ilda specs
				p1.rg = (int)roundf(p2.r) | ((int)roundf(p2.g)<<8);
				p1.b = roundf(p2.b);
				addPoint(p1);
				// TODO make frame replay an option
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

inline bool DacLaserDock :: addPoint(const LaserdockSample &point ){
	LaserdockSample* p = getLaserdockSample();
	*p = point; // copy assignment hopefully!
	bufferedPoints.push_back(p);
	return true;
}


LaserdockSample*  DacLaserDock :: getLaserdockSample() {
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


bool DacLaserDock::sendPoints(const vector<Point>& points) {
	return true;
};

bool DacLaserDock::setPointsPerSecond(uint32_t pps) {
	return true;
	
};

void DacLaserDock :: threadedFunction(){
	
	const uint32_t samples_per_packet = 64;
	const uint32_t circle_steps = 300;
	LaserdockSample * samples = (LaserdockSample *)calloc(sizeof(LaserdockSample), samples_per_packet);
	
	
	int _index = 0;
	
	while(isThreadRunning()) {
	
		int count = samples_per_packet;
//		uint16_t x, y, rg, b;
//		rg = 0xFFFF;
//		b = 0xFFFF;
//
//		for(int i = 0; i < count; i++){
//			_index++;
//
//			x = ofMap(cosf(2*PI/circle_steps*_index),-1,1,0,4095);
//			y = ofMap(sinf(2*PI/circle_steps*_index),-1,1,0,4095);
//
//			samples[i].x = x;
//			samples[i].y = y;
//			samples[i].rg = rg;
//			samples[i].b = b;
//		}
		while(!lock()){}
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
		unlock();
		if(!device->send_samples(samples,samples_per_packet)){
			ofLog(OF_LOG_NOTICE, "send_samples failed");
		}
	}
}

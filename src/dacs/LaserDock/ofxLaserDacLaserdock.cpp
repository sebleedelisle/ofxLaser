//
//  ofxLaserDacLaserdock.cpp
//
//  Created by Seb Lee-Delisle on 03/03/2019
//
//

#include "ofxLaserDacLaserdock.h"

typedef bool (LaserdockDevice::*ReadMethodPtr)(uint32_t *);


using namespace ofxLaser;

DacLaserdock:: ~DacLaserdock() {
    // close() stops the thread and deletes the dac device
    close();
}
void DacLaserdock :: close() {
    if(isThreadRunning()) {
        // also stops the thread
        waitForThread(true);
    }
    if(dacDevice!=nullptr) {
        delete dacDevice;
        dacDevice = nullptr;
    }

    
}


bool DacLaserdock::setup(libusb_device* usbdevice){
    // should only ever be called once. If we need to connect to a different usb device
    // we should delete and start over
    
    if (dacDevice!=nullptr) {
        ofLogError("DacLaserdock::setLaserDockUsbDevice() - Laserdock device already set");
        return false;
    }
    
    dacDevice = new LaserdockDevice(usbdevice);
    if(dacDevice->status() != LaserdockDevice::Status::INITIALIZED) {
        ofLogError("DacLaserdock::setLaserDockUsbDevice() - Laserdock device initialisation failed");
        delete dacDevice;
        return false;
        
    }
    // WE ARE GOOD TO GO!!!!
    
    
    
    connected = true;
    
    dacDevice->max_dac_rate(&maxPPS);// returns false if unsuccessful, should probably check that
    
    // should ensure that pps get set
    pps = 0;

    //
//    cout << "Device Status:" << device->status() << endl;
//    print_uint32("Firmware major version", device, &LaserdockDevice::version_major_number);
//    print_uint32("Firmware minor version", device, &LaserdockDevice::version_minor_number);
//    print_uint32("Max Dac Rate", device, &LaserdockDevice::max_dac_rate);
//    print_uint32("Min Dac Value", device, &LaserdockDevice::min_dac_value);
//    print_uint32("Max Dac Value", device, &LaserdockDevice::max_dac_value);
//    device->set_dac_rate(1000);
//    print_uint32("Current Dac Rate", device, &LaserdockDevice::dac_rate);
//    device->set_dac_rate(30000);
//    print_uint32("Current Dac Rate", device, &LaserdockDevice::dac_rate);
//
//    print_uint32("Sample Element Count", device, &LaserdockDevice::sample_element_count);
//    print_uint32("ISO packket sample count", device, &LaserdockDevice::iso_packet_sample_count);
//    print_uint32("Bulky packet sample count", device, &LaserdockDevice::bulk_packet_sample_count);
//    print_uint32("Ringbuffer sample count", device, &LaserdockDevice::ringbuffer_sample_count);
//    print_uint32("Ringbuffer empty sample count", device, &LaserdockDevice::ringbuffer_empty_sample_count);
//
//    cout << "Serial number: " << device->serial_number() << endl;
//
//    cout << "Clearing ringbuffer: " << device->clear_ringbuffer() << endl;
    
    bool enabled = false ;
    
    if(!dacDevice->enable_output()){
        cout << "Failed enabling output state" << endl;
    }
    
    if(!dacDevice->get_output(&enabled)){
        cout << "Failed reading output state" << endl;
    } else
    {
        cout << "Output Enabled/Disabled: " << enabled << endl;
    }
    
    dacDevice->runner_mode_enable(1);
    dacDevice->runner_mode_enable(0);
    dacDevice->runner_mode_run(1);
    dacDevice->runner_mode_run(0);
    
    LaserdockSample * samples = (LaserdockSample*) calloc(sizeof(LaserdockSample), 7);
    memset(samples, 0xFF, sizeof(LaserdockSample) * 7);
    dacDevice->runner_mode_load(samples, 0, 7);

    serialNumber.setName("Serial");
    serialNumber.set(dacDevice->serial_number());
    ofLogNotice("DacLaserdock : connecting to : " + ofToString(serialNumber));
    
    // TODO if failed, then delete device and don't start the thread
    
    startThread();
    return true;
}
//
//bool DacLaserdock:: sendFrame(const vector<Point>& points){
//	if(!connected) return false;
//
//	//	ofLog(OF_LOG_NOTICE, "point create count  : " + ofToString(dac_point::createCount));
//	//	ofLog(OF_LOG_NOTICE, "point destroy count : " + ofToString(dac_point::destroyCount));
//
//	int maxBufferSize = 1000;
//	LaserdockSample& p1 = sendpoint;
//	// if we already have too many points in the buffer,
//	// then it means that we need to skip this frame
//
//	if(isReplaying || (bufferedPoints.size()<maxBufferSize)) {
//
//		framePoints.resize(points.size());
//
//		if(lock()) {
//			frameMode = true;
//			for(size_t i = 0; i<points.size(); i++) {
//
//				const Point& p2 = points[i];
//				p1.x = ofMap(p2.x,0,800, LASERDOCK_MIN, LASERDOCK_MAX);
//				p1.y = ofMap(p2.y,800,0, LASERDOCK_MIN, LASERDOCK_MAX); // Y is UP
//				p1.rg = (int)roundf(p2.r) | ((int)roundf(p2.g)<<8);
//				p1.b = roundf(p2.b);
//				addPoint(p1);
//
//				framePoints[i] = p1;
//			}
//			isReplaying = false;
//			unlock();
//		}
//		return true;
//	} else {
//		// we've skipped this frame... TODO - dropped frame count?
//		return false;
//	}
//}
//
//inline bool DacLaserdock :: addPoint(const LaserdockSample &point ){
//	LaserdockSample* p = getLaserdockSample();
//	*p = point; // copy assignment hopefully!
//	bufferedPoints.push_back(p);
//	return true;
//}

//
//LaserdockSample*  DacLaserdock :: getLaserdockSample() {
//	LaserdockSample* p;
//	if(sparePoints.size()==0) {
//		p= new LaserdockSample();
//	} else {
//		p = sparePoints.back();
//		sparePoints.pop_back();
//	}
//
//	p->x =
//	p->y =
//	p->rg =
//	p->b = 0;
//
//	return p;
//}

//
//bool DacLaserdock::sendPoints(const vector<Point>& points) {
//	if(bufferedPoints.size()>pps*0.5) {
//		return false;
//	}
//	frameMode = false;
//	LaserdockSample p1;
//	if(lock()) {
//		frameMode = false;
//
//		for(size_t i = 0; i<points.size(); i++) {
//			const Point& p2 = points[i];
//			p1.x = ofMap(p2.x,0,800, LASERDOCK_MIN, LASERDOCK_MAX);
//			p1.y = ofMap(p2.y,800,0, LASERDOCK_MIN, LASERDOCK_MAX); // Y is UP in ilda specs
//			p1.rg = (int)roundf(p2.r) | ((int)roundf(p2.g)<<8);
//			p1.b = roundf(p2.b);
//			addPoint(p1);
//		}
//		unlock();
//	}
//	return true;
//};

bool DacLaserdock::setPointsPerSecond(uint32_t newpps) {
	ofLog(OF_LOG_NOTICE, "setPointsPerSecond " + ofToString(newpps));
	while(!lock());
	newPPS = newpps;
	unlock();
	return true;
};


void DacLaserdock :: reset() {
    if(lock()) {
        resetFlag = true;
        unlock();
    }
}


void DacLaserdock :: threadedFunction(){
    
    //const uint32_t samples_per_packet = 64;
    //const uint32_t circle_steps = 300;
    //LaserdockSample * samples = (LaserdockSample *)calloc(sizeof(LaserdockSample), samples_per_packet);
    
    int _index = 0;
    
    while(isThreadRunning()) {
        
        //		int count = samples_per_packet;
        
        //		while(!lock()){};
        //
        //        if(resetFlag) {
        //            // TODO - do something
        //        }
        
        
        int pointBufferMin = MIN(getMaxPointBufferSize(), maxLatencyMS * pps /1000);
        
        if(connected) {
            
            
            
            // if we're out of points, let's replay the frame!
            //		if(bufferedPoints.size()<samples_per_packet) {
            //			if(frameMode && replayFrames) {
            //				for(size_t i = 0; i<framePoints.size(); i++) {
            //					addPoint(framePoints[i]);
            //				}
            //				isReplaying = true;
            //			}
            //		}
            //
            //		LaserdockSample& p = sendpoint;
            //		for(int i = 0; i<samples_per_packet; i++) {
            //
            //			if(bufferedPoints.size()>0) {
            //				p = *bufferedPoints[0];
            //				sparePoints.push_back(bufferedPoints[0]);
            //				bufferedPoints.pop_front();
            //				lastpoint = p;
            //			} else {
            //				p = lastpoint;
            //				p.rg = p.b =0;
            //
            //			}
            //			samples[i] = p;
            //		}
            if(newPPS!=pps) {
                if(lock()) {
                    if (newPPS>maxPPS) {
                        newPPS = pps = maxPPS;
                    }
                    unlock();
                    if(dacDevice->set_dac_rate(newPPS)) {
                        pps = newPPS;
                    } else {
                        // ?? do something?
                    }
                }
            }
            
            
        }
        
        waitUntilReadyToSend(pointBufferMin);
        sendPointsToDac(); 
        
        //
        //        if(!dacDevice->send_samples(samples,samples_per_packet)){
        //            ofLog(OF_LOG_NOTICE, "send_samples failed");
        //            setConnected(false);
        //        } else {
        //            setConnected(true);
        //        }
        
    }
    
    //free(samples);
}

inline bool DacLaserdock::sendPointsToDac() {
    dacCommand.clear();
    
    int minDacBufferSize = calculateBufferSizeByTimeSent();
    int bufferSize =  bufferedPoints.size();
    
    // get min buffer size
    int minBufferSize = maxLatencyMS * pps / 1000;
    
    int pointBufferCapacity = getMaxPointBufferSize();
    
    int minPointsToQueue = MAX(0, minBufferSize - minDacBufferSize - bufferSize);
    int maxPointsToSend = MAX(0, pointBufferCapacity - calculateBufferSizeByTimeAcked() - 256);
    
    int numpointstosend = 0;
    
    if(frameMode) {
        
        updateFrameQueue(minPointsToQueue);

        numpointstosend = MIN(bufferedPoints.size(), maxPointsToSend);
        
        if(numpointstosend==0) {
            if(verbose) ofLogNotice("sendData : no points to send");
            return false;
        }
        //cout << dacBufferFullness << " " << currentDacBufferFullnessMin << " " << numpointstosend << endl;
    } else {
        // for non-frame mode, just send the buffer
       numpointstosend = MIN(bufferedPoints.size(), maxPointsToSend);
    }

     
    LaserdockSample& dacPoint = sendPoint;
    int colourShiftPointCount =  (float)pps/10000.0f*colourShift ;
    for(int i = 0; i<numpointstosend; i++) {
        
        if(bufferedPoints.size()>0) {
            // pop the point off the front
            // TODO figure out how to add extra points in the buffer to accommodate the
            // colour shift
            int pointindex = colourShiftPointCount;
            if(pointindex >= bufferedPoints.size()) pointindex = bufferedPoints.size()-1;
            
            ofxLaser::Point& laserPoint = *bufferedPoints[pointindex];
            ofxLaser::Point& colourPoint = *bufferedPoints[0];
            
            dacPoint.x = ofMap(laserPoint.x,0,800,LASERDOCK_MIN, LASERDOCK_MAX);
            dacPoint.y = ofMap(laserPoint.y,800,0,LASERDOCK_MIN, LASERDOCK_MAX); // Y is UP
            dacPoint.rg = (int)roundf(colourPoint.r) | ((int)roundf(colourPoint.g)<<8);//colourPoint.r/255.0f*65535;
            //dacPoint.g = colourPoint.g/255.0f*65535;
            dacPoint.b = roundf(colourPoint.b);///255.0f*65535;
           
            PointFactory :: releasePoint(bufferedPoints[0]); // recycling system
            bufferedPoints.pop_front(); // no longer destroys point
            lastPointSent = dacPoint; //
        } else  {
            
            // THIS SHOULD NEVER HAPPEN!!!
            // EXCEPT MAYBE IN STREAM MODE
            // just send some blank points in the same position as the
            // last point
            
            dacPoint = lastPointSent;
            
            dacPoint.rg = 0;
            dacPoint.b = 0;

        }
        
        dacCommand.addPoint(dacPoint);
        
    }
//
//    if(dacCommand.size()>=100000) {
//        ofLog(OF_LOG_ERROR, "ofxLaser::DacEtherDream - too many bytes to send! - " + ofToString(dacCommand.size()));
//    }
    
//
//    // check we sent enough points
//    if(dacCommand.numPointsExpected!=dacCommand.numPoints) {
//        ofLogError("DacEtherDream, incorrect point count sent, expected "+ofToString(dacCommand.numPointsExpected)+", got "+ofToString(dacCommand.numPoints));
//    }
    uint64_t sendTime = ofGetElapsedTimeMicros();

    bool success = dacDevice->send(dacCommand.getBuffer(), dacCommand.size());
     // sendCommand(dacCommand);
    if(success) {
        lastDataSentTime = sendTime;
        lastAckTime = ofGetElapsedTimeMicros();
        lastReportedBufferSize = dacCommand.numPoints;
        stateRecorder.recordStateThreadSafe(lastDataSentTime, 1, lastReportedBufferSize, lastAckTime-lastDataSentTime, dacCommand.numPoints, pps, dacCommand.size());
    }

    return success;
    
    
    
}

void DacLaserdock :: setConnected(bool state) {
	if(connected!=state) {
		
		while(!lock()){};
		connected = state;
		unlock();
		
	}
}

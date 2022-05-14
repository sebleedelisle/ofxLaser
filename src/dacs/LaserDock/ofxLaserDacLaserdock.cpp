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
//   print_uint32("Sample Element Count", device, &LaserdockDevice::sample_element_count);
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
    
    int _index = 0;
    
    while(isThreadRunning()) {
        
        
        int pointBufferMin = MIN(getMaxPointBufferSize(), maxLatencyMS * pps /1000);
        
        if(connected) {
            
            
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
        
        
    }
    
}

inline bool DacLaserdock::sendPointsToDac() {
    
    int minDacBufferSize = calculateBufferSizeByTimeSent();
    int bufferSize =  bufferedPoints.size();
    
    // get min buffer size
    int minBufferSize = maxLatencyMS * pps / 1000;
    
    int pointBufferCapacity = getMaxPointBufferSize();
    
    int minPointsToQueue = MAX(0, minBufferSize - minDacBufferSize - bufferSize);
    int maxPointsToSend = MAX(0, pointBufferCapacity - calculateBufferSizeByTimeAcked());
    
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

    dacCommand.clear();
   
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
            dacPoint.rg = (int)roundf(colourPoint.r) | ((int)roundf(colourPoint.g)<<8);
            
            dacPoint.b = roundf(colourPoint.b);
           
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

    uint64_t sendTime = ofGetElapsedTimeMicros();

    bool success = dacDevice->send(dacCommand.getBuffer(), dacCommand.size());
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

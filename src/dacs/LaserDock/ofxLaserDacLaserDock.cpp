//
//  ofxLaserDacLaserdock.cpp
//
//  Created by Seb Lee-Delisle on 03/03/2019
//
//

#include "ofxLaserDacLaserDock.h"

typedef bool (LaserdockDevice::*ReadMethodPtr)(uint32_t *);


using namespace ofxLaser;

DacLaserDock:: ~DacLaserDock() {
    // close() stops the thread and deletes the dac device
    close();
    cleanUpFramesAndPoints();
}
void DacLaserDock :: close() {
    if(isThreadRunning()) {
        
        if(connected) {
            // send stop message?
            
        }
        
        // also stops the thread
        waitForThread(true, 1000); // 1 second timeout
    }
    if(dacDevice!=nullptr) {
        delete dacDevice;
        dacDevice = nullptr;
    }

    
}


bool DacLaserDock::setup(libusb_device* usbdevice){
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

bool DacLaserDock::setPointsPerSecond(uint32_t newpps) {
	ofLog(OF_LOG_NOTICE, "setPointsPerSecond " + ofToString(newpps));
	while(!lock());
	newPPS = newpps;
	unlock();
	return true;
};


void DacLaserDock :: reset() {
    if(lock()) {
        resetFlag = true;
        unlock();
    }
}


void DacLaserDock :: threadedFunction(){
    
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
        // returns false if it doesn't work
        if(!sendPointsToDac()) {
            if(!connected) {
                
              // try to reconnect?
            }
           // connected = false;
            //ofLogError("laserdock sendpoints error");
        }
        
    }
    
}

inline bool DacLaserDock::sendPointsToDac() {
    
    int minDacBufferSize = calculateBufferFullnessByTimeSent();
    int bufferSize =  bufferedPoints.size();
    
    // get min buffer size
    int minBufferSize = maxLatencyMS * pps / 1000;
    
    int pointBufferCapacity = getMaxPointBufferSize();
    
    int minPointsToQueue = MAX(0, minBufferSize - minDacBufferSize - bufferSize);
    int maxPointsToSend = MAX(0, pointBufferCapacity - calculateBufferFullnessByTimeAcked());
    
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
            
            dacPoint.x = ofMap(armed ? laserPoint.x : 400, 0, 800, LASERDOCK_MIN, LASERDOCK_MAX);
            dacPoint.y = ofMap(armed ? laserPoint.y : 400, 800, 0, LASERDOCK_MIN, LASERDOCK_MAX); // Y is UP
            dacPoint.rg = (int)roundf(armed ? colourPoint.r : 0) | ((int)roundf(armed ? colourPoint.g : 0)<<8);
            
            dacPoint.b = roundf(armed ? colourPoint.b : 0);
           
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
        lastReportedBufferFullness = dacCommand.numPoints;
        stateRecorder.recordStateThreadSafe(lastDataSentTime, 1, lastReportedBufferFullness, lastAckTime-lastDataSentTime, dacCommand.numPoints, pps, dacCommand.size());
    } else {
        connected = false; 
        ofLogError("Laserdock send failed"); 
    }

    return success;
    
    
    
}

void DacLaserDock :: setConnected(bool state) {
	if(connected!=state) {
		
		while(!lock()){};
		connected = state;
		unlock();
		
	}
}



void DacLaserdockByteStream :: clear()  {
    ByteBuffer :: clear();
    numPoints = 0;
}

void DacLaserdockByteStream :: addPoint(LaserdockSample& p) {
   
    appendUInt16(p.rg);
    appendUInt16(p.b);
    appendUInt16(p.x);
    appendUInt16(p.y);
    numPoints++;
}


void DacLaserdockByteStream :: logData() {
    resetReadIndex();
    
    char command = readChar();
    printf( "---------------------------------------------------------------\n");
    cout << "command            :" << command << endl;
   
    if(command != 'd') {
        return;
        
    }
    
    int numpoints = readUInt16(); //bytesToUInt16(&outbuffer[1]);
    printf("num points         : %d\n", numpoints);
    
    for(int i = 0; i<numpoints; i++) {
        
        printf("%03d - " ,i );
        printf(" ctl : %s ", (std::bitset<16>(readUInt16()).to_string().c_str())) ;
        printf(" x   : %06d ",readInt16()) ;
        printf(" y   : %06d ",readInt16()) ;
        printf(" rgb   : %04X%04X%04X " ,readUInt16(),readUInt16(),readUInt16()) ;
        printf(" i   : %d ",readUInt16()) ;
        printf(" u1   : %d ",readUInt16()) ;
        printf(" u2   : %d \n",readUInt16()) ;
       
    }
    printf("size : %zu readIndex : %d", size(), readIndex);
    
        
        
        
    
}

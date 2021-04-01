//
//  ofxLaserDacBase.cpp
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

// Helio Dac is an affordable open source USB laser controller 
// It has a frame send functionality and you can't stream points
// unless you put them in separate frames, which is probably fine. 
// 
// I've had issues using three or more of them on a single Windows 
// machine, even following the official instructions to change the
// USB drivers using Zadig
// 
// I think it's some kind of incompatability with some USB hubs, so 
// if you have issues, try using different hubs, or put the controllers
// on different USB buses. 
// 
// Unlike other DAC implementations in ofxLaser, I'm using the built-in 
// functionality that replays frames until a new one is sent. This seems  
// to help with the performance as I've found that any USB activity can 
// cause short drop-outs otherwise. 


#include "ofxLaserDacHelios.h"

using namespace ofxLaser;

DacHelios:: DacHelios() {//: heliosManager(DacHeliosManager::getInstance()) {
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
		
	}
    
    if(dacDevice!=nullptr) {
        dacDevice->SetClosed();
        delete dacDevice;
        dacDevice = nullptr;
    }

}

bool DacHelios::setup(libusb_device* usbdevice) {

    usbDevice = usbdevice; 
    libusb_device_handle* devHandle;
    int result = libusb_open(usbdevice, &devHandle);
    cout << "...libusb_open result : " << result << endl;
    
    //libusb_get_device()
    
    if (result < 0) {
 
        return false;
    }
    
    result = libusb_claim_interface(devHandle, 0);
    cout << "...libusb_claim_interface : " << result << endl;
    // seems to return LIBUSB_ERROR_ACCESS if it's busy
    if (result < 0) {
        libusb_close(devHandle);
        return false;
    }
    
    result = libusb_set_interface_alt_setting(devHandle, 0, 1);
    cout << "...libusb_set_interface_alt_setting : " << result << endl;
    if (result < 0) {
        libusb_close(devHandle);
        return false;
    }
    
    HeliosDacDevice* dac = new HeliosDacDevice(devHandle);
    cout << "...new dac device " << dac->nameStr << endl;
    if(dac->nameStr.empty()) {
        ofLogError("new dac name is wrong!");
    }

    dacDevice = dac;
    serialNumber = dacDevice->GetName();

	// Not sure if we need this right now - it changes the thread priority. 
	// Could be necessary on Raspberry Pi. 
    //#ifndef _MSC_VER
    //        // only linux and osx
    //        //http://www.yonch.com/tech/82-linux-thread-priority
    //        struct sched_param param;
    //        param.sched_priority = 60;
    //        pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &param );
    //#else
    //        // windows implementation
    //        SetThreadPriority( thread.native_handle(), THREAD_PRIORITY_LOWEST);
    //#endif

    connected = true;
    
    startThread();
    return true;

}


const vector<ofAbstractParameter*>& DacHelios :: getDisplayData() {
	return displayData;
}


bool DacHelios:: sendFrame(const vector<Point>& points){
	if(!connected) return false;
	// make a new frame object or reuse one if it already exists
	DacHeliosFrame* frame = getFrame();
	
    //int minSampleCount = pps * 0.01; // minimum frame length is 1/100 of a second
    
	// add all points into the frame object
	frameMode = true;
   // while(frame->numSamples<minSampleCount) {
        for(const ofxLaser::Point& p : points) {
            frame->addPoint(p);
        }
   
   // }
//    if ((((int)frame->numSamples-45) % 64) == 0) {
//        Point p =points.back();
//        p.setColour(0,0,0);
//        frame->addPoint(p);
//    }
	// add the frame object to the frame channel
	framesChannel.send(frame);
	
	return true;
	
}


bool DacHelios::sendPoints(const vector<Point>& points) {
	
	if(!connected) return false;
	// get frame object

	DacHeliosFrame* frame = getFrame();
	
	// add all points into the frame object
	frameMode = false;
	frame->addPoint(lastPoint);
	for(ofxLaser::Point p : points) {
		frame->addPoint(p);
		lastPoint = p; 
	}
	// add the frame object to the frame channel
	framesChannel.send(frame);
	
	return true;
};

bool DacHelios::setPointsPerSecond(uint32_t newpps) {
	ofLog(OF_LOG_NOTICE, "setPointsPerSecond " + ofToString(newpps));
	while(!lock());
	newPPS = newpps;
	unlock();
	return true;
	
	
};

void DacHelios :: setArmed(bool _armed){
    newArmed = _armed;
    
}
void DacHelios :: reset() {
    if(lock()) {
        resetFlag = true;
        unlock();
    }
}


void DacHelios :: threadedFunction(){
	
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
				
			while((!dacReady) && isThreadRunning())  {
				// if we're in frame mode or we don't have a next frame let's pull another frame off the buffer
				// (means that we only skip frames in frame mode)
				if( (frameMode || nextFrame==nullptr) &&
					 (framesChannel.tryReceive(newFrame)) ) {
					if(nextFrame!=nullptr) {
						// release the nextFrame back into the object pool 
						// (also returns nullptr so clears the pointer as well)
						nextFrame = deleteFrame(nextFrame);
					}
					nextFrame = newFrame;
				}
				yield(); 
				//float time = ofGetElapsedTimef();
				if(dacDevice!=nullptr) status = dacDevice->GetStatus();
				
				yield(); 
				dacReady = (status == 1);
				
				if(!dacReady) {

					if(status<0) {
                        ofLog(OF_LOG_NOTICE, "heliosDac.getStatus error: "+ ofToString(status));// +" " + ofToString(ofGetElapsedTimef()-time));
					}
					yield();
				}
				
				//yield();
			}
			// if we got to here then the dac is ready but we might not have a
			// currentFrame yet...
			if(nextFrame!=nullptr) {
				if(currentFrame!=nullptr) {
					deleteFrame(currentFrame);
				}
				currentFrame = nextFrame;
				nextFrame = nullptr;
			}
			if(currentFrame!=nullptr) {
				int result =  0;
				int attempts = 0; 
				if(dacDevice!=nullptr) {
					while((attempts<10) && (result!=HELIOS_SUCCESS) && isThreadRunning()) {
					
						result = dacDevice->SendFrame(pps, frameMode ? HELIOS_FLAGS_DEFAULT : HELIOS_FLAGS_SINGLE_MODE, currentFrame->samples, currentFrame->numSamples);
						attempts++;
						yield(); 
					}
				}
				// if the frame was sent, delete it!
				if(result == HELIOS_SUCCESS){ 
					currentFrame=deleteFrame(currentFrame);
				}
				
				//ofLog(OF_LOG_NOTICE, "heliosDac.WriteFrame : "+ ofToString(result));
				if(result <=-5000){ // then we have a USB connection error
					ofLog(OF_LOG_NOTICE, "heliosDac.WriteFrame failed. LIBUSB error : "+ ofToString(result));
					//if(result!=-5007) setConnected(false); // time out error
				} else if(result!=HELIOS_SUCCESS) {
					ofLog(OF_LOG_NOTICE, "heliosDac.WriteFrame failed. Error : "+ ofToString(result));
					//setConnected(false);
				} else {
					//setConnected(true);
				}
			} else {
				//ofLogError("DacHelios - run out of points!");
			}
		} else {
            
 
			// TODO : handle reconnection 
			// try to reconnect!
			
			if(isThreadRunning()) {
				if(lock()) {
					if(false) { // connectToDevice(deviceName)) { *****************************************************************
						setConnected(true);
					
						unlock();
					} else {
						unlock();
						// wait five seconds and try again
						for(size_t i = 0; (i<100)&&(isThreadRunning()); i++ ) {
							sleep(50);
						}
					}
				}
				yield();
				
			}
			
		}
	}

}



void DacHelios :: setConnected(bool state) {
	if(connected!=state) {
		
		while(!lock()){};
		connected = state;
		if(!connected) {
			//heliosManager.deviceDisconnected(deviceName);
			armed = !newArmed;
			//heliosManager.deleteDevice(dacDevice);
			dacDevice = nullptr;
		}
		ofLogNotice("Helios Dac changed connection state : "+ofToString(connected));
		unlock();
		
	}
}

// object pooling system
DacHeliosFrame* DacHelios :: getFrame() {
	DacHeliosFrame* returnframe;
	
	if(spareFrames.tryReceive(returnframe)) {
		//ofLogNotice("DacHelios :: getFrame() - removed frame from channel");
		returnframe->reset();
		return returnframe;
	} else {
		//ofLogNotice("DacHelios :: getFrame() - created new frame");
		returnframe =new DacHeliosFrame();
		return returnframe;
	}
		
}
DacHeliosFrame* DacHelios :: deleteFrame(DacHeliosFrame* frame){
	//delete frame;
	//
	spareFrames.send(frame);
	//ofLogNotice("DacHelios :: releaseFrame() - added frame to channel "+ofToString(frame->numSamples));
	return nullptr;
}


//
//  ofxLaserDacHelios.cpp
//
//  Created by Seb Lee-Delisle on 07/05/2020.
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

DacHelios:: DacHelios() {
	pps = 30000;
	newPPS = 30000;
	
	frameMode = true;
	
	connected = false;
    newArmed = false;  // as in the PPS system, this knows
                            // if armed status has changed and sends
                            // signal to DAC
    dacName = "";
	dacDevice = nullptr;
	
}
DacHelios:: ~DacHelios() {
	//ofLogNotice("DacHelios destructor");
    
    // close() stops the thread and deletes the dac device
    close();
	
}

void DacHelios::close() {
	
    // Stop the thread running, if it's still running

	if(isThreadRunning()) {
		waitForThread(true); // also stops the thread
		
	}
    // we own the dacDevice so let's close it down
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
    dacName = dacDevice->GetName();

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
	    
    
	// add all points into the frame object
	frameMode = true;
    for(const ofxLaser::Point& p : points) {
        frame->addPoint(p);
    }

    
    // there's a truly weird bug in Helios were if we
    // have a specific number of points the dac gets unhappy.
    // There's a horrific hack in the HeliosDac code that
    // adds a point and adjusts the point rate to accommodate
    // but I'm very unhappy with that - it will have issues
    // when using the frame sync system.
    
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
	
    // sends a point stream. So far very un-tested for this DAC
    
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
	
        // pps = points per second
		if(connected && (newPPS!=pps)) {
            // we don't have to do anything particularly
            // special as pps is passed through in the
            // SendFrame function
			pps = newPPS;
		}
        
        // if the armed state has changed, let's also
        // change the shutter state!
        if(connected && (newArmed!=armed)) {
            // Do we need to check that dacDevice isn't null?
            // Probably overkill.
           // if((dacDevice!=nullptr) &&
            if(dacDevice->SetShutter(newArmed)==HELIOS_SUCCESS) {
                armed = newArmed;
            }
        }
       
        
		if(isThreadRunning()) {
            
			// is there a new frame?
			// if so, store it
			bool dacReady = false;
			int status = 0;
            
            // keep checking for new frames as long as
            // the dac isn't ready for new frames.
			while((!dacReady) && isThreadRunning())  {
				// if we're in frame mode we want to check for
                // a new frame every time.
                //
                // if we're not in frame mode then we only want
                // to check for a new frame if we've run out of frames.
                //
                // If either of these things are true then let's
                // pull another frame off the buffer.
                //
				// This means that we only skip frames in frame mode,
                // so if we're not, there's a danger that we could
                // build up a huge buffer of frames. This should be checked
                // in sendPoints, although
                //
                // note that it's a while, not an if, so we keep pulling off
                // frames as long as there is a new one - that way we
                // don't get a build up of frames
				while( (frameMode || nextFrame==nullptr ) &&
					 (framesChannel.tryReceive(newFrame)) ) {
                    // we have a new frame, so delete the old one and store it
					if(nextFrame!=nullptr) {
						deleteFrame(nextFrame);
					}
					nextFrame = newFrame;
				}
				yield(); 
				
                // Is the dac ready for a new frame?
				//if(dacDevice!=nullptr)
                status = dacDevice->GetStatus();
				
				yield(); 
				dacReady = (status == 1);
                if(status==1) {
                    dacReady = true;
                    setConnected(true);
                } else {
                    // if we have an actual error...
					if(status<0) {
                        ofLog(OF_LOG_NOTICE, "heliosDac.getStatus error: "+ ofToString(status));// +" " + ofToString(ofGetElapsedTimef()-time));
                        
                        // if the error is -5001 or -1002
                        // then i think it's game over and we have to
                        // concede defeat.
                        setConnected(false);
                    } else {
                        setConnected(true);
                    }
					yield();
				}
				
				//yield();
			}
            
            // We know now that the dac is ready for a new
            // frame.
            
			// if we have a new frame
			if(nextFrame!=nullptr) {
                // clear the existing frame
				if(currentFrame!=nullptr) {
					deleteFrame(currentFrame);
				}
                // and get the next frame
				currentFrame = nextFrame;
				nextFrame = nullptr;
            }
            
            // if we didn't get a new frame, we might
            // still have a currentframe if the last
            // time round it failed after 10 attempts
            
            if(currentFrame!=nullptr) {
			
				int result =  0;
				int attempts = 0;
                
                // let's attempt to send the points
                while((attempts<10) && (result!=HELIOS_SUCCESS) && isThreadRunning()) {
                
                   
                    // if we're in frame mode, send the points
                    // with the default flags - this means that
                    // the Helios will automatically replay the
                    // frame until it gets a new one.
                    //
                    // This is different from other Dacs where
                    // we manage that replay system ourself, but
                    // the Helios seems happiest this way.
                    //
                    // If we're not in frame mode then just send
                    // the frame in single mode.
                    
                    result = dacDevice->SendFrame(pps, frameMode ? HELIOS_FLAGS_DEFAULT : HELIOS_FLAGS_SINGLE_MODE, currentFrame->samples, currentFrame->numSamples);
                    
                    if(result!=HELIOS_SUCCESS) {
                        ofLogNotice("LaserDacHelios thread SendFrame attempt " + ofToString(attempts) + " failed - error " + ofToString(result));
                    }
                    attempts++;
                    yield();
                }
            
				// if the frame was sent successfully, delete it!
				if(result == HELIOS_SUCCESS){ 
					currentFrame=deleteFrame(currentFrame);
                    setConnected(true);
                } else {
                    setConnected(false);
                }
				
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
			//armed = !newArmed;
			//heliosManager.deleteDevice(dacDevice);
			//dacDevice = nullptr;
		}
        unlock();
		ofLogNotice("Helios Dac changed connection state : "+ofToString(connected));
		
		
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


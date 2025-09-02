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
	
	connected = false;
    armedStatus = false;  // as in the PPS system, this knows
    
    dacName = "";
	dacDevice = nullptr;
    // should keep adding frames until max points is reached
    while(blankFrame.addPoint(ofxLaser::Point(ofPoint(400,400),ofColor::black)));
	
}
DacHelios:: ~DacHelios() {
	//ofLogNotice("DacHelios destructor");
    
    // close() stops the thread and deletes the dac device
    close();
	
}

void DacHelios::close() {
    if (isThreadRunning()) {
        waitForThread(true);
    }

    if (dacDevice) {
        dacDevice->SetClosed(); // still needed if it affects internal state
        delete dacDevice;
        dacDevice = nullptr;
    }

    usbHandle.reset(); // releases interfaces and closes the handle via RAII
}

bool DacHelios::setup(std::unique_ptr<UsbDeviceHandleHelios> handle) {
    
    usbHandle = std::move(handle);

    // this is now done in the handler
//    libusb_device_handle* devHandle = usbHandle->getHandle();

//    int result = libusb_set_interface_alt_setting(devHandle, 0, 1);
//    ofLogNotice("DacHelios") << "libusb_set_interface_alt_setting: " << result;
//    if (result < 0) {
//        usbHandle.reset();
//        return false;
//    }

    dacDevice = new HeliosDacSafe(usbHandle.get());
    dacName = dacDevice->GetName();

    if (dacName.empty()) {
        ofLogError("DacHelios") << "DAC device name is empty.";
    }

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
	std::shared_ptr<DacHeliosFrame> frame = getFrame();
	    
    
	// add all points into the frame object
	//frameMode = true;
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


//bool DacHelios::sendPoints(const vector<Point>& points) {
//	
//    // sends a point stream. So far very un-tested for this DAC
//    
//	if(!connected) return false;
//    
//	// get frame object
//
//	DacHeliosFrame* frame = getFrame();
//	
//	// add all points into the frame object
//	frameMode = false;
//	frame->addPoint(lastPoint);
//	for(ofxLaser::Point p : points) {
//		frame->addPoint(p);
//		lastPoint = p; 
//	}
//	// add the frame object to the frame channel
//	framesChannel.send(frame);
//	
//	return true;
//};

bool DacHelios::setPointsPerSecond(uint32_t newpps) {
	ofLog(OF_LOG_NOTICE, "setPointsPerSecond " + ofToString(newpps));
    if(newPPS!=newpps) {
        newPPS = newpps;
        
        return true;
    } else {
        return false;
    }
	
};

uint32_t DacHelios::getPointsPerSecond()  {
    return pps;
}


void DacHelios :: reset() {

    resetFlag = true;

}


void DacHelios :: threadedFunction(){
	
    std::shared_ptr<DacHeliosFrame> currentFrame = nullptr;
    std::shared_ptr<DacHeliosFrame> nextFrame = nullptr;
    std::shared_ptr<DacHeliosFrame> newFrame = nullptr;
	
	while(isThreadRunning()) {
	
        // pps = points per second
		if(connected && (newPPS!=pps)) {
            // we don't have to do anything particularly
            // special as pps is passed through in the
            // SendFrame function
			pps.store(newPPS.load());
		}
        
        // if the armed state has changed, let's also
        // change the shutter state!
        if(connected && (armedStatus!=armed)) {
            // Do we need to check that dacDevice isn't null?
            // Probably overkill.
           // if((dacDevice!=nullptr) &&
            if(dacDevice->SetShutter(armed)==HELIOS_SUCCESS) {
                armedStatus = armed ;
                //ofLogNotice("Armed : ") << armed;
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
				//while( (frameMode || nextFrame==nullptr ) &&
				//	 (framesChannel.tryReceive(newFrame)) ) {
                
                while(framesChannel.tryReceive(newFrame) && isThreadRunning())  {
                    // we have a new frame, so delete the old one and store it
					if(nextFrame!=nullptr) {
						deleteFrame(nextFrame);
					}
					nextFrame = newFrame;
				}
                sleep(1);
				
                // Is the dac ready for a new frame?
				//if(dacDevice!=nullptr)
                status = dacDevice->GetStatus();
				
                sleep(1);
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
                    sleep(1);
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
                    
                    // if we're not armed send the blank samples

                    int samplenum = 0;
                    while((blankPointsAfterReArmRemaining>0) && (samplenum<currentFrame->numSamples)) {
                        HeliosPoint* point = &currentFrame->samples[samplenum];
                        point->r = point->g = point->b = 0;
                        samplenum++;
                        blankPointsAfterReArmRemaining--;
                    }
                    //ofLogNotice("armed : ") << armed;
                    result = dacDevice->SendFrame(pps, HELIOS_FLAGS_DEFAULT, armed ? currentFrame->samples : blankFrame.samples, currentFrame->numSamples);
                   
                    if(result!=HELIOS_SUCCESS) {
                        ofLogNotice("LaserDacHelios thread SendFrame attempt " + ofToString(attempts) + " failed - error " + ofToString(result));
                    }
                    attempts++;
                    sleep(1);
                }
            
				// if the frame was sent successfully, delete it!
				if(result == HELIOS_SUCCESS){ 
					deleteFrame(currentFrame);
                    currentFrame = nullptr;
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
		connected = state;
		ofLogNotice("Helios Dac changed connection state : "+ofToString(connected));
	}
}

// object pooling system
std::shared_ptr<DacHeliosFrame> DacHelios :: getFrame() {
    std::shared_ptr<DacHeliosFrame> returnframe;
    while(spareFramesChannel.tryReceive(returnframe)) {
        spareFrames.push_back(returnframe);
    }
    
    while(spareFrames.size()>10) {
        //std::shared_ptr<DacHeliosFrame>  = spareFrames.back();
        spareFrames.pop_back();
       // delete frame;
    }
    
	if(spareFrames.size()>0) {
		//ofLogNotice("DacHelios :: getFrame() - removed frame spares ") <<spareFrames.size();
        returnframe = spareFrames.back();
        spareFrames.pop_back();
		returnframe->reset();
		return returnframe;
	} else {
		//ofLogNotice("DacHelios :: getFrame() - created new frame");
		returnframe = std::make_shared<DacHeliosFrame>();
		return returnframe;
	}
		
}
void DacHelios :: deleteFrame(std::shared_ptr<DacHeliosFrame> frame){
	//delete frame;
	//
	spareFramesChannel.send(frame);
	//ofLogNotice("DacHelios :: releaseFrame() - added frame to channel "+ofToString(frame->numSamples));
	//return nullptr;
}


 //
//  ofxLaserDacEtherDream.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#include "ofxLaserDacEtherDream.h"

using namespace ofxLaser;

DacEtherDream :: DacEtherDream(){

//	pointBufferDisplay.set("Point Buffer", 0,0,1799);
//	latencyDisplay.set("Latency", 0,0,10000);
//	reconnectCount.set("Reconnect Count", 0, 0,10);
//
//    displayData.push_back(&pointBufferDisplay);
//	displayData.push_back(&latencyDisplay);
//	displayData.push_back(&reconnectCount);

    pointBufferCapacity = 1799; 	// the maximum points in the dac's buffer

    lastCommandSendTime = 0;
    lastAckTime = 0;
    lastDataSentTime = 0;
    
    colourShiftImplemented = true;
}



DacEtherDream :: ~DacEtherDream(){
	
    close();
    
    // NOTE thread must be stopped by now
    frameThreadChannel.close();
    
    DacFrame* frame;
    while(frameThreadChannel.tryReceive(frame)) {
        delete frame;
    }
   
    while(bufferedFrames.size()>0) {
        delete bufferedFrames[0];
        bufferedFrames.pop_front();
    }
   
	for (size_t i= 0; i < bufferedPoints.size(); ++i) {
        PointFactory :: releasePoint(bufferedPoints[i]); // Calls ~object
	}
	bufferedPoints.clear();
    
}



void DacEtherDream :: setup(string _id, string _ip, EtherDreamData& ed) {
	
	pps = 0;
	pps = newPPS = 30000; // this is always sent on begin
	queuedPPSChangeMessages = 0;
	connected = false;
	ipAddress = _ip;
    id = _id;
    etherDreamData = ed;
    lastAckTime = ofGetElapsedTimeMicros();
    
    
    // EtherDream Hardware revision :
    // ED v1 : hardwareRevision 2, softwareRevision 2
    // ED v2 : hardwareRevision 10, softwareRevision 2
    // ED v3 : hardwareRevision 30, softwareRevision 3
    
    // TODO update max point rate from dacdata
    pointBufferCapacity = ed.bufferCapacity;
 
    
    
	Poco::Timespan timeout( 1000000 * 1 / 1); // 1/1 second timeout
	
	try {
		// EtherDreams always talk on port 7765
		Poco::Net::SocketAddress sa(ipAddress, 7765);
		//ofLog(OF_LOG_NOTICE, "TIMEOUT" + ofToString(timeout.totalSeconds()));
        
		socket.connect(sa, timeout);
		socket.setSendTimeout(timeout);
		socket.setReceiveTimeout(timeout);
		
		connected = true;
	} catch (Poco::Exception& exc) {
		//Handle your network errors.
		ofLog(OF_LOG_ERROR,  "DacEtherDream setup failed - Network error: " +ipAddress+" "+ exc.displayText());
		connected = false;

	}catch (Poco::Net::HostNotFoundException& exc) {
		//Handle your network errors.
		ofLog(OF_LOG_ERROR,  "DacEtherDream setup failed - host not found: " + exc.displayText());
		connected = false;
		
	}catch (Poco::TimeoutException& exc) {
		//Handle your network errors.
		ofLog(OF_LOG_ERROR,  "DacEtherDream setup failed - Timeout error: " + exc.displayText());
		connected = false;
		
	}
	catch(...){
		ofLog(OF_LOG_ERROR, "DacEtherDream setup failed - unknown error");
		//std::rethrow_exception(current_exception);
		connected = false;
	}
		

	if(connected) {
		//prepareSent = false;
		beginSent = false;
		startThread(); // blocking is true by default I think?
		

	}
}
bool DacEtherDream :: isReadyForFrame(int maxlatencyms) {
   // return true;
    int queuedPoints = 0;
    if(lock()) {
        
        queuedPoints = getNumPointsInAllBuffers();
        maxLatencyMS = maxlatencyms;
        unlock();
    }
    bool ready = queuedPoints<(maxLatencyMS*newPPS/1000);

    return  ready;
}

int DacEtherDream :: getNumPointsInAllBuffers() {
    // if not in thread then needs lock!
      return getCurrentBufferFullnessMin() + bufferedPoints.size() + getNumPointsInBufferedFrames();
    
}

int DacEtherDream :: getNumPointsInFrames(deque<DacFrame*>& frames) {
    int totalpoints = 0;
    for(DacFrame* frame : frames) {
        totalpoints+=(frame->getNumPoints());
    }
    return totalpoints;
}

int DacEtherDream :: getNumPointsInBufferedFrames() {
    return  getNumPointsInFrames(bufferedFrames);
}

void DacEtherDream :: threadedFunction(){
    
    auto & thread = getNativeThread();
    
#ifndef _MSC_VER
    // only linux and osx
    //http://www.yonch.com/tech/82-linux-thread-priority
    struct sched_param param;
    param.sched_priority = 99; // (highest) sched_get_priority_max(SCHED_FIFO);//89; // - higher is faster
    pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &param );
#else
    // windows implementation
    SetThreadPriority( thread.native_handle(), THREAD_PRIORITY_HIGHEST);
#endif

    // the dac sends a ping response as soon as you connect
    waitForAck('?');
    
    bool needToSendPrepare = true;
    
    // in older ether dreams this doesn't seem to reset even if you disconnect and reconnect
    if(response.status.playback_state == PLAYBACK_PREPARED) resetFlag = true;
      
    
    while(isThreadRunning()) {
        
        if(resetFlag) {
            
            resetFlag = false;
            
            // clear the socket of data
            try {
                int n = socket.receiveBytes(inBuffer, 1000);
            } catch(...) {
                // doesn't matter
            }
            sendStop();
            waitForAck('s');
            if(response.status.light_engine_state == LIGHT_ENGINE_ESTOP) {
                sendClear();
                waitForAck('c');
            }
            prepareSendCount = 0;
          
            while(bufferedPoints.size()>0) {
                PointFactory :: releasePoint(bufferedPoints[0]);
                bufferedPoints.pop_front();
            }
            
            ofLogNotice ("RESET DAC--------------------------------");
           
            
        }
        
        if((response.status.playback_state == PLAYBACK_IDLE) && (response.status.light_engine_state == LIGHT_ENGINE_READY)) {
            needToSendPrepare = true;
            ofLogNotice("PLAYBACK IDLE and LIGHT ENGINE READY");
        }
        
        if(needToSendPrepare) {
            
            bool success = sendPrepare();
            
            if(success) {
                success = waitForAck('p');
            }
            else ofLogNotice("sendPrepare() failed");
            
            
            if( success ) {
                ofLogNotice("waitForAck('p') success");
                needToSendPrepare = false;
                beginSent = false;
                
               
            } else {
                ofLogNotice("waitForAck('p') failed");
                    
            }
        }
        
        // if we're playing and we have a new point rate, send it!
        if(connected && (response.status.playback_state==PLAYBACK_PLAYING) && (newPPS!=pps)) {
            
            
            if(sendPointRate(newPPS)){
                pps = newPPS;
                waitForAck('q');
                // after you send a rate change message you need to
                // include a flag on one of the points that tells
                // the etherdream to actually make the change.
                // We use this counter to keep track of how many
                // rate change messages we've sent and make sure
                // to send a control change flag for each one.
                queuedPPSChangeMessages++;
            }
            
            
        }
        int pointBufferMin = MIN(pointBufferCapacity, maxLatencyMS * pps /1000);
        if(etherDreamData.softwareRevision>=30) {
            pointBufferMin = MAX(pointBufferMin, 256);
        }
        // if state is prepared or playing, and we have points in the buffer, then send the points
        if(connected && (response.status.playback_state!=PLAYBACK_IDLE)) {
            
            // figure out when the next update should be
            // figure out the current buffer
            int bufferFullness = getCurrentBufferFullnessMin();
            
            //int pointBufferMin = maxLatencyMS * pps /1000;
            int pointsUntilEmpty = MAX(0, bufferFullness - pointBufferMin);
            int microsToWait = pointsUntilEmpty * (1000000.0f/pps);
         
            // min buffer amount
            if(microsToWait>0)
                usleep(microsToWait);
            
            //check buffer and send the next points
           // while(!lock()) {}
                bool dataSent = sendPointsToDac();
            //unlock();
            if(dataSent) {
               // cout << "DATA SENT " << endl;
                //if(verbose) logData();
                // note if this fails, the connected flag is disabled which triggers the reset flag below
                waitForAck('d');
                
            } else {
                
                   // cout << "DATA NOT SENT " << endl;
            }
            
        } else {
            // get rid of frames!
            DacFrame* frame;
            while(frameThreadChannel.tryReceive(frame)) {
                //bufferedFrames.push_back(frame);
                //newFrame = true;
                delete frame;
            }
            
        }
       

        // if state is prepared and we have sent enough points and we haven't already, send begin
        if(connected && (response.status.playback_state==PLAYBACK_PREPARED) && (dacBufferFullness >= pointBufferMin)) {
            cout << "Send begin, buffer_fullness : " << dacBufferFullness << " pointBufferMin : " << pointBufferMin << endl;
            sendBegin();
            beginSent = waitForAck('b');
            if(beginSent)  {
                ofLogNotice("waitForAck('b') success");
            }
            
        }
         
        
        if(!connected) {
            if(socket.available()) {
                connected = true;
                resetFlag = true;
            }
            
        }
        
        yield();
    }
}

int DacEtherDream :: getCurrentBufferFullnessMin() {
    
    if(response.status.playback_state == PLAYBACK_IDLE) return dacBufferFullness;
    int elapsedMicros = ofGetElapsedTimeMicros() - lastCommandSendTime;
    // figure out the current buffer
    return MAX(0, dacBufferFullness - (((float)elapsedMicros/1000000.0f) * pps));
   
    
}

int DacEtherDream :: getCurrentBufferFullnessMax() {
   
    if(response.status.playback_state == PLAYBACK_IDLE) return dacBufferFullness;
    int elapsedMicros = ofGetElapsedTimeMicros() - lastAckTime;
    // figure out the current buffer
    return MAX(0, dacBufferFullness - (((float)elapsedMicros/1000000.0f) * pps));
   
    
}
void DacEtherDream :: reset() {
	if(lock()) {
		resetFlag = true;
		unlock();
	}
	
}

void DacEtherDream :: closeWhileRunning() {
	if(!connected) return;
	
	sendStop();
	
    
	waitForThread();
	
	while(!lock());
	socket.close();
	unlock();
	
	
}


bool DacEtherDream:: sendFrame(const vector<Point>& points){

	//ofLog(OF_LOG_NOTICE, "point create count  : " + ofToString(EtherDreamDacPoint::createCount));
	//ofLog(OF_LOG_NOTICE, "point destroy count : " + ofToString(EtherDreamDacPoint::destroyCount));
	//int maxBufferSize = 1000;
	//EtherDreamDacPoint& p1 = sendpoint;
	
    // framePoints is a vector of EtherDreamDacPoint*
    // points is a vector of ofxLaser::Point
    
    stateRecorder.update();
    frameRecorder.update();

    if(lock()) {
        frameMode = true;
        //newFrame = true;
        unlock();
    }
    
    DacFrame* frame = new DacFrame(ofGetElapsedTimeMicros());
    
    //frame->clear();
    
    // iterate through all the points, and add into frame
    for(size_t i= 0; i<points.size(); i++) {
        frame->addPoint(points[i]);
    }
    
    frameThreadChannel.send(frame);
    
    return true;

}


inline bool DacEtherDream :: sendPointsToDac(){
    
    
    // get current buffer
    int currentDacBufferFullnessMin = getCurrentBufferFullnessMin();
    int softwareBufferFullness =  bufferedPoints.size();
    
    // get min buffer size
    int minBufferSize = maxLatencyMS * pps / 1000;
    if(etherDreamData.softwareRevision>=30) {
        minBufferSize = MAX(minBufferSize, 256);
    }
    
    int minPointsToQueue = MAX(0, minBufferSize - currentDacBufferFullnessMin - softwareBufferFullness);
    int maxPointsToSend = MAX(0, pointBufferCapacity - getCurrentBufferFullnessMax() - 256);
    
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

    dacCommand.setDataCommand(numpointstosend);
	
	EtherDreamDacPoint& dacPoint = sendPoint;
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
            
            dacPoint.x = ofMap(laserPoint.x,0,800,ETHERDREAM_MIN, ETHERDREAM_MAX);
            dacPoint.y = ofMap(laserPoint.y,800,0,ETHERDREAM_MIN, ETHERDREAM_MAX); // Y is UP
            dacPoint.r = colourPoint.r/255.0f*65535;
            dacPoint.g = colourPoint.g/255.0f*65535;
            dacPoint.b = colourPoint.b/255.0f*65535;
            dacPoint.i = 0;
            dacPoint.u1 = 0;
            dacPoint.u2 = 0;
         
            // if we haven't started the laser yet, maybe turn the
            // brightness off?
//            if(!beginSent) {
//                p.r = p.g = p.b = 0;
//            }
            

            PointFactory :: releasePoint(bufferedPoints[0]); // recycling system
			bufferedPoints.pop_front(); // no longer destroys point
			lastPointSent = dacPoint; //
		} else  {
            
            // THIS SHOULD NEVER HAPPEN!!!
            // EXCEPT MAYBE IN STREAM MODE
			// just send some blank points in the same position as the
			// last point
			
            dacPoint = lastPointSent;
			
            dacPoint.r = 0;
            dacPoint.g = 0;
            dacPoint.b = 0;

		}
		
		if(queuedPPSChangeMessages>0) {
			// bit 15 is a flag to tell the DAC about a new point rate
            dacPoint.control = 0b1000000000000000;
            ofLogNotice("PPS Change queue "+ofToString(queuedPPSChangeMessages));
			queuedPPSChangeMessages--;
        } else {
            dacPoint.control = 0;
        }
     
        dacCommand.addPoint(dacPoint);
		
	}
	
	if(dacCommand.size()>=100000) {
		ofLog(OF_LOG_ERROR, "ofxLaser::DacEtherDream - too many bytes to send! - " + ofToString(dacCommand.size()));
	}
	
  
	
    // check we sent enough points
    if(dacCommand.numPointsExpected!=dacCommand.numPoints) {
        ofLogError("DacEtherDream, incorrect point count sent, expected "+ofToString(dacCommand.numPointsExpected)+", got "+ofToString(dacCommand.numPoints));
    }
    
	
	bool success =  sendCommand(dacCommand);
    if(success) lastDataSentTime = ofGetElapsedTimeMicros();

    return success;
	
}
// updates the frame buffer with new frames from the threadchannel,
// adds frames to the frame queue until we have minPointsToQueue
// and up to maxPointsToSend
void DacEtherDream :: updateFrameQueue(int minPointsToQueue){
    
    // get all the new frames in the channel
    DacFrame* frame;
    while(frameThreadChannel.tryReceive(frame)) {
        bufferedFrames.push_back(frame);
    }
    deque<DacFrame*> queuedFrames;
   
    int dacBufferFullness = getCurrentBufferFullnessMin();

    // go through the buffered frames and add them into the buffer until we have enough points
    // or we run out of frames
    int skipcount = 0;
    int queuecount = 0;
    int pointsInQueuedFrames =getNumPointsInFrames(queuedFrames);
    while((pointsInQueuedFrames<minPointsToQueue) && (bufferedFrames.size()>0)) {
        // calculate the time that the last point in the queue will be processed
        uint64_t lastPointTimeMicros = ((dacBufferFullness + + bufferedPoints.size() + pointsInQueuedFrames) *1000000 / pps) + ofGetElapsedTimeMicros();
        DacFrame* frame = bufferedFrames[0];
        bufferedFrames.pop_front();
        // if we didn't get to the frame in time and it's more than 10ms late then skip it
        if(frame->frameTime + ((maxLatencyMS)*1000) < lastPointTimeMicros) {
            // skip frame!
            frameRecorder.recordFrameInfoThreadSafe(frame->frameTime, 0, frame->framePoints.size(), 0, true);
            delete frame;
            skipcount++;
        } else {
            queuedFrames.push_back(frame);
            queuecount++;

        }
        pointsInQueuedFrames =getNumPointsInFrames(queuedFrames);
    }
    //cout << "skipped : " << skipcount  << " queued : " << queuecount << endl;
    //cout << "queued frames : " << queuedFrames.size()  << " buffered frames : " << bufferedFrames.size() << endl;
    // if we still don't have enough points then double up!
    // TODO make this better, spread the repeats better
    int i = 0;
    while((i<queuedFrames.size()) && (getNumPointsInFrames(queuedFrames)<minPointsToQueue)) {
        queuedFrames[i]->repeatCount++;
        //cout << "+++ repeating " << i << " " << queuedFrames[i]->repeatCount << endl;
        i++;
        if(i>=queuedFrames.size()) i=0;
    }

    // add all queued frames points to the buffer
    
    for(int i = 0; i<queuedFrames.size(); i++ ) {
        DacFrame& frame = *queuedFrames[i];
        frameRecorder.recordFrameInfoThreadSafe(frame.frameTime, ofGetElapsedTimeMicros() + (( getCurrentBufferFullnessMin() + bufferedPoints.size()) * 1000000 / pps), frame.framePoints.size(), frame.repeatCount, frame.repeatCount == 0);
        
      
        while(frame.repeatCount>0) {
            for(ofxLaser::Point* point : frame.framePoints) {
                addPointToBuffer(*point);
            }
            frame.repeatCount--;
        }
    }
    // now clear the frames!

    while(queuedFrames.size()>0) {
        delete queuedFrames[0];
        queuedFrames.pop_front();
    }

    
    
}

bool DacEtherDream:: sendPoints(const vector<Point>& points){
    
    stateRecorder.update();
 
    if(bufferedPoints.size()>pps*0.5) {
        return false;
    }
	
	if(lock()) {
		frameMode = false;
		for(const Point& p: points) {
			addPointToBuffer(p);
		}
		unlock();
	}
	return true;
  
}



inline bool DacEtherDream::waitForAck(char command) {
	
	// TODO :
	
	// ADD TIMEOUT!
	
	// keep parsing buffer data
	// check validity of response
	// if response not valid then clear buffer and send ping?
	
	// basically this just waits til there's stuff in the socket
	// to read. Bit of code gymnastics because I was getting
	// an error when closing the socket.
    
	bool waiting = true;
	bool failed = false;
    if(verbose) ofLogNotice("waitForAck - " + ofToString(command));
	
    //uint64_t previousLastCommandSendTime = lastCommandSendTime;
    
	int n = 0;
    while ((n==0) && (!failed)) {
		if(!isThreadRunning()) return false;
		// i think this should block until it gets bytes
		
		try {
			n = socket.receiveBytes(inBuffer, 22);
            lastAckTime = ofGetElapsedTimeMicros();
			
		} catch (Poco::Exception& exc) {
			//Handle your network errors.
			ofLog(OF_LOG_ERROR,  "Network error: " + exc.displayText());
			//	isOpen = false;
			failed = true;
		} catch (Poco::TimeoutException& exc) {
			//Handle your network errors.
			ofLog(OF_LOG_ERROR,  "Timeout error: " + exc.displayText());
			//	isOpen = false;
			failed = true;
			
		}
		
		// this should mean that the socket has been closed...
        if((n==0) || (failed)) {
            //yield();
		    //sleep(1);
			//ofLog(OF_LOG_ERROR,  "Socket disconnected");
			failed = true;
			
		}
		//count ++;
	
		//if(count > timeoutwait) {
		//	failed = true;
		//	ofLog(OF_LOG_WARNING, "DACEtherDream.waitForAck timeout)" );
			
		//}
	}
	// = count;
	// TODO - handle incomplete data
	
	//cout << "received " << n << "bytes" <<endl;
    int previousStateBufferFullness = dacBufferFullness;
    
	if(n==22) {
        
        
        int roundTripTimeMicros  = lastAckTime - lastCommandSendTime;
		connected = true;
        response.deserialize(inBuffer);
        dacBufferFullness = response.status._buffer_fullness;
        if(command == 'd') {
            int numbytes = dacCommand.size()+22;
           
            stateRecorder.recordStateThreadSafe(lastDataSentTime, response.status.playback_state, dacBufferFullness, roundTripTimeMicros, dacCommand.numPoints, response.status.point_rate, numbytes);
          
        }

        // conditions to look out for :
        // Command was a data command, and we got a NACK back
        //      Likely to be either an underrun or an overrun. So try to figure out which it is.
        //      If under :
        //          CHECK playback mode - are we in idle? If yes, then set to Prepared
        //          beginSent should be set to false... then it should get set again once the
        //          buffer is back up to minimum. Shouldn't be any points lost I don't think?
        
        //      If over :
        //          Maybe we lost points? Check.
        //          Are we still in play state? If not, then send begin.
        // playback state reverted to IDLE
        
        
		
        if(verbose || (response.response!='a') || (dacBufferFullness > pointBufferCapacity)) {// || (command=='p')|| (command=='?')|| (command=='b')) {
            
           
            
            ofLog(OF_LOG_NOTICE, "response : "+ ofToString(response.response) +  " command : " + ofToString(response.command) );
            if(command == 'd') {
                ofLogNotice("num points sent : " + ofToString(dacCommand.numPoints));
                ofLogNotice("previous buffer_fullness : " + ofToString(previousStateBufferFullness));
                //ofLogNotice("time between ack and send : " + ofToString(lastDataSentTime  - previousLastAckTime));
                ofLogNotice("latest buffer_fullness : "+ofToString(dacBufferFullness));
                ofLogNotice("estimated buffer_fullness : "+ofToString(getCurrentBufferFullnessMin()));

               // dacCommand.logData();
            }
			cout << response.toString() << endl;
            
            // EDGE CASE THAT WE NEED TO CATCH :
            
            // invalid response 'I'
            // command = 'd' (sent data)
            // response.status.playback_state = IDLE
            
            // it means that there's been a bit of a hold up and the DAC
            // has gone into idle mode and is refusing points
            //
            // how to recover?
            // send prepare
            // send the frame again?
            // or just send a load of blank points at the start of the next points?
            
            if(response.response=='I') {

                printf("INVALID COMMAND : %c\n", command);
                //logData();
                
                failed = true;
                
            }
                
		}
		// things we /are/ interested in in this response data :
		//
		// light_engine_state :
		// ====================
		// 0 : ready
		// 1 : warmup
		// 2 : cooldown
		// 3 : Emergency stop
		//
		// I've only ever seen it as 0, I don't think warmup and cooldown are implemented. Emergency stop
		// only happens if you send a 0x00 command or an 0xff command (or any command it doesn't recognise
		//
		// light_engine_flags :
		// ====================
		// 00001 : Emergency stop due to E-Stop packet (or weird command)
		// 00010 : Emergency stop due to E-Stop input to projector (not  sure how etherdream would know?)
		// 00100 : Emergency stop input to projector is currently active (no idea what this means)
		// 01000 : Emergency stop due to over temperature (interesting... probably worth looking into...)
		// 10000 : Emergency stop due to loss of Ethernet (not sure how we'd get the message? unless this is
		//		   sent after a reconnection)
		//
		// playback_state :
		// ================
		// 0 : Idle
		// 1 : Prepared
		// 2 : Playing
		//
		// So zero is the default. In idle, you can't send point data.
		// Prepared means we can start sending points
		// Playing is when we're prepared, have sent data, and started streaming
		//
		// playback_flags :
		// ================
		// Bit # :
		// 001 : Shutter state (1 for open, 0 for closed)
		// 010 : Underflow - the most common, is 1 if the system runs out of points
		// 100 : E-Stop - happens if you send a stop command (or any weird bytes). Worth keeping an eye on
		//
		// buffer_fullness :
		// =================
		// This is how many points are queued up in the buffer. Seems to be a limit of 1799.
		//
		// point_rate :
		// ============
		// whatever the current point rate is set to
		//
		// point_count :
		// =============
		// The number of points it has processed - I wonder what happens when this is clocked?
		// It gets reset on a prepare.
		//
		// things we aren't interested in :
		// ================================
		// protocol - always seems to be zero
		// source - always 0 for data stream. Could be 1 for ilda playback from SD card or 2 for internal abstract generator (no idea what that is but it sounds cool!)
		// source_flags - no idea what this even is. No docs about it.
		
		
	}
	else {
		ofLog(OF_LOG_NOTICE, "Network failure or data received from EtherDream not 22 bytes :" + ofToString(n));
		// what do we do now?
		
	}
	
	if(failed) {
		beginSent = false;
		
		
		return false;
	} else {

		return true;
	}
}


string DacEtherDream :: getId(){
	return "EtherDream "+id;
  
}

int DacEtherDream :: getStatus(){
	if(!connected) return OFXLASER_DACSTATUS_ERROR;
	if(response.status.playback_state <=1) return OFXLASER_DACSTATUS_WARNING;
	else if(response.status.playback_state ==2) return OFXLASER_DACSTATUS_GOOD;
	else return OFXLASER_DACSTATUS_ERROR;
}

inline bool DacEtherDream :: sendBegin(){
	ofLog(OF_LOG_NOTICE, "sendBegin()");
    dacCommand.setBeginCommand(pps);
	beginSent = sendCommand(dacCommand);
	return beginSent;
}
inline bool DacEtherDream :: sendPrepare(){
	ofLog(OF_LOG_NOTICE, "sendPrepare()");
	prepareSendCount++;
    dacCommand.setCommand('p');
    return sendCommand(dacCommand);
}

inline bool DacEtherDream :: sendPointRate(uint32_t rate){
    dacCommand.setPointRateCommand(rate);
	return sendCommand(dacCommand);
}

void DacEtherDream :: logData() {
    dacCommand.logData();
    
}
bool DacEtherDream :: sendPing(){

    dacCommand.setCommand('?');
    return sendCommand(dacCommand);
}
bool DacEtherDream :: sendEStop(){

    dacCommand.setCommand('\0');
    return sendCommand(dacCommand);
}
bool DacEtherDream :: sendStop(){
	// non-emergency stop
    dacCommand.setCommand('s');
    return sendCommand(dacCommand);

}
bool DacEtherDream :: sendClear(){
    dacCommand.setCommand('c');
    return sendCommand(dacCommand);
	
}

bool DacEtherDream :: sendCommand(DacEtherDreamCommand& command) { // sendBytes(const uint8_t* buffer, int length) {
	
    const uint8_t* buffer = command.getBuffer();
    int length = command.size();
    
	int numBytesSent = 0;
	bool failed = false;
	bool networkerror = false;
    lastCommandSendTime = ofGetElapsedTimeMicros();//  count = 0;

	try {
		numBytesSent = socket.sendBytes(buffer, length);
//		if(verbose && (length>1)) {
//			cout << "command sent : " << buffer[0] << " numBytesSent : " << numBytesSent <<  endl;
//			for(int i = 1; i<length; i++) {
//
//				printf("%X2 ", (uint8_t)buffer[i]);
//				if(i%8==0) cout << endl;
//			}
//			cout << endl;
//		}
	}
	catch (Poco::Exception& exc) {
		//Handle your network errors.
		cerr << "sendBytes : Network error: " << exc.displayText() << endl;
		networkerror = true;
		failed = true;
	}
	catch (Poco::TimeoutException& exc) {
		//Handle your network errors.
		cerr << "sendBytes : Timeout error: " << exc.displayText() << endl;
		//	isOpen = false;
		failed = true;
	}
	if(numBytesSent!=length) {
		//do something!
		cerr << "send fail, fewer bytes sent than expected : "<< numBytesSent << endl;
		failed = true;
	} else if (numBytesSent<0) {
		//do something!
		cerr << "send fail, sendBytes returned : "<< numBytesSent << endl;
		failed = true;
	}
	
	if(failed) {
		if(networkerror) {
            connected = false; 
			closeWhileRunning();
			setup(id, ipAddress, etherDreamData);
		}
		beginSent = false;
        
		return false;
	}
	return true;
}



void DacEtherDream :: close() {
    
    if(isThreadRunning()) {
        if(connected) {
            if(lock()) {
                sendStop();
                unlock();
                waitForAck('s');
            }
        }
        // also stops the thread :
        waitForThread();
    }
        
    socket.close();

}

inline bool DacEtherDream :: addPointToBuffer(const ofxLaser::Point &point ){
    ofxLaser::Point* p = PointFactory :: getPoint(point);
    //*p = point; // copy assignment hopefully!
    bufferedPoints.push_back(p);
    return true;
}



int DacEtherDream::getMaxPointBufferSize() {
    int returnvalue = 0;
    if(lock()) {
        returnvalue = pointBufferCapacity;
        unlock();
    }
    return returnvalue;
}

bool DacEtherDream::setPointsPerSecond(uint32_t newpps){
    //ofLog(OF_LOG_NOTICE, "setPointsPerSecond " + ofToString(newpps));
    if(!isThreadRunning()){
        pps = newPPS = newpps;
        return true;
    } else {
        while(!lock());
        newPPS = newpps;
        if (!beginSent) {
            pps = newPPS; // pps rate will get sent with begin anyway
            unlock();
            return true;
        } else {
            unlock();
            return false;
        }
    }
}


bool DacEtherDream::setColourShift(float shift)  {

    if(!isThreadRunning()){
        colourShift =shift ;;
        return true;
    } else {
        if(lock()){
            colourShift = shift ;;
            unlock();
            return true;
        } else {
            return false;
        }
    }
    
}

const vector<ofAbstractParameter*>& DacEtherDream :: getDisplayData() {
//    if(lock()) {
//
//        int pointssincelastmessage = (float)((ofGetElapsedTimeMicros()-lastMessageTimeMicros)*response.status.point_rate)/1000000.0f;
//
//        pointBufferDisplay = response.status.buffer_fullness-pointssincelastmessage;
//
//        latencyDisplay += (roundTripTimeMicros - latencyDisplay)*0.1;
//        reconnectCount = prepareSendCount;
//
//        unlock();
//    }
    
    return displayData;
}


bool DacEtherDream :: setup(string ip) {
    ofLogError("DACs are no longer set up in code! Do it within the app instead");
    throw;
    return false;
}

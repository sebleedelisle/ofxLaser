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
    // the maximum points in the dac's buffer
    // I've set it to the number of points in the
    // original etherdream, but this is passed in when
    // the etherdream is initialised
    dacTotalPointBufferCapacity = 1799;

    lastCommandSendTime = 0;
    lastAckTimeMicros = 0;
    lastDataSentTimeMicros = 0;
    
    colourShiftImplemented = true;
    verbose = false;
    
}



DacEtherDream :: ~DacEtherDream(){
	
    // sends stop command, stops the thread, closes the socket
    close();
    
    cleanUpFramesAndPoints();
    
}


void DacEtherDream :: setup(string _id, string _ip, EtherDreamData& ed) {
	
	pps = 0;
	pps = newPPS = 30000; // this is always sent on begin
	queuedPPSChangeMessages = 0;
	networkConnected = false;
	ipAddress = _ip;
    id = _id;
    etherDreamData = ed; // should copy! 
    lastAckTimeMicros = ofGetElapsedTimeMicros();
    
    // EtherDream Hardware revision :
    // ED v1 : hardwareRevision 2, softwareRevision 2
    // ED v2 : hardwareRevision 10, softwareRevision 2
    // ED v3 : hardwareRevision 30, softwareRevision 3
    // Virtual Etherdream : hardwareRevision 0, softwareRevision portoffset
    versionNumber = 0; //ed.hardwareRevision
    if(ed.hardwareRevision == 2) {
        versionNumber = 1;
    } else if(ed.hardwareRevision == 10) {
        versionNumber = 2;
    } else if(ed.hardwareRevision == 30) {
        versionNumber = 3;
    } else if((ed.hardwareRevision == 40) ||(ed.hardwareRevision == 41)) {
        versionNumber = 4;
    }
    
    // this is a nasty hack to attempt to recognise a mercury, which looks identical to an
    // Ether Dream v1 except that is has a custom buffer.
    if((ed.hardwareRevision == 2) && (ed.softwareRevision ==2) && (ed.bufferCapacity!=1799)) {
        isMercury = true;
    } else {
        isMercury = false;
    } 
    versionString = "v"+ofToString(versionNumber);
    if(versionNumber == 0) {
        versionString = "(virtual)";
    }
    
    int port = 7765;
    if(ed.hardwareRevision == 0) {
        //logNotice("VIRTUAL ETHERDREAM FOUND! ") << ed.hardwareRevision << " " << ed.softwareRevision;
        port += ed.softwareRevision;
    }
    // TODO update max point rate from dacdata
    dacTotalPointBufferCapacity = ed.bufferCapacity;
 
	Poco::Timespan timeout( 1 * 1000000); // 1 second timeout
	
	try {
		// EtherDreams always talk on port 7765
		Poco::Net::SocketAddress sa(ipAddress, port);
		//logNotice"TIMEOUT" + ofToString(timeout.totalSeconds()));
        
		socket.connect(sa, timeout);
		socket.setSendTimeout(timeout);
		socket.setReceiveTimeout(timeout);
		
		networkConnected = true;


	}catch (Poco::TimeoutException& exc) {
		//Handle your network errors.
		ofLog(OF_LOG_ERROR,  "DacEtherDream setup failed - Timeout error: " + exc.displayText());
		networkConnected = false;
    }catch (Poco::Net::HostNotFoundException& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "DacEtherDream setup failed - host not found: " + exc.displayText());
        networkConnected = false;
    } catch (Poco::Exception& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "DacEtherDream setup failed - Network error: " +ipAddress+" "+ exc.displayText());
        networkConnected = false;
	} catch(...){
		ofLog(OF_LOG_ERROR, "DacEtherDream setup failed - unknown error");
		//std::rethrow_exception(current_exception);
		networkConnected = false;
	}
		

	if(networkConnected) {
		//prepareSent = false;
		beginSent = false;
		startThread(); // blocking is true by default I think?
		

	}
}

void DacEtherDream :: threadedFunction(){
    
    auto & thread = getNativeThread();
   
#ifndef _MSC_VER
    // only linux and osx
    //http://www.yonch.com/tech/82-linux-thread-priority
    struct sched_param param;
    param.sched_priority = 90; // (highest) sched_get_priority_max(SCHED_FIFO);//89; // - higher is faster
    pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &param );
#else
    // windows implementation
    SetThreadPriority( thread.native_handle(), THREAD_PRIORITY_HIGHEST);
#endif

    // the dac sends a ping response as soon as you connect
    waitForAck('?');
    
    bool needToSendPrepare = true;
    
    // in older ether dreams this doesn't seem to reset even if you disconnect and reconnect
    if(responseThreaded.status.playback_state == ETHERDREAM_PLAYBACK_PREPARED) resetFlag = true;
      
    
    while(isThreadRunning()) {
       
        if(resetFlag) {
            
            resetFlag = false;
            
            // clear the socket of data
            try {
                //int n =
                socket.receiveBytes(inBuffer, 1000);
            } catch(...) {
                // doesn't matter
            }
            sendStop();
            waitForAck('s');
            if(responseThreaded.status.light_engine_state == LIGHT_ENGINE_ESTOP) {
                sendClear();
                waitForAck('c');
            }
            prepareSendCount = 0;
          
            // could we use a higher level clear ?
            while(bufferedPoints.size()>0) {
                PointFactory :: releasePoint(bufferedPoints[0]);
                bufferedPoints.pop_front();
            }
            numBufferedPoints = 0;
            
            logNotice ("RESET DAC--------------------------------");
           
            
        }
        
        if((responseThreaded.status.playback_state == ETHERDREAM_PLAYBACK_IDLE) && (responseThreaded.status.light_engine_state == LIGHT_ENGINE_READY)) {
            needToSendPrepare = true;
            logNotice("PLAYBACK IDLE and LIGHT ENGINE READY");
        }
        
        if(needToSendPrepare) {
            
            bool success = sendPrepare();
            
            if(success) {
                success = waitForAck('p');
            } else logNotice("sendPrepare() failed");
            
            
            if( success ) {
                logNotice("waitForAck('p') success");
                needToSendPrepare = false;
                blankPointsToSend = numBlankPointsToSendAfterReset;
                beginSent = false;
                
               
            } else {
                logNotice("waitForAck('p') failed");
                    
            }
        }
        
        
        // if we're playing and we have a new point rate, send it!
        if(networkConnected && (responseThreaded.status.playback_state==ETHERDREAM_PLAYBACK_PLAYING) && (newPPS!=pps)) {
            
            if(sendPointRate(newPPS)){
                pps = (uint32_t)newPPS;
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

        // if state is prepared or playing, and we have points in the buffer, then send the points
        if(networkConnected && (responseThreaded.status.playback_state!=ETHERDREAM_PLAYBACK_IDLE)) {
           
            waitUntilReadyToSend();
            
            //check buffer and send the next points
            bool dataSent = sendPointsToDac();
             
            if(dataSent) {
               // cout << "DATA SENT " << endl;
                //if(verbose) logData();
                // note if this fails, the connected flag is disabled which triggers the reset flag below
                // actually this is a lie - it doesn't automatically disable the connected flag, so I'm doing it
                // manually here... TODO - should I do it in the waitForAck function?
                if(!waitForAck('d')) {
                    networkConnected = false;
                }
                
            } else {
                // probably because there are no points in the buffer.
                    //cout << "DATA NOT SENT " << endl;
            }
            
        } else {
            // get rid of frames!
            std::shared_ptr<DacFrame>frame;
            while(frameThreadChannel.tryReceive(frame)) {
                //bufferedFrames.push_back(frame);
                //newFrame = true;
                //delete frame;
            }
           
        }
       

        // if state is prepared and we have sent enough points and we haven't already, send begin
        if(networkConnected && (responseThreaded.status.playback_state==ETHERDREAM_PLAYBACK_PREPARED) && (lastReportedBufferFullness >= getMinimumDacBufferFullnessForLatency())) {
            logNotice( "Send begin, buffer_fullness : " +ofToString(lastReportedBufferFullness) + " pointBufferMin : " + ofToString(getMinimumDacBufferFullnessForLatency()));
            sendBegin();
            beginSent = waitForAck('b');
            if(beginSent)  {
                logNotice("waitForAck('b') success");
            }
            
        }
         
        
        if(!networkConnected) {
            if(socket.available()) {
                networkConnected = true;
                resetFlag = true;
            }
            
        }
        
        yield();
    }
}

int DacEtherDream :: calculateBufferFullnessByTimeSent() {
    
    if(lastReportedPlaybackState != ETHERDREAM_PLAYBACK_PLAYING) return lastReportedBufferFullness;
    
    return DacNetworkBaseThreaded::calculateBufferFullnessByTimeSent();
   
    
}

int DacEtherDream :: calculateBufferFullnessByTimeAcked() {
   
    if(lastReportedPlaybackState != ETHERDREAM_PLAYBACK_PLAYING) return lastReportedBufferFullness;
    return DacNetworkBaseThreaded::calculateBufferFullnessByTimeAcked();

    
}
void DacEtherDream :: reset() {
	//if(lock()) {
		resetFlag = true;
	//	unlock();
	//}
	
}

void DacEtherDream :: closeWhileRunning() {
	if(!networkConnected) return;
	
	sendStop();
	
	waitForThread();
	
	while(!lock());
	socket.close();
	unlock();
	
	
}




bool DacEtherDream :: sendPointsToDac(){

    updateFrameQueue();
 
    int maxEstimatedBufferFullness = calculateBufferFullnessByTimeAcked();
    
    int maxPointsToAdd = MIN(MAX(0, getDacTotalPointBufferCapacity() - maxEstimatedBufferFullness), ETHERDREAM_MAX_POINTS_IN_PACKET);
    int numpointstoadd = MIN(bufferedPoints.size(), maxPointsToAdd);
    
    if(numpointstoadd == 0) return false;
        
    dacCommand.setAsDataCommand(numpointstoadd);
    	
	EtherDreamDacPoint& dacPoint = sendPoint;
    
    int colourShiftPointCount =  (float)pps/10000.0f*colourShift ;
    
	for(int i = 0; i<numpointstoadd; i++) {
		
		if(bufferedPoints.size()>0) {
     
            int pointindex = colourShiftPointCount;
            if(pointindex >= bufferedPoints.size()) pointindex = bufferedPoints.size()-1;
            
            ofxLaser::Point& laserPoint = *bufferedPoints[pointindex];
            ofxLaser::Point& colourPoint = *bufferedPoints[0];
            
            dacPoint.x = ofMap(armed ? laserPoint.x : 400, 0, 800, ETHERDREAM_MIN, ETHERDREAM_MAX);
            dacPoint.y = ofMap(armed ? laserPoint.y : 400, 800, 0, ETHERDREAM_MIN, ETHERDREAM_MAX); // Y is UP
          
            
            if(! armed || blankPointsToSend>0) {
                dacPoint.r =  dacPoint.g = dacPoint.b = 0;
                if(blankPointsToSend>0) blankPointsToSend--;
                
            } else {
                dacPoint.r = colourPoint.r/255.0f*65535;
                dacPoint.g = colourPoint.g/255.0f*65535;
                dacPoint.b = colourPoint.b/255.0f*65535;
            }
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
            numBufferedPoints--;
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
            logNotice("PPS Change queue "+ofToString(queuedPPSChangeMessages));
			queuedPPSChangeMessages--;
        } else {
            dacPoint.control = 0;
        }
     
        dacCommand.addPoint(dacPoint);
		
	}
	
	if(dacCommand.size()>=ETHERDREAM_MAX_PACKET_SIZE) {
		ofLog(OF_LOG_ERROR, "ofxLaser::DacEtherDream - too many bytes to send! - " + ofToString(dacCommand.size()));
	}
	
  
    // check we sent enough points
    if(dacCommand.numPointsExpected!=dacCommand.numPoints) {
        ofLogError("DacEtherDream, incorrect point count sent, expected "+ofToString(dacCommand.numPointsExpected)+", got "+ofToString(dacCommand.numPoints));
    }
    
    //ofLogNotice("Packet size : ") << dacCommand.numPoints;
	bool success =  sendCommand(dacCommand);
    if(success) {
        lastDataSentTimeMicros = ofGetElapsedTimeMicros();
        lastDataSentBufferSize = maxEstimatedBufferFullness + dacCommand.numPoints;
    }  else {
        logNotice("sendCommand failed!");
        
    }
    
    
    return success;
	
}


EtherDreamData DacEtherDream::getEtherDreamData() {
    EtherDreamData ed;
    
    if(isThreadRunning()) {
        if(lock()) {
            ed = etherDreamData;
            unlock();
        }
        
    } else {
        ed = etherDreamData;
    }
    return ed;
}

int DacEtherDream::getLastReportedBufferFullness() {
    return lastReportedBufferFullness.load();
//
//    int result = 0;
//    
//    if(isThreadRunning()) {
//        if(lock()) {
//            result = lastReportedBufferFullness;
//            unlock();
//        }
//        
//    } else {
//        result = lastReportedBufferFullness;
//    }
//    return result;
    
}



bool DacEtherDream::waitForAck(char command) {
	
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
    if(verbose) logNotice("waitForAck - " + ofToString(command));
	
    //uint64_t previousLastCommandSendTime = lastCommandSendTime;
    
	int n = 0;
    
    long starttime = ofGetElapsedTimeMillis();
    
    while ((n==0) && (!failed)) {
        
        yield();
        
		if(!isThreadRunning()) return false;
		// i think this should block until it gets bytes
		
		try {
			n = socket.receiveBytes(inBuffer, 22);
            lastAckTimeMicros = ofGetElapsedTimeMicros();
			
	
		} catch (Poco::TimeoutException& exc) {
			//Handle your network errors.
			ofLog(OF_LOG_ERROR,  "Timeout error: " + exc.displayText());
            networkErrors.send(exc.displayText()); 
			//	isOpen = false;
			failed = true;
			
        } catch (Poco::Exception& exc) {
            //Handle your network errors.
            ofLog(OF_LOG_ERROR,  "Network error: " + exc.displayText());
            networkErrors.send(exc.displayText());
            //    isOpen = false;
            failed = true;
        }
		
		// this should mean that the socket has been closed...
        if((n==0) || (failed)) {
            //yield();
		    //sleep(1);
			//ofLog(OF_LOG_ERROR,  "Socket disconnected");
			failed = true;
			
		}
        
        if( ofGetElapsedTimeMillis() -starttime >5000) {
            //TIMEOUT!
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
    int previousStateBufferFullness = lastReportedBufferFullness;
    
	if(n==22) {
        lastResponseTimeMS = ofGetElapsedTimeMillis(); 
        
        int roundTripTimeMicros  = lastAckTimeMicros - lastCommandSendTime;
		networkConnected = true;
        responseThreaded.deserialize(inBuffer);
        lastReportedBufferFullness = responseThreaded.status._buffer_fullness;
        lastReportedPlaybackState = responseThreaded.status.playback_state;
        //playbackStateString = responseThreaded.toString();
        
        if(command == 'd') {
            int numbytes = dacCommand.size()+22;
           
            stateRecorder.recordStateThreadSafe(lastDataSentTimeMicros, responseThreaded.status.playback_state, lastReportedBufferFullness, roundTripTimeMicros, dacCommand.numPoints, responseThreaded.status.point_rate, numbytes);
          
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
        
        
		
        if(verbose || (responseThreaded.response!='a') || (responseThreaded.status.playback_flags & 0b010) || (lastReportedBufferFullness > dacTotalPointBufferCapacity)) {// || (command=='p')|| (command=='?')|| (command=='b')) {
            if(responseThreaded.response!='a') {
                lastNackReceived = responseThreaded.response;
                logNotice("INVALID COMMAND -------------------");
            }
            if(responseThreaded.status.playback_flags & 0b010) {
                //logNotice("BUFFER UNDERFLOW -------------------");
            }
            if(lastReportedBufferFullness > dacTotalPointBufferCapacity) {
                
                //logNotice("BUFFER OVERFLOW -------------------");
            }
            
            logNotice("response : "+ ofToString(responseThreaded.response) +  " command : " + ofToString(responseThreaded.command) );
            if(command == 'd') {
                logNotice("num points sent : " + ofToString(dacCommand.numPoints));
                logNotice("previousStateBufferFullness : " + ofToString(previousStateBufferFullness));
                //logNotice("time between ack and send : " + ofToString(lastDataSentTime  - previousLastAckTime));
                logNotice("lastReportedBufferSize : "+ofToString(lastReportedBufferFullness));
                logNotice("calculateBufferSizeByTimeSent() : "+ofToString(calculateBufferFullnessByTimeSent()));
                logNotice("calculateBufferSizeByTimeAcked() : "+ofToString(calculateBufferFullnessByTimeAcked()));

               // dacCommand.logData();
            }
			logNotice(responseThreaded.toString());
            
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
            
            if(responseThreaded.response=='I') {

                logNotice("INVALID COMMAND : " + ofToString(command));
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
		logNotice("Network failure or data received from EtherDream not 22 bytes :" + ofToString(n));
		// what do we do now?
        networkConnected = false;
		
	}
	
	if(failed) {
		beginSent = false;
		
		
		return false;
	} else {

		return true;
	}
}

string DacEtherDream :: getRawId(){
    //return "Ether Dream "+versionString+ " " +id;
    //if(isMercury) {
    //    return "Mercury "+id;
    //} else {
        return id;
   // }
        
}
string DacEtherDream :: getEdId(){
    //return "Ether Dream "+versionString+ " " +id;
    return id;
  
}
// TODO could this be a conflict?
int DacEtherDream :: getStatus(){
    
    string newerror;
    if(networkErrors.tryReceive(newerror)) {
        lastNetworkError = newerror;
    } 
    
	if(!networkConnected) return OFXLASER_DACSTATUS_ERROR;
    int status = lastReportedPlaybackState;
//    while(responseChannel.tryReceive(response)) ;
//    
//    status = response.status.playback_state;
    
	if(status <=1) return OFXLASER_DACSTATUS_WARNING;
	else if(status ==2) return OFXLASER_DACSTATUS_GOOD;
	else return OFXLASER_DACSTATUS_ERROR;
}


string DacEtherDream :: getEtherDreamStateString() {
// todo make a thread
    return playbackStateString;
    //    string statusstring;
//
//        statusstring = response.status.toString();
//   
//    return statusstring;
}

bool DacEtherDream :: sendBegin(){
	logNotice("sendBegin()");
    dacCommand.setAsBeginCommand(pps);
	beginSent = sendCommand(dacCommand);
	return beginSent;
}

bool DacEtherDream :: sendPrepare(){
	logNotice("sendPrepare()");
	prepareSendCount++;
    dacCommand.setCommand('p');
    return sendCommand(dacCommand);
}

bool DacEtherDream :: sendPointRate(uint32_t rate){
    dacCommand.setAsPointRateCommand(rate);
	return sendCommand(dacCommand);
}

void DacEtherDream :: logData() {
   //dacCommand.logData();
    std::stringstream str;
    dacCommand.getDataLog(str);
    logNotice(str.str());
    
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
        
    } catch (Poco::TimeoutException& exc) {
        //Handle your network errors.
        std::cerr << "sendBytes : Timeout error: " << exc.displayText() << endl;
        networkErrors.send(exc.displayText());
        //	isOpen = false;
        failed = true;
    } catch (Poco::Exception& exc) {
        //Handle your network errors.
        std::cerr << "sendBytes : Network error: " << exc.displayText() << endl;
        networkErrors.send(exc.displayText());
        networkerror = true;
        failed = true;
    } catch (...) {
        std::cerr << "sendBytes : unspecified error " << endl;
    }
	if(numBytesSent!=length) {
		//do something!
        std::cerr << "send fail, fewer bytes sent than expected : "<< numBytesSent << endl;
		failed = true;
	} else if (numBytesSent<0) {
		//do something!
        std::cerr << "send fail, sendBytes returned : "<< numBytesSent << endl;
		failed = true;
	}
	
	if(failed) {
		if(networkerror) {
            networkConnected = false; 
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
        if(networkConnected) {
            // lock to make sure other thread isn't also trying to send anything
            if(lock()) {
                sendStop();
                unlock();
                waitForAck('s');
            }
        }
        // also stops the thread :
        waitForThread(true, 1000); // 1 second time out
    }
        
    socket.close();

}

int DacEtherDream::getDacTotalPointBufferCapacity() {
    return dacTotalPointBufferCapacity;
}


int DacEtherDream::getMinimumDacBufferFullnessForLatency() {
    int minDacBufferFullness = DacNetworkBaseThreaded :: getMinimumDacBufferFullnessForLatency();
    // because the newest etherdreams use DMA transfer, they
    // always need at least 256 bytes in the buffer otherwise
    // they report a buffer under-run
    if(etherDreamData.softwareRevision>=30) {
        minDacBufferFullness = MAX(minDacBufferFullness, 256);
    }

    return minDacBufferFullness;
    
}

bool DacEtherDream::setPointsPerSecond(uint32_t newpps){
    //logNotice"setPointsPerSecond " + ofToString(newpps));
    if(!isThreadRunning()){
        pps = newPPS = newpps;
        return true;
    } else {
       // while(!lock());
        newPPS = newpps;
        if (!beginSent) {
            pps = (uint32_t)newPPS; // pps rate will get sent with begin anyway
            //unlock();
            return true;
        } else {
           // unlock();
            return false;
        }
    }
    
}

const vector<ofAbstractParameter*>& DacEtherDream :: getDisplayData() {
    
    return displayData;
}


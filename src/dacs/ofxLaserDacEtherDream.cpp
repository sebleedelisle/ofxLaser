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

	pointBufferDisplay.set("Point Buffer", 0,0,1799);
	latencyDisplay.set("Latency", 0,0,10000);
	reconnectCount.set("Reconnect Count", 0, 0,10);
    pointBufferMinParam.set("Min point buffer", 1500,0,3000);
	displayData.push_back(&pointBufferDisplay);
	displayData.push_back(&latencyDisplay);
	displayData.push_back(&reconnectCount);
    
    
    //TODO
    // this should really be dependent on network latency	
    // and also we should have the option of lower latency on better networks
	// also make this adjustable
	// also maxBufferedPoints should be higher on etherdream 2
	//
    pointBufferCapacity = 1799; 				// the maximum points to fill the buffer with
    pointBufferMin = 1500;    //500;//100; 	// the minimum number of points to buffer before
										// we tell the ED to start playing
										// TODO - these should be time based!
    
    lastCommandSendTime = 0;
    lastAckTime = 0;
    lastDataSentTime = 0;

}


const vector<ofAbstractParameter*>& DacEtherDream :: getDisplayData() {
	if(lock()) {
	
		//pointBufferDisplay += (response.status.buffer_fullness-pointBufferDisplay)*1;
		
		int pointssincelastmessage = (float)((ofGetElapsedTimeMicros()-lastMessageTimeMicros)*response.status.point_rate)/1000000.0f;
		
		pointBufferDisplay = response.status.buffer_fullness-pointssincelastmessage;
																 
		latencyDisplay += (latencyMicros - latencyDisplay)*0.1;
		reconnectCount = prepareSendCount;
	
		unlock();
	}
	
	return displayData;
}


DacEtherDream :: ~DacEtherDream(){
	
	close();
	
	for (size_t i= 0; i < sparePoints.size(); ++i) {
		delete sparePoints[i]; // Calls ~object (which deallocates tmp[i]->foo)
		// and deallocates *tmp[i]
	}
	sparePoints.clear();
	for (size_t i= 0; i < bufferedPoints.size(); ++i) {
		delete bufferedPoints[i]; // Calls ~object (which deallocates tmp[i]->foo)
		// and deallocates *tmp[i]
	}
	bufferedPoints.clear();
	
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

bool DacEtherDream :: setup(string ip) {
    ofLogError("DACs are no longer set up in code! Do it within the app instead");
    throw; 
    return false;
}

void DacEtherDream :: setup(string _id, string _ip, EtherDreamData& ed) {
	
	pps = 0;
	pps = newPPS = 30000; // this is always sent on begin
	queuedPPSChangeMessages = 0;
	connected = false;
	ipaddress = _ip;
    id = _id;
    etherDreamData = ed;
    lastAckTime = ofGetElapsedTimeMicros();
    
    // EtherDream Hardware revision :
    // ED v1 : hardwareRevision 2, softwareRevision 2
    // ED v2 : hardwareRevision 10, softwareRevision 2
    // ED v3 : hardwareRevision 30, softwareRevision 3
    
    // TODO update max point rate from dacdata
    pointBufferCapacity = ed.bufferCapacity;
    //pointBufferMin = pointBufferCapacity*0.70;
	
	Poco::Timespan timeout( 1000000 * 1 / 1); // 1/1 second timeout
	
	try {
		// EtherDreams always talk on port 7765
		Poco::Net::SocketAddress sa(ipaddress, 7765);
		//ofLog(OF_LOG_NOTICE, "TIMEOUT" + ofToString(timeout.totalSeconds()));
        
		socket.connect(sa, timeout);
		socket.setSendTimeout(timeout);
		socket.setReceiveTimeout(timeout);
		
		connected = true;
	} catch (Poco::Exception& exc) {
		//Handle your network errors.
		ofLog(OF_LOG_ERROR,  "DacEtherDream setup failed - Network error: " +ipaddress+" "+ exc.displayText());
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
	

	}
}


void DacEtherDream :: threadedFunction(){
    
    // the dac sends a ping response as soon as you connect
    waitForAck('?');
    
    bool needToSendPrepare = true;
    // in older ether dreams this doesn't seem to reset even if you disconnect and reconnect
    if(response.status.playback_state == PLAYBACK_PREPARED) resetFlag = true;
       //bool needToSendBegin = true;
    
    while(isThreadRunning()) {
        
        
        if(lock()) {
            if(pointBufferMinParam!=pointBufferMin)  pointBufferMin = pointBufferMinParam;
            unlock();
        }
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
           // needToSendPrepare = true;
            
            // clear frame
            EtherDreamDacPoint lastPoint;
            if(framePoints.size()>0) {
                lastPoint = framePoints[0];
               
            } else {
                lastPoint.x = 400;
                lastPoint.y = 400;
            }
            lastPoint.r = 0;
            lastPoint.g = 0;
            lastPoint.b = 0;
            // go through the buffer and clear all the points. Could we just delete the buffer?
            for(EtherDreamDacPoint* point : bufferedPoints) {
                *point = lastPoint;
            }
            
            ofLogNotice ("RESET DAC--------------------------------");
           // cout << lastPoint.x << " "<< lastPoint.y << " "<< lastPoint.r << " "<< lastPoint.g << " "<< lastPoint.b << endl;
           // cout << " framePoints size :" << framePoints.size() << " buffered points size : " << bufferedPoints.size() << endl;
            
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
        
        // if state is prepared or playing, and we have points in the buffer, then send the points
        if(connected && (response.status.playback_state!=PLAYBACK_IDLE)) {
            
            // figure out when the next update should be
            int elapsedMicros = ofGetElapsedTimeMicros() - lastDataSentTime;
            // find out buffer based on last update - TODO allow for latency?
            int bufferFullness =response.status.buffer_fullness - (((float)elapsedMicros/1000000.0f) * pps);
            
            if(etherDreamData.hardwareRevision>=30) {
                // if we have an ether dream v3 then we have to keep a minimum of
                // 256 points in the buffer otherwise it'll error out.
                bufferFullness-=256;
                
            }
            bufferFullness = MAX(0,bufferFullness);
            //int pointsToFill = (minPointBufferSize - bufferFullness);
            int pointsUntilEmpty = MAX(0, bufferFullness - pointBufferMin);
            
            
            int microsToWait = pointsUntilEmpty * (1000000.0f/pps);
            
            int maxPointsToSend = pointBufferCapacity - pointBufferMin;
            if(bufferFullness < pointBufferMin) maxPointsToSend += (pointBufferMin-bufferFullness);
            //cout << latencyMicros/1000 << " " << microsToWait << " " << bufferFullness << " " << response.status.buffer_fullness <<  " " << maxPointsToSend << endl;
            
            // min buffer amount
            if(microsToWait>0)
                usleep(microsToWait);
            
            // recalculate buffer fullness dependent on time
            elapsedMicros = ofGetElapsedTimeMicros() - lastAckTime;
            bufferFullness = MAX(0, response.status.buffer_fullness - (((float)elapsedMicros/1000000.0f) * pps));
            //check buffer and send the next points
            while(!lock()) {}
                bool dataSent = sendData(pointBufferMin-bufferFullness, maxPointsToSend-bufferFullness);
            unlock();
            if(dataSent) {
                //if(verbose) logData();
                // note if this fails, the connected flag is disabled which triggers the reset flag below
                waitForAck('d');
                
            }
            
            
        }
        // if state is prepared and we have sent enough points and we haven't already, send begin
        if(connected && (response.status.playback_state==PLAYBACK_PREPARED) && (response.status.buffer_fullness>=pointBufferMin)) {
            sendBegin();
            beginSent = waitForAck('b');
            if(beginSent)  {
                ofLogNotice("waitForAck('p') success");
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



void DacEtherDream :: reset() {
	if(lock()) {
		resetFlag = true;
		unlock();
	}
	
}

void DacEtherDream :: closeWhileRunning() {
	if(!connected) return;
	while(!lock());
	sendStop();
	unlock();
    
	waitForThread();
	
	while(!lock());
	socket.close();
	unlock();
	
	
}


bool DacEtherDream:: sendFrame(const vector<Point>& points){

//	ofLog(OF_LOG_NOTICE, "point create count  : " + ofToString(dac_point::createCount));
//	ofLog(OF_LOG_NOTICE, "point destroy count : " + ofToString(dac_point::destroyCount));
	//int maxBufferSize = 1000;
	EtherDreamDacPoint& p1 = sendpoint;
	
	// if we already have too many points in the buffer,
	// then it means that we need to skip this frame
	
	//if(isReplaying || (bufferedPoints.size()<pointsToSendBeforePlaying)) {

	
		if(lock()) {
			frameMode = true;
			framePoints.resize(points.size());

			for(size_t i= 0; i<points.size(); i++) {
				
				const Point& p2 = points[i];
				p1.x = ofMap(p2.x,0,800,ETHERDREAM_MIN, ETHERDREAM_MAX);
				p1.y = ofMap(p2.y,800,0,ETHERDREAM_MIN, ETHERDREAM_MAX); // Y is UP
				p1.r = p2.r/255.0f*65535;
				p1.g = p2.g/255.0f*65535;
				p1.b = p2.b/255.0f*65535;
				p1.i = 0;
				p1.u1 = 0;
				p1.u2 = 0;
				//addPoint(p1);
				
				framePoints[i] = p1;
			}
			newFrame = true;
			unlock();
		}
		return true;
	//} else {
		// we've skipped this frame... TODO - dropped frame count?
	//	return false;
	//}
}


inline bool DacEtherDream :: sendData(int minPointsToSend, int maxPointsToSend){
	
    minPointsToSend = MAX(0,minPointsToSend);
    maxPointsToSend = MAX(0,maxPointsToSend);
	
	// this is to stop sending too many points into the future
	//float bufferTime = 0.5f; // was 0.1f - TODO make this a param!
	//if(npoints>pps*bufferTime) npoints = pps*bufferTime;
	
		
	// TODO make these params
	//float minBufferTime = 0.01; // (30 frames at 30k)
	//int minBuffer = minPointBufferSize; // pps*minBufferTime;
	
    
    int numpointstosend;
    
	// system for resending existing frames... probably can be optimised
	// if we're in frame mode and we're replaying frames
    
	if(frameMode) {
		
        numpointstosend = minPointsToSend;
        // send points up to the minimum buffer size unless
        // we have a new frame and if we dont already have the max points,
        // then definitely send it all
        if(newFrame && bufferedPoints.size()<maxPointsToSend) {
            numpointstosend = MAX(minPointsToSend, framePoints.size());
        }
		
		while((framePoints.size()>0) && (bufferedPoints.size()<minPointsToSend)) {
			//cout << npointstosend << " " << minpointcount << endl;
			// send the frame!
			for(size_t i= 0; i<framePoints.size(); i++) {
				addPoint(framePoints[i]);
			}
			newFrame = false;
			numpointstosend = MIN(bufferedPoints.size(), maxPointsToSend);
		
		}
        if(numpointstosend==0) {
            if(verbose) ofLogNotice("sendData : no points to send");
            return false;
        }
		
    } else {
        // for non-frame mode, just send the buffer
       numpointstosend = MIN(bufferedPoints.size(), maxPointsToSend);
    }
    
    dacCommand.setDataCommand(numpointstosend);
	
	EtherDreamDacPoint& p = sendpoint;
	
	for(int i = 0; i<numpointstosend; i++) {
		
		if(bufferedPoints.size()>0) {
            // pop the point off the front
			p = *bufferedPoints[0]; // copy assignment
            // if we haven't started the laser yet, maybe turn the
            // brightness off?
            if(!beginSent) {
                p.r = p.g = p.b = 0;
            }
            
            p.u1 = 0;
            p.u2 = 0;
            p.i = 0;
            
			sparePoints.push_back(bufferedPoints[0]); // recycling system
			bufferedPoints.pop_front(); // no longer destroys point
			lastpoint = p; //
		} else  {
			// just send some blank points in the same position as the
			// last point
			// TODO count the blanks!
			
			p = lastpoint;
			
			
			p.r = 0;
			p.g = 0;
			p.b = 0;
			p.u1 = 0;
			p.u2 = 0;
            p.i = 0;
		}
		
		if(queuedPPSChangeMessages>0) {
			// bit 15 is a flag to tell the DAC about a new point rate
			p.control = 0b1000000000000000;
            ofLogNotice("PPS Change queue "+ofToString(queuedPPSChangeMessages));
			queuedPPSChangeMessages--;
        } else {
            p.control = 0;
        }
       // cout << " ctl : " + std::bitset<16>(p.control).to_string() + "\n";

        dacCommand.addPoint(p);
		
	}
	
	numBytesSent = dacCommand.size();
    //printf("outbuffer.size() : %zu numpoints : %d, buffer fullness : %d \n", dacCommand.size(), numpointstosend, response.status.buffer_fullness);
	if(numBytesSent>=100000) {
		ofLog(OF_LOG_ERROR, "ofxLaser::DacEtherDream - too many bytes to send! - " + ofToString(numBytesSent));
	}
	
  
    
	if(verbose) {
        ofLogNotice("sending points : " + ofToString(numpointstosend));
	}
	
    // check we sent enough points
    if(dacCommand.numPointsExpected!=dacCommand.numPoints) {
        ofLogError("DacEtherDream, incorrect point count sent, expected "+ofToString(dacCommand.numPointsExpected)+", got "+ofToString(dacCommand.numPoints));
    }
    
	//cout << "sent " << numBytesSent << " bytes" << endl;
	bool success =  sendCommand(dacCommand);
    if(success) lastDataSentTime = ofGetElapsedTimeMicros();
    return success;
	//cout << "numPointsToSend " << numPointsToSend << endl;
}



bool DacEtherDream:: sendPoints(const vector<Point>& points){
    // max half second buffer
	//cout << "DacEtherDream::sendPoints -------------------------" << endl;
    if(bufferedPoints.size()>pps*0.5) {
        return false;
    }
	
	
    EtherDreamDacPoint p1;
	if(lock()) {
		frameMode = false;
		
		for(size_t i= 0; i<points.size(); i++) {
			
			const Point& p2 = points[i];
			p1.x = ofMap(p2.x,0,800, ETHERDREAM_MIN, ETHERDREAM_MAX);
			p1.y = ofMap(p2.y,800,0, ETHERDREAM_MIN, ETHERDREAM_MAX); // Y is UP in ilda specs
			p1.r = p2.r/255.0f*65535;
			p1.g = p2.g/255.0f*65535;
			p1.b = p2.b/255.0f*65535;
			p1.i = 0;
			p1.u1 = 0;
			p1.u2 = 0;
			addPoint(p1);

		}
		unlock();
	}
	return true;
  
}


inline bool DacEtherDream :: addPoint(const EtherDreamDacPoint &point ){
	EtherDreamDacPoint* p = getDacPoint();
	*p = point; // copy assignment hopefully!
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
	
	int n = 0;
    while ((n==0) && (!failed)) {
		if(!isThreadRunning()) return false;
		// i think this should block until it gets bytes
		
		try {
			n = socket.receiveBytes(inBuffer, 22);
			lastMessageTimeMicros = ofGetElapsedTimeMicros();
			
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
	
	if(n==22) {
        
        lastAckTime = lastMessageTimeMicros ;
        latencyMicros  = lastMessageTimeMicros - lastCommandSendTime;
		connected = true;
        response.deserialize(inBuffer);
		
        if((command == 'd') && lock()) {
//            bufferState.timeMicros = lastDataSentTime;
//            bufferState.buffer = response.status.buffer_fullness;
//            bufferState.playing = response.status.playback_state == PLAYBACK_PLAYING;
//            bufferState.pointRate = response.status.point_rate;
            unlock();
            stateRecorder.recordState(lastDataSentTime, response.status.playback_state, response.status.buffer_fullness, latencyMicros, dacCommand.numPoints, response.status.point_rate); 
            unlock();
        }

		
        if(verbose || (response.response!='a')) {// || (command=='p')|| (command=='?')|| (command=='b')) {
            ofLog(OF_LOG_NOTICE, "response : "+ ofToString(response.response) +  " command : " + ofToString(response.command) );
			
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
		connected = false;
		
		return false;
	} else {
//        if(lock()) {
//            addLatencyRecording(latencyMicros);
//            unlock();
//        }
		return true;
	}
}


//void DacEtherDream :: addLatencyRecording(int latency){
//    latencyHistory[latencyHistoryOffset] = ((float)latency/1000.0f);
//    latencyHistoryOffset++;
//    if(latencyHistoryOffset>=latencyHistorySize) latencyHistoryOffset = 0;
//}

string DacEtherDream :: getId(){
	return "EtherDream "+id;
   // string getLabel() override{return "Laserdock " + ofToString(serialNumber);};
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
			closeWhileRunning();
			setup(id, ipaddress, etherDreamData);
		}
		beginSent = false;
        
		return false;
	}
	return true;
}


EtherDreamDacPoint*  DacEtherDream :: getDacPoint() {
	EtherDreamDacPoint* p;
	if(sparePoints.size()==0) {
		p= new EtherDreamDacPoint();
		//ofLog(OF_LOG_NOTICE, "new point made");
	} else {
		p = sparePoints.back();
		sparePoints.pop_back();
        // not sure if we need this
        p->control = 0;
        p->x =
        p->y = 0;
        p->r =
        p->g =
        p->b =
        p->i =
        p->u1 =
        p->u2 = 0;
	}
   
	return p;
}

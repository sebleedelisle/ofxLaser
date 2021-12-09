 //
//  ofxLaserDacEtherdream.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#include "ofxLaserDacEtherdream.h"

using namespace ofxLaser;

#ifdef _MSC_VER
#include <Windows.h>
#endif
//
//int dac_point:: createCount = 0;
//int dac_point:: destroyCount = 0;

DacEtherdream :: DacEtherdream(){

	pointBufferDisplay.set("Point Buffer", 0,0,1799);
	latencyDisplay.set("Latency", 0,0,10000);
	reconnectCount.set("Reconnect Count", 0, 0,10);
	displayData.push_back(&pointBufferDisplay);
	displayData.push_back(&latencyDisplay);
	displayData.push_back(&reconnectCount);
    numPointsToSend = 0;
    
    
    //TODO
    // this should really be dependent on network latency	
    // and also we should have the option of lower latency on better networks
	// also make this adjustable
	// also maxBufferedPoints should be higher on etherdream 2
	//
    dacBufferSize = 1200; 				// the maximum points to fill the buffer with
    pointsToSendBeforePlaying = 500;    //500;//100; 	// the minimum number of points to buffer before
										// we tell the ED to start playing
										// TODO - these should be time based!

}
const vector<ofAbstractParameter*>& DacEtherdream :: getDisplayData() {
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

DacEtherdream :: ~DacEtherdream(){
	
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

void DacEtherdream :: close() {
	
	
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

bool DacEtherdream :: setup(string ip) {
    ofLogError("DACs are no longer set up in code! Do it within the app instead");
    throw; 
    return false;
}

void DacEtherdream :: setup(string _id, string _ip) {
	
	pps = 0;
	pps = newPPS = 30000; // this is always sent on begin
	queuedPPSChangeMessages = 0;
	connected = false;
	ipaddress = _ip;
    id = _id;
	
	Poco::Timespan timeout(1 * 250000); // 1/4 seconds timeout
	
	try {
		// Etherdreams always talk on port 7765
		Poco::Net::SocketAddress sa(ipaddress, 7765);
		//ofLog(OF_LOG_NOTICE, "TIMEOUT" + ofToString(timeout.totalSeconds()));
		socket.connect(sa, timeout);
		socket.setSendTimeout(timeout);
		socket.setReceiveTimeout(timeout);
		
		connected = true;
	} catch (Poco::Exception& exc) {
		//Handle your network errors.
		ofLog(OF_LOG_ERROR,  "DacEtherdream setup failed - Network error: " +ipaddress+" "+ exc.displayText());
		connected = false;

	}catch (Poco::Net::HostNotFoundException& exc) {
		//Handle your network errors.
		ofLog(OF_LOG_ERROR,  "DacEtherdream setup failed - host not found: " + exc.displayText());
		connected = false;
		
	}catch (Poco::TimeoutException& exc) {
		//Handle your network errors.
		ofLog(OF_LOG_ERROR,  "DacEtherdream setup failed - Timeout error: " + exc.displayText());
		connected = false;
		
	}
	catch(...){
		ofLog(OF_LOG_ERROR, "DacEtherdream setup failed - unknown error");
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
		param.sched_priority = 60; // 89;
		pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &param );
#else
		// windows implementation
		SetThreadPriority( thread.native_handle(), THREAD_PRIORITY_HIGHEST);
#endif
	

	}
}

void DacEtherdream :: reset() {
	if(lock()) {
		resetFlag = true;
		unlock();
	}
	
}

void DacEtherdream :: closeWhileRunning() {
	if(!connected) return;
	while(!lock());
	sendStop();
	unlock();
    
	waitForThread();
	
	while(!lock());
	socket.close();
	unlock();
	
	
}



bool DacEtherdream:: sendFrame(const vector<Point>& points){

//	ofLog(OF_LOG_NOTICE, "point create count  : " + ofToString(dac_point::createCount));
//	ofLog(OF_LOG_NOTICE, "point destroy count : " + ofToString(dac_point::destroyCount));
	//int maxBufferSize = 1000;
	dac_point& p1 = sendpoint;
	
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


inline bool DacEtherdream :: sendData(){
	
	//data_command d;
	uint8_t command = 'd';
	
	// numPointsToSend is automatically calculated when we get data back from the DAC
	uint16_t npointstosend = numPointsToSend;
	
	// this is to stop sending too many points into the future
	//float bufferTime = 0.5f; // was 0.1f - TODO make this a param!
	//if(npoints>pps*bufferTime) npoints = pps*bufferTime;
	
	if(npointstosend<=0) npointstosend = 0;
	// i'm not sure there's any point in doing this... because
	// we wait to hear back from the DAC anyway...
	//numPointsToSend-=npoints;
	
	// TODO make these params
	//float minBufferTime = 0.01; // (30 frames at 30k)
	int minBuffer = pointsToSendBeforePlaying; // pps*minBufferTime;
	
	// system for resending existing frames... probably can be optimised
	// if we're in frame mode and we're replaying frames
	if(frameMode) {
		
		// send as many points as we can,
		npointstosend = MIN(bufferedPoints.size(), numPointsToSend);
		
		// now calculate the min points before we send the new frame...
		// if we have a new frame, then send it as long as we have spare points
		int minpointcount;
		if(newFrame) {
			minpointcount = numPointsToSend;
		} else { // otherwise just send it if we're at the absolute minimum
			minpointcount = MAX(minBuffer - response.status.buffer_fullness,0);
		}
		
		while((framePoints.size()>0) && (npointstosend<minpointcount)) {
			//cout << npointstosend << " " << minpointcount << endl;
			// send the frame!
			for(size_t i= 0; i<framePoints.size(); i++) {
				addPoint(framePoints[i]);
			}
			newFrame = false;
			npointstosend = MIN(bufferedPoints.size(), numPointsToSend);
		
		}
		
		
	}
	outbuffer[0]= command;
	writeUInt16ToBytes(npointstosend, &outbuffer[1]);
	int pos = 3;
	
	dac_point& p = sendpoint;
	
	for(int i = 0; i<npointstosend; i++) {
		
		if(bufferedPoints.size()>0) {
			p = *bufferedPoints[0]; // copy assignment
			sparePoints.push_back(bufferedPoints[0]); // recycling system
			bufferedPoints.pop_front(); // no longer destroys point
			lastpoint = p; //
		} else  {
			// just send some blank points in the same position as the
			// last point
			// TODO count the blanks!
			
			p = lastpoint;
			
			p.i = 0;
			p.r = 0;
			p.g = 0;
			p.b = 0;
			p.u1 = 0;
			p.u2 = 0;
		}
		
		if(queuedPPSChangeMessages>0) {
			// bit 15 is a flag to tell the DAC about a new point rate
			p.control = 0b1000000000000000;
			queuedPPSChangeMessages--;
		}
		
		writeUInt16ToBytes(p.control, &outbuffer[pos]);
		pos+=2;
		writeInt16ToBytes(p.x, &outbuffer[pos]);
		pos+=2;
		writeInt16ToBytes(p.y, &outbuffer[pos]);
		pos+=2;
		writeUInt16ToBytes(p.r, &outbuffer[pos]);
		pos+=2;
		writeUInt16ToBytes(p.g, &outbuffer[pos]);
		pos+=2;
		writeUInt16ToBytes(p.b, &outbuffer[pos]);
		pos+=2;
		writeUInt16ToBytes(p.i, &outbuffer[pos]);
		pos+=2;
		writeUInt16ToBytes(p.u1, &outbuffer[pos]);
		pos+=2;
		writeUInt16ToBytes(p.u2, &outbuffer[pos]);
		pos+=2;
		
	}
	
	numBytesSent = pos;
	if(numBytesSent>=100000) {
		
		ofLog(OF_LOG_ERROR, "ofxLaser::DacEtherdream - too many bytes to send! - " + ofToString(numBytesSent));
	}
	
	if(verbose) {
		ofLogNotice("sending points : " + ofToString(npointstosend));
	}
	
	//cout << "sent " << numBytesSent << " bytes" << endl;
	return sendBytes(outbuffer, numBytesSent);
	
	//cout << "numPointsToSend " << numPointsToSend << endl;
}



bool DacEtherdream:: sendPoints(const vector<Point>& points){
    // max half second buffer
	//cout << "DacEtherdream::sendPoints -------------------------" << endl;
    if(bufferedPoints.size()>pps*0.5) {
        return false;
    }
	
	
    dac_point p1;
	if(lock()) {
		frameMode = false;
		
		for(size_t i= 0; i<points.size(); i++) {
			
			const Point& p2 = points[i];
			p1.x = ofMap(p2.x,0,800,ETHERDREAM_MIN, ETHERDREAM_MAX);
			p1.y = ofMap(p2.y,800,0,ETHERDREAM_MIN, ETHERDREAM_MAX); // Y is UP in ilda specs
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


inline bool DacEtherdream :: addPoint(const dac_point &point ){
	
	//if(response.status.playback_state!=PLAYBACK_IDLE) {
		// because we are blocking this should wait
		//if(lock()){
			// makes a copy
			//dac_point* p = getDacPoint();
			//*p = point; // copy assignment hopefully!
			//bufferedPoints.push_back(p);
		//	unlock();
		//}
	//}
	
	dac_point* p = getDacPoint();
	*p = point; // copy assignment hopefully!
	bufferedPoints.push_back(p);
	return true;
}


void DacEtherdream :: threadedFunction(){
	
	waitForAck('?');
	
	bool needToSendPrepare = true;
	//bool needToSendBegin = true;
	
	while(isThreadRunning()) {
		
		
		// flag 010 is an underflow check. So if it didn't
		// get enough points when it needed them, we have to restart
		// the stream.
//		if((response.status.playback_state == PLAYBACK_IDLE) && (response.status.playback_flags & 0b010)) {
//			needToSendPrepare = true;
//		}
//
//		// this would be for the first time we connect to the etherdream
//		if((response.status.light_engine_state == LIGHT_ENGINE_READY) && (!prepareSent)) {
//			needToSendPrepare = true;
//		}
		
		if(resetFlag) {
			
			resetFlag = false;
			
			// clear the socket of data
			try {
				int n = socket.receiveBytes(buffer, 1000);
			} catch(...) {
				// doesn't matter
			}
			sendPing();
			waitForAck('?');
			if(response.status.light_engine_state == LIGHT_ENGINE_ESTOP) {
				sendClear();
				waitForAck('c');
			}
			prepareSendCount = 0;
			
			// clear frame
            dac_point lastPoint;
            if(framePoints.size()>0) {
                lastPoint = framePoints[0];
               
            } else {
                lastPoint.x = 400;
                lastPoint.y = 400;
            }
            lastPoint.r = 0;
            lastPoint.g = 0;
            lastPoint.b = 0;
            
			for(dac_point* point : bufferedPoints) {
                *point = lastPoint;
				
			}
			
		}

		if((response.status.playback_state == PLAYBACK_IDLE) && (response.status.light_engine_state == LIGHT_ENGINE_READY)) {
			needToSendPrepare = true;
		}
		
		if(needToSendPrepare) {
			;
			bool success = (sendPrepare()	&& waitForAck('p'));
			
			if( success ) {
				needToSendPrepare = false;
				beginSent = false;
			}
			//else prepareSent = false;
		}
		
		// if we're playing and we have a new point rate, send it!
		if(connected && (response.status.playback_state==PLAYBACK_PLAYING) && (newPPS!=pps)) {
			
			
			if(sendPointRate(pps)){
				pps = newPPS;
				waitForAck('q');
				queuedPPSChangeMessages++;
			}
			
			
		}
		
		// if state is prepared or playing, and we have points in the buffer, then send the points
		if(connected && (response.status.playback_state!=PLAYBACK_IDLE)) {
			
			// min buffer amount
			if(numPointsToSend>pointsToSendBeforePlaying){
				//check buffer and send the next points
				while(!lock()) {}
				sendData();
				unlock();
				waitForAck('d');
			} else if(isThreadRunning()) {
				// if we're not sending data, then let's ping the etherdream so it can
				// tell us how many points can fit into its buffer.
				sendPing(); // ping is '?' character
				waitForAck('?');
			}
			
		}
		// if state is prepared and we have sent enough points and we haven't already, send begin
		if(connected && (response.status.playback_state==PLAYBACK_PREPARED) && (response.status.buffer_fullness>=pointsToSendBeforePlaying)) {
			sendBegin();
			beginSent = waitForAck('b');

			
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

bool DacEtherdream::setPointsPerSecond(uint32_t newpps){
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


inline bool DacEtherdream::waitForAck(char command) {
	
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
//	int count = 0;
//	int timeoutwait = 1000;// one second - ish
//	while(waiting && !failed) {
//
//
//		if(lock()) {
//            if(socket.available()){
//				waiting = false;
//            }
//			unlock();
//		}
//
//		if(!isThreadRunning()) return false;
//
//        if(waiting) {
//            yield();
//            sleep(1);
//			//count++;
//        }
//
////		if(count > timeoutwait) {
////			failed = true;
////			ofLog(OF_LOG_WARNING, "DACEtherdream.waitForAck timeout)" );
////
////		}
//	}
	
	
	int n = 0;
    while ((n==0) && (!failed)) {
		if(!isThreadRunning()) return false;
		// i think this should block until it gets bytes
		
		try {
			n = socket.receiveBytes(buffer, 22);
			lastMessageTimeMicros = ofGetElapsedTimeMicros();
			latencyMicros = lastMessageTimeMicros - startTime;
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
		//	ofLog(OF_LOG_WARNING, "DACEtherdream.waitForAck timeout)" );
			
		//}
	}
	// = count;
	// TODO - handle incomplete data
	
	//cout << "received " << n << "bytes" <<endl;
	
	if(n==22) {
		
		connected = true;
		response.response = buffer[0];
		response.command = buffer[1];
		response.status.protocol = buffer[2];
		response.status.light_engine_state = buffer[3];
		response.status.playback_state = buffer [4];
		response.status.source = buffer[5];
		response.status.light_engine_flags = bytesToUInt16(&buffer[6]);
		response.status.playback_flags =  bytesToUInt16(&buffer[8]);
		response.status.source_flags =  bytesToUInt16(&buffer[10]);
		response.status.buffer_fullness = bytesToUInt16(&buffer[12]);
		response.status.point_rate = bytesToUInt32(&buffer[14]);
		response.status.point_count = bytesToUInt32(&buffer[18]);
		
        if((response.response == 'a') && (response.status.playback_state!= PLAYBACK_IDLE )) {
            numPointsToSend = dacBufferSize - response.status.buffer_fullness;
            if(numPointsToSend<0) numPointsToSend = 0;
			
//			// numpointstosend should be
//			if((bufferedPoints.size()>minBuffer) && (numPointsToSend>bufferedPoints.size())) {
//				numPointsToSend
//			}
			
        }
		
		if(verbose) {
			if(command == 'd') logData();
		}
		
		if(verbose || (response.response!='a')) {
            ofLog(OF_LOG_NOTICE, "response : "+ ofToString(response.response) +  " command : " + ofToString(response.command) );
//            ofLog(OF_LOG_NOTICE, "num points sent : "+ ofToString(numPointsToSend) );
//
			string data = "";
			data+= "\nprotocol           : " + to_string(response.status.protocol) + "\n";
			data+= "light_engine_state : " + light_engine_states[response.status.light_engine_state]+" "+to_string(response.status.light_engine_state) + "\n";
			data+= "playback_state     : " + playback_states[response.status.playback_state]+" "+to_string(response.status.playback_state) + "\n";
			data+= "source             : " + to_string(response.status.source) + "\n";
			data+= "light_engine_flags : " + std::bitset<5>(response.status.light_engine_flags).to_string() + "\n";
			data+= "playback_flags     : " + std::bitset<3>(response.status.playback_flags).to_string() + "\n";
			data+= "source_flags       : " + to_string(response.status.source_flags) + "\n";
			data+= "buffer_fullness    : " + to_string(response.status.buffer_fullness) + "\n";
			data+= "point_rate         : " + to_string(response.status.point_rate) + "\n";
			data+= "point_count        : " + to_string(response.status.point_count) + "\n";
			
			cout << data << endl;
            
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
                
                // ofLog(OF_LOG_ERROR, ofToString(outbuffer));
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
		// 00010 : Emergency stop due to E-Stop input to projector (not sure how etherdream would know?)
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
		ofLog(OF_LOG_NOTICE, "data received from Etherdream not 22 bytes :" + ofToString(n));
		// what do we do now?
		
	}
	
	if(failed) {
		beginSent = false;
		connected = false;
		
		return false;
	} else {
		
		return true;
	}
}

string DacEtherdream ::getId(){
	return "Etherdream "+id;
   // string getLabel() override{return "Laserdock " + ofToString(serialNumber);};
}

int DacEtherdream :: getStatus(){
	if(!connected) return OFXLASER_DACSTATUS_ERROR;
	if(response.status.playback_state <=1) return OFXLASER_DACSTATUS_WARNING;
	else if(response.status.playback_state ==2) return OFXLASER_DACSTATUS_GOOD;
	else return OFXLASER_DACSTATUS_ERROR;


}


inline bool DacEtherdream :: sendBegin(){
	ofLog(OF_LOG_NOTICE, "sendBegin()");
	begin_command b;
	b.command = 'b';
	b.low_water_mark = 0 ;
	b.point_rate = pps;
	
	buffer[0] = b.command;
	writeUInt16ToBytes(b.low_water_mark, &buffer[1]);
	writeUInt32ToBytes(b.point_rate, &buffer[3]);
	
	//	for(int i = 0; i<7;i++) {
	//		cout << i << ":" <<buffer[i]<< "(" << std::dec << (int)buffer[i] << std::dec <<")\n";
	//	}
	
	//int n = socket->sendBytes(&send[0],7);
	beginSent = sendBytes(buffer, 7);
	
	return beginSent;
	
	//cout << "sent " << n << " bytes" << endl;
	//beginSent = true;
}
inline bool DacEtherdream :: sendPrepare(){
	ofLog(OF_LOG_NOTICE, "sendPrepare()");
	prepareSendCount++;
	uint8_t send = 0x70; //'p'
	return sendBytes(&send,1);
	//cout << "sent " << n << " bytes" << endl;
	//prepareSent = true;

}


inline bool DacEtherdream :: sendPointRate(uint32_t rate){
	outbuffer[0] = 'q';
	writeUInt32ToBytes(rate, &outbuffer[1]);
	return sendBytes(outbuffer, 5);
	
}

void DacEtherdream :: logData() {
    //bytesToUInt16(&buffer[8]
    string data = "---------------------------------------------------------------\n";
    data+= "command            : ";
    data+=(char)outbuffer[0];
    data+= "\n";
    int numpoints =bytesToUInt16(&outbuffer[1]);
    data+= "num points         : " + to_string(numpoints) + "\n";
    int pos = 3;
    
    for(int i = 0; i<numpoints; i++) {
        data+= "------------------------------ npoint # " + to_string(i) + "\n";
        
        data+= " ctl : " + to_string(bytesToUInt16(&outbuffer[pos])) + "\n";
        pos+=2;
        data+= " x   : " + to_string(bytesToInt16(&outbuffer[pos])) + "\n";
        pos+=2;
        data+= " y   : " + to_string(bytesToInt16(&outbuffer[pos])) + "\n";
        pos+=2;
        data+= " r   : " + to_string(bytesToUInt16(&outbuffer[pos])) + "\n";
        pos+=2;
        data+= " g   : " + to_string(bytesToUInt16(&outbuffer[pos])) + "\n";
        pos+=2;
        data+= " b   : " + to_string(bytesToUInt16(&outbuffer[pos])) + "\n";
        pos+=2;
        data+= " i   : " + to_string(bytesToUInt16(&outbuffer[pos])) + "\n";
        pos+=2;
        data+= " u1   : " + to_string(bytesToUInt16(&outbuffer[pos])) + "\n";
        pos+=2;
        data+= " u2   : " + to_string(bytesToUInt16(&outbuffer[pos])) + "\n";
        pos+=2;
    }
    cout << data << endl;
    
}
bool DacEtherdream :: sendPing(){
	uint8_t ping = '?';
	return sendBytes(&ping, 1);
}
bool DacEtherdream :: sendEStop(){
	uint8_t ping = '\0';
	return sendBytes(&ping, 1);
}
bool DacEtherdream :: sendStop(){
	// non-emergency stop
	uint8_t ping = 's';
	return sendBytes(&ping, 1);
}
bool DacEtherdream :: sendClear(){
	uint8_t ping = 'c';
	// clear emergency stop
	return sendBytes(&ping, 1);
}

bool DacEtherdream :: sendBytes(const uint8_t* buffer, int length) {
	
	int numBytesSent = 0;
	bool failed = false;
	bool networkerror = false;
    startTime = ofGetElapsedTimeMicros();//  count = 0;

	try {
		numBytesSent = socket.sendBytes(buffer, length);
		if(verbose && (length>1)) {
			cout << "command sent : " << buffer[0] << " numBytesSent : " << numBytesSent <<  endl;
			for(int i = 1; i<length; i++) {
				
				printf("%X2 ", (uint8_t)buffer[i]);
				if(i%8==0) cout << endl;
			}
			cout << endl;
		}
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
			setup(id, ipaddress);
		}
		beginSent = false;
		//prepareSent = false;
		return false;
	}
	return true;
}
//bool DacEtherdream :: receiveBytes(const void* buffer, int length) {
//
//	int numBytesRecieved = 0;
//	try {
//
//	} catch (Poco::Exception& exc) {
//		//Handle your network errors.
//		cerr << "Network error: " << exc.displayText() << endl;
//		//	isOpen = false;
//	}catch (Poco::TimeoutException& exc) {
//		//Handle your network errors.
//		cerr << "Network error: " << exc.displayText() << endl;
//		//	isOpen = false;
//	}
//}

dac_point*  DacEtherdream :: getDacPoint() {
	dac_point* p;
	if(sparePoints.size()==0) {
		p= new dac_point();
		//ofLog(OF_LOG_NOTICE, "new point made");
	} else {
		p = sparePoints.back();
		sparePoints.pop_back();
		
	}
	p->control =
	p->x =
	p->y =
	p->r =
	p->g =
	p->b =
	p->i =
	p->u1 =
	p->u2 = 0;
	return p;
	}

//
//  ofxLaserDacEtherdream.cpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#include "ofxLaserDacEtherdream.h"

using namespace ofxLaser;



void DacEtherdream :: setup(string ip) {
	
	// Etherdreams always talk on port 7765
	Poco::Net::SocketAddress sa(ip, 7765);
	pps = 0;
	pps = newPPS = 30000; // this is always sent on begin
	queuedPPSChangeMessages = 0;
	connected = false;
	
	try {
		Poco::Timespan timeout(5000);
		socket.connect(sa, timeout);
		connected = true;
	} catch(...) {
		ofLog(OF_LOG_ERROR, "DacEtherdream setup failed");
	} 
	if(connected) {
		prepareSent = false;
		beginSent = false;
		startThread();
	}
}

void DacEtherdream :: close() {
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
	
	
	int maxBufferSize = 1000;
	if(bufferedPoints.size()<maxBufferSize) {
		vector<dac_point> sendpoints(points.size());
		for(int i = 0; i<points.size(); i++) {
			dac_point& p1 = sendpoints[i];
			Point p2 = points[i];
			p1.x = ofMap(p2.x,0,800,ETHERDREAM_MIN, ETHERDREAM_MAX);
			p1.y = ofMap(p2.y,800,0,ETHERDREAM_MIN, ETHERDREAM_MAX); // Y is UP in ilda specs
			p1.r = p2.r/255.0f*65535;
			p1.g = p2.g/255.0f*65535;
			p1.b = p2.b/255.0f*65535;
			p1.i = 0;
			p1.u1 = 0;
			p1.u2 = 0;
		}
		addPoints(sendpoints);
		return true;
	} else {
		return false;
	}
}

bool DacEtherdream:: sendPoints(const vector<Point>& points){
    // max half second buffer
    if(bufferedPoints.size()>pps*0.5) {
        return false;
    }
    
    vector<dac_point> sendpoints(points.size());
    for(int i = 0; i<points.size(); i++) {
        dac_point& p1 = sendpoints[i];
        Point p2 = points[i];
        p1.x = ofMap(p2.x,0,800,ETHERDREAM_MIN, ETHERDREAM_MAX);
        p1.y = ofMap(p2.y,800,0,ETHERDREAM_MIN, ETHERDREAM_MAX); // Y is UP in ilda specs
        p1.r = p2.r/255.0f*65535;
        p1.g = p2.g/255.0f*65535;
        p1.b = p2.b/255.0f*65535;
        p1.i = 0;
        p1.u1 = 0;
        p1.u2 = 0;
    }
    addPoints(sendpoints);
    return true;
  
}

bool DacEtherdream :: addPoints(const vector<dac_point> &points ){
	
	if(response.status.playback_state!=PLAYBACK_IDLE) {
		if(lock()) {
			for(int i = 0; i<points.size(); i++) {
				bufferedPoints.push_back(points[i]);
			
			}
			unlock();
		}
	}
	return true;
}

bool DacEtherdream :: addPoint(const dac_point &point ){
	
	if(response.status.playback_state!=PLAYBACK_IDLE) {
		lock();
		bufferedPoints.push_back(point);
		unlock();
	}
	return true;
}
void DacEtherdream :: threadedFunction(){
	
	waitForAck('?');
	
	while(isThreadRunning()) {
		
		
		if((response.status.playback_state == PLAYBACK_IDLE) && (response.status.playback_flags & 0b010)) {
			prepareSent = false;
			beginSent = false;
			sendPrepare();
			waitForAck('p');
			
		}
		
		if((response.status.light_engine_state == LIGHT_ENGINE_READY) && (!prepareSent)) {
			sendPrepare();
			waitForAck('p');
		}
		
		// if we're playing and we have a new point rate, send it!
		if((response.status.playback_state==PLAYBACK_PLAYING) && (newPPS!=pps)) {
			pps = newPPS;
			sendPointRate(pps);
			waitForAck('q');
			queuedPPSChangeMessages++;
		}
		
		// if state is prepared or playing, and we have points in the buffer, then send the points
		if(response.status.playback_state!=PLAYBACK_IDLE) {
			
			if(numPointsToSend>80){
				//check buffer and send the next points
				while(!lock()) {}
				sendData();
				unlock();
				waitForAck('d');
			} else if(isThreadRunning()) {
				sendPing();
				waitForAck('?');
			}
			
		}
		// if state is prepared and we have enough points in the buffer and we haven't already, send begin
		if((response.status.playback_state==PLAYBACK_PREPARED) && (!beginSent)&& (response.status.buffer_fullness>=1000)) {
			sendBegin();
			waitForAck('b');

			
		}
		
		yield();
	}
}

bool DacEtherdream::setPointsPerSecond(uint32_t newpps){
	ofLog(OF_LOG_NOTICE, "setPointsPerSecond " + ofToString(newpps));
	newPPS = newpps;
	if(!beginSent) pps = newPPS; // pps rate will get sent with begin anyway
	
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
	while(waiting) {
		yield();
		usleep(100);

		if(lock()) {
			if(socket.available())
				waiting = false;
			unlock();
		}
		if(!isThreadRunning()) return false;
		
	}
	
	
	int n = 0;
	while (n==0) {
		if(!isThreadRunning()) return false;
		n = socket.receiveBytes(buffer, 22);
		yield();
		usleep(10);
	}
	// TODO - handle incomplete data
	
	//cout << "received " << n << "bytes" <<endl;
	
	if(n==22) {
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
		
		
		//memcpy(&response, &buffer, sizeof(response));
		
		//ofLog(OF_LOG_NOTICE, "response : "+ ofToString(response.response) +  " command : " + ofToString(response.command) );
		
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
		// 10000 : Emergency stop due to loss of Ethernet (not sure how we'd get the message???)
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
		
		numPointsToSend = 1700 - response.status.buffer_fullness;
		if(numPointsToSend<0) numPointsToSend = 0;
		
		if(command =='q') ofLog(OF_LOG_NOTICE,data);
	}
	return true;
	
}
inline void DacEtherdream :: sendBegin(){
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
	int n = socket.sendBytes(&buffer, 7);
	//cout << "sent " << n << " bytes" << endl;
	beginSent = true;
}
inline void DacEtherdream :: sendPrepare(){
	ofLog(OF_LOG_NOTICE, "sendPrepare()");
	int send = 0x70;
	int n = socket.sendBytes(&send,1);
	//cout << "sent " << n << " bytes" << endl;
	prepareSent = true;
}


inline void DacEtherdream :: sendPointRate(uint32_t rate){
	outbuffer[0] = 'q';
	writeUInt32ToBytes(rate, &outbuffer[1]);
	int n = socket.sendBytes(&outbuffer, 5);
	
}

inline void DacEtherdream :: sendData(){
	
	data_command d;
	d.command = 'd';
	// TODO - send more than 80 bytes if we can?
	d.npoints = 80;
	if(numPointsToSend<d.npoints) d.npoints = numPointsToSend;
	if(d.npoints>pps/10) d.npoints = pps/10;
	if(d.npoints<=0) d.npoints = 1;
	numPointsToSend-=d.npoints;
	
	outbuffer[0]= d.command;
	writeUInt16ToBytes(d.npoints, &outbuffer[1]);
	int pos = 3;
	
	dac_point p;
	
	
	for(int i = 0; i<d.npoints; i++) {
		
		if(bufferedPoints.size()>0) {
			p = bufferedPoints[0];
			bufferedPoints.pop_front();
			lastpoint = p;
		} else  {
			// just send some blank points
			//TODO count the blanks!
			p = lastpoint;
			//			p.control = 0;
			//			p.x = 0;
			//			p.y = 0;
			p.i = 0;
			p.r = 0;
			p.g = 0;
			p.b = 0;
			p.u1 = 0;
			p.u2 = 0;
		}
		//		} else {
		//			break;
		//		}
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
	
	
	
	int n = socket.sendBytes(&outbuffer, pos);
	//cout << "sent " << n << " bytes" << endl;
	//cout << "numPointsToSend " << numPointsToSend << endl;
}
void DacEtherdream :: sendPing(){
	char ping = '?';
	socket.sendBytes(&ping, 1);
}
void DacEtherdream :: sendEStop(){
	char ping = '\0';
	socket.sendBytes(&ping, 1);
}
void DacEtherdream :: sendStop(){
	// non-emergency stop
	char ping = 's';
	socket.sendBytes(&ping, 1);
}
void DacEtherdream :: sendClear(){
	char ping = 'c';
	// clear emergency stop
	socket.sendBytes(&ping, 1);
}

uint16_t DacEtherdream :: bytesToUInt16(Byte* byteaddress) {
	return (uint16_t)(*(byteaddress+1)<<8)|*byteaddress;
	
}
uint32_t DacEtherdream :: bytesToUInt32(Byte* byteaddress){
	return (uint32_t)(*(byteaddress+3)<<24)|(*(byteaddress+2)<<16)|(*(byteaddress+1)<<8)|*byteaddress;
	
}
void DacEtherdream :: writeUInt16ToBytes(uint16_t& n, Byte* byteaddress){
	*(byteaddress+1) = n>>8;
	*byteaddress = n&0xff;
}
void DacEtherdream :: writeInt16ToBytes(int16_t& n, Byte* byteaddress){
	*(byteaddress+1) = n>>8;
	*byteaddress = n&0xff;
}
void DacEtherdream :: writeUInt32ToBytes(uint32_t& n, Byte* byteaddress){
	*(byteaddress+3) = (n>>24) & 0xff;
	*(byteaddress+2) = (n>>16) & 0xff;
	*(byteaddress+1) = (n>>8) & 0xff;
	*byteaddress = n&0xff;
	
}

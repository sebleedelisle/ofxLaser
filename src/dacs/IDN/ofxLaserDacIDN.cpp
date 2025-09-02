//
//  ofxLaserDacIDN.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 13/04/2018.
//
//

#include "ofxLaserDacIDN.h"

using namespace ofxLaser;

void DacIDN :: setup(string ip) {
	
	
	pps = 30000;
	newFrameIsBuffered = false;
	lastFrameTime = 0;
	lastFrameDuration = 0;
	connected = false;
	counter = 0;
	
	try {
		
		bool success = true;
		success &= udpConnection.Create();
		success &= udpConnection.Connect(ip.c_str(),7255);
		success &= udpConnection.SetNonBlocking(false);
		if(success) connected = true;
		else {
			ofLog(OF_LOG_ERROR, "DacIDN setup failed");
		}
	} catch(...) {
		ofLog(OF_LOG_ERROR, "DacIDN setup failed");
	}
	
	
	if(connected) {
		
		startThread();
        auto & thread = getNativeThread();
        
#ifndef _MSC_VER
        // only linux and osx
        struct sched_param param;
        param.sched_priority = 89;
        pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &param );
#else
        // windows implementation
        SetThreadPriority( thread.native_handle(), THREAD_PRIORITY_HIGHEST);
#endif 
	}
}

bool DacIDN :: sendFrame(const vector<Point>& points) {
	
	pointsToSend.resize(points.size());
	
	for(size_t i = 0; i<points.size(); i++) {
		IDN_point& p1 = pointsToSend[i];
		Point p2 = points[i];
		p1.x = ofMap(p2.x,0,800,IDN_MIN, IDN_MAX, true);
		p1.y = ofMap(p2.y,800,0,IDN_MIN, IDN_MAX, true); // Y is UP in ilda specs
		p1.r = p2.r;
		p1.g = p2.g;
		p1.b = p2.b;
	}
	
	if(lock()) {
		newFrameIsBuffered = true;
		unlock();
	}
    
    return true;
};

//bool DacIDN :: sendPoints(const vector<Point>& points) {
//	return false;
//};

bool DacIDN :: setPointsPerSecond(uint32_t newpps) {
	pps = newpps;
    return true;
};

void DacIDN :: threadedFunction(){
	
	while(isThreadRunning()) {
		
		// wait for the last frame to finish...
		unsigned long long usWait = lastFrameDuration - (ofGetElapsedTimeMicros() - lastFrameTime);
		if((int)usWait > 0) sleep(usWait/1000);
		
		// and also wait until we have a new frame!
		while(!newFrameIsBuffered) {
			//send void to keep alive?
			// sendVoid();
			sleep(1);
		}
		
		bufferedPoints.resize(pointsToSend.size());
		// now we have new frames so send them!
		while(!lock()); // wait until we have lock
		lastFrameTime = ofGetElapsedTimeMicros();
		lastFrameDuration = (((uint64_t)(pointsToSend.size() - 1)) * 1000000ull) / (uint64_t)pps;
		// copy the points into the buffer
		for(size_t i = 0; i<pointsToSend.size(); i++) {
			bufferedPoints[i] = pointsToSend[i];
		}
		newFrameIsBuffered = false;
		unlock();
		// now it's safe to send the buffered points
		sendFrameToDac();
		yield();
	}
}

void DacIDN :: sendFrameToDac() {
	
	int pointIndex = 0;
	int numPointsToSend = bufferedPoints.size();
	int packetsSent = 0;
	
	// 7 bytes in a point, max bytes is 9000
	// 9000 / 7 is max points, let's say 1200
	int maxPointsPerFragment = 1200;
	
	int fragmentsToSend = ceil((float)numPointsToSend/(float)maxPointsPerFragment);
	
	if(verbose) ofLog(OF_LOG_NOTICE, "FRAGMENTS TO SEND : " + ofToString(fragmentsToSend));
	
	uint64_t time = ofGetElapsedTimeMicros();
	
	for(int i = 0; i<fragmentsToSend; i++){
		
		string output;
		
		
		output.push_back(0x40);
		output.push_back(0x00);
		output.push_back( (uint8_t)(counter>>8 ) );
		output.push_back( (uint8_t) counter);
		
		//PACKET SIZE minus first four bytes - populate it later!
		output.push_back(0x00);
		output.push_back(0x00);
		
		bool sendConfig = (i==0);
		
		// CNL with configuration bit set (0x80 | 0x40) = 0xC0;
		// also set with last fragment. Weird.
		if(sendConfig || (i==fragmentsToSend-1))
			output.push_back(0xc0);
		else
			output.push_back(0x80);
		
		// CHUNK TYPE
		// 0x02 - Frame samples entire frame
		// 0x03 - Frame samples first fragment
		// 0xC0 - Frame samples sequel fragment
		if(fragmentsToSend==1)
			output.push_back(0x02);
		else if(i==0)
			output.push_back(0x03);
		else
			output.push_back(0xc0);
		
		// TIME STAMP
		//ofGetSystemTime();
		output.push_back((uint8_t) ((time+i) >> 24)) ;
		output.push_back((uint8_t) ((time+i) >> 16)) ;
		output.push_back((uint8_t) ((time+i) >> 8)) ;
		output.push_back((uint8_t) (time+i)) ;
		
		
		if(sendConfig) {
			// CHANNEL CONFIG
			// NUMBER OF CONFIG WORDS
			output.push_back(0x04);
			// SET ROUTING FLAG
			output.push_back(0x01);
			// ID (always 0);
			output.push_back(0x00);
			// DISCRETE GRAPHICS MODE
			output.push_back(0x02);
			
			// CONF 1 X
			output.push_back(0x42);
			output.push_back(0x00);
			
			// 16 BIT PRECISION
			output.push_back(0x40);
			output.push_back(0x10);
			
			// CONF 2 Y
			output.push_back(0x42);
			output.push_back(0x10);
			
			// 16 BIT PRECISION
			output.push_back(0x40);
			output.push_back(0x10);
			
			// CONF 3 RED 638nm
			output.push_back(0x52);
			output.push_back(0x7e);
			
			// CONF 4 GREEN 532nm
			output.push_back(0x52);
			output.push_back(0x14);
			
			// CONF 5 BLUE
			output.push_back(0x51);
			output.push_back(0xcc);
			
			// Conf 6... blank zeros???
			output.push_back(0x00);
			output.push_back(0x00);
		
			
			
			// Data header
			// Flags - if bit 1 is set then frame is played once, otherwise it repeats
			output.push_back(0x01);
			
			// THIS IS WHAT SETS THE POINT SPEED ------------------------
			// Then it's the frame duration in microseconds, should be number of points/point rate
			int framemicros = (((uint64_t)(numPointsToSend - 1)) * 1000000ull) / (uint64_t)pps;
			output.push_back((uint8_t) (framemicros >> 16));
			output.push_back((uint8_t) (framemicros >> 8));
			output.push_back((uint8_t) (framemicros));
		}
		
		//DATA START!
		// now it's points! 7 bytes per point :
		// XXYYRGB
		int lastPoint = std::min(maxPointsPerFragment*(i+1), numPointsToSend);
		for(; pointIndex<lastPoint ; pointIndex++) {
			IDN_point &point = bufferedPoints[pointIndex];
			point.getSerialised();
			output.append(point.serialized, 7);
			
		}
		
		
		int messagesize = output.size()-4;
		output[4] = (uint8_t)(messagesize>>8);
		output[5] = (uint8_t)messagesize;
		if(verbose) {
			ofLog(OF_LOG_NOTICE, ofToString(messagesize));
		
			
			//cout << hex << ((int)(counter>>8 ))  << " " << ((int)(uint8)(counter &0xff ))  << endl;
			
			
			
			//for (int i = 0; i<output.size() ; i++) {
			for (int j = 0; j<12 ; j++) {

				cout << std::hex << std::setfill('0') << std::setw(2) << (int)(uint8_t)(output[j]) << " ";
				if((j<36) && (j%4==3)) cout <<endl;
				else if((j>=36) && ((j-36)%7==6)) cout <<endl;
			}
		//		//cout << hex <<  ntohs(hello) << endl;
		}
		counter ++;
		
		udpConnection.Send(output.c_str(),output.length());
		
		if(verbose) ofLog(OF_LOG_NOTICE, ofToString(output.length()));
		if(verbose) cout <<endl;
		
		
	}
	
}

void DacIDN :: close() {
	udpConnection.Close();
}

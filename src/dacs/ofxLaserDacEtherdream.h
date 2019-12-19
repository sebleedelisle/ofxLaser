//
//  ofxLaserDacEtherdream.hpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//


#pragma once
#include "ofxLaserDacBase.h"

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/NetException.h"

#ifdef TARGET_WIN32
#include <Windows.h>
#endif


#define LIGHT_ENGINE_READY    0
#define LIGHT_ENGINE_WARMUP	  1
#define LIGHT_ENGINE_COOLDOWN 2
#define LIGHT_ENGINE_ESTOP    3

#define PLAYBACK_IDLE 0
#define PLAYBACK_PREPARED 1
#define PLAYBACK_PLAYING 2

#define ETHERDREAM_MIN -32768 
#define ETHERDREAM_MAX 32767

struct dac_status {
	uint8_t protocol;
	uint8_t light_engine_state;
	uint8_t playback_state;
	uint8_t source;
	uint16_t light_engine_flags;
	uint16_t playback_flags;
	uint16_t source_flags;
	uint16_t buffer_fullness;
	uint32_t point_rate;
	uint32_t point_count;
};


struct dac_response {
	uint8_t response;
	uint8_t command;
	struct dac_status status;
};

struct begin_command {
	uint8_t command; /* 'b' (0x62) */
	uint16_t low_water_mark;
	uint32_t point_rate;
};


struct dac_point {
//	public:
//	dac_point() {
//		dac_point::createCount++;
//	}
//	~dac_point() {
//		destroyCount++;
//	}
	
	
	uint16_t control;
	int16_t x;
	int16_t y;
	uint16_t r;
	uint16_t g;
	uint16_t b;
	uint16_t i;
	uint16_t u1;
	uint16_t u2;

//	static int createCount;
//	static int destroyCount;
	
};





namespace ofxLaser {

	class DacEtherdream : public DacBase, ofThread {
	
	public:
		DacEtherdream();
		~DacEtherdream();
		
		// DacBase functions
		bool sendFrame(const vector<Point>& points);
        bool sendPoints(const vector<Point>& points);
		bool setPointsPerSecond(uint32_t newpps);
		string getLabel();
		ofColor getStatusColour();
		const vector<ofAbstractParameter*>& getDisplayData();
		
		void setup(string ip);
		//bool addPoints(const vector<dac_point> &points );
		bool addPoint(const dac_point &point );
		void close();
		
		void reset(); 
        
        //output the data that we just sent
        void logData();
		
		// simple object pooling system
		dac_point* getDacPoint();
		
		ofParameter<int> pointBufferDisplay;
		ofParameter<int> latencyDisplay;
		ofParameter<int> reconnectCount;
		uint64_t lastMessageTimeMicros;
		
        
		// the maximum number of points the etherdream can hold.
		// in etherdream v1 it's 1799, but you may want it to be lower
		// for lower latency
		int dacBufferSize;
		// the minimum number of points in the buffer before the etherdream
		// starts playing.
		int pointsToSendBeforePlaying;

		vector<dac_point> framePoints;
		
	private:
		void threadedFunction();

		inline bool sendBegin();
		inline bool sendPrepare();
		inline bool sendData();
		inline bool sendPing();
		bool sendEStop();
		bool sendStop();
		bool sendClear();
		inline bool sendPointRate(uint32_t rate);
		inline bool waitForAck(char command);
		bool sendBytes(const void* buffer, int length);
		
		dac_point lastpoint;
		dac_point sendpoint;
		
		uint8_t buffer[1024];
		uint8_t outbuffer[100000];
        int numBytesSent;
		
		Poco::Net::StreamSocket socket;
		
		string light_engine_states[4] = {"ready", "warmup", "cooldown", "emergency stop"};
		string playback_states[3] = {"idle", "prepared", "playing"};
		
		dac_response response;
		int latencyMicros = 0;
		int prepareSendCount = 0;
        uint64_t startTime; // to measure latency
		bool beginSent;
		
		string ipaddress; 
		
		deque<dac_point*> bufferedPoints;
		vector<dac_point*> sparePoints;
		int numPointsToSend;
		uint32_t pps, newPPS;
		int queuedPPSChangeMessages;
		bool connected; 
		//bool replayFrames = true;
		//bool isReplaying = false;
		bool newFrame = false; 
		bool frameMode = true;
		bool verbose = false;
		  
		
	};

}

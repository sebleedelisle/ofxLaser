//
//  ofxLaserDacEtherdream.hpp
//  ofxLaser
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
		bool sendFrame(const vector<Point>& points) override;
        bool sendPoints(const vector<Point>& points) override;
		bool setPointsPerSecond(uint32_t newpps) override;
		string getId() override;
		int getStatus() override;
		const vector<ofAbstractParameter*>& getDisplayData() override;
		
		void setup(string id, string ip);
        
        OF_DEPRECATED_MSG("DACs are no longer set up in code, do it within the app instead",  bool setup(string ip));
       
		bool addPoint(const dac_point &point );
		void closeWhileRunning();
		void close() override;
	
		
		void reset() override; 
        
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
        
        
        // data conversion utilities - should probably go somewhere else
        
        static uint16_t bytesToUInt16(unsigned char* byteaddress) {
            return (uint16_t)(*(byteaddress+1)<<8)|*byteaddress;
            
        }
        static uint16_t bytesToInt16(unsigned char* byteaddress) {
            uint16_t i = *(signed char *)(byteaddress);
            i *= 1 << CHAR_BIT;
            i |= (*byteaddress+1);
            return i;
            
        }
        static uint32_t bytesToUInt32(unsigned char* byteaddress){
            return (uint32_t)(*(byteaddress+3)<<24)|(*(byteaddress+2)<<16)|(*(byteaddress+1)<<8)|*byteaddress;
            
        }
        static void writeUInt16ToBytes(uint16_t& n, unsigned char* byteaddress){
            *(byteaddress+1) = n>>8;
            *byteaddress = n&0xff;
        }
        static void writeInt16ToBytes(int16_t& n, unsigned char* byteaddress){
            *(byteaddress+1) = n>>8;
            *byteaddress = n&0xff;
        }
        static void writeUInt32ToBytes(uint32_t& n, unsigned char* byteaddress){
            *(byteaddress+3) = (n>>24) & 0xff;
            *(byteaddress+2) = (n>>16) & 0xff;
            *(byteaddress+1) = (n>>8) & 0xff;
            *byteaddress = n&0xff;
            
        }
        
		
	private:
		void threadedFunction() override;

		inline bool sendBegin();
		inline bool sendPrepare();
		inline bool sendData();
		inline bool sendPing();
		bool sendEStop();
		bool sendStop();
		bool sendClear();
		inline bool sendPointRate(uint32_t rate);
		inline bool waitForAck(char command);
		bool sendBytes(const uint8_t* buffer, int length);
		
		dac_point lastpoint;
		dac_point sendpoint;
		
		uint8_t buffer[1024];
		uint8_t outbuffer[100000]; // TODO is this enough? is it ever checked ? 
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
        string id; 
		
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

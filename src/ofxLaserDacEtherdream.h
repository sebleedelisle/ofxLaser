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

#define Byte unsigned char

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
	uint16_t control;
	int16_t x;
	int16_t y;
	uint16_t r;
	uint16_t g;
	uint16_t b;
	uint16_t i;
	uint16_t u1;
	uint16_t u2;
};

struct data_command {
	uint8_t command; /* ‚Äòd‚Äô (0x64) */
	uint16_t npoints;
	struct dac_point data[];
};





namespace ofxLaser {

	class DacEtherdream : public DacBase, ofThread {
	
	public:
		void setup(string ip);
		bool sendFrame(const vector<Point>& points);
        bool sendPoints(const vector<Point>& points);
		bool setPointsPerSecond(uint32_t newpps);
		bool addPoints(const vector<dac_point> &points );
		bool addPoint(const dac_point &point );
		void close();
		
	private:
		void threadedFunction();

		inline void sendBegin();
		inline void sendPrepare();
		inline void sendData();
		inline void sendPing();
		void sendEStop();
		void sendStop();
		void sendClear();
		inline void sendPointRate(uint32_t rate);
		inline bool waitForAck(char command);
		
		
		uint16_t bytesToUInt16(unsigned char* byteaddress);
		uint32_t bytesToUInt32(unsigned char* byteaddress);
		void writeUInt16ToBytes(uint16_t& n, unsigned char* byteaddress);
		void writeInt16ToBytes(int16_t& n, unsigned char* byteaddress);
		void writeUInt32ToBytes(uint32_t& n, unsigned char* byteaddress);

		dac_point lastpoint;
		
		Byte buffer[1024];
		Byte outbuffer[100000];
		
		Poco::Net::StreamSocket socket;
		
		string light_engine_states[4] = {"ready", "warmup", "cooldown", "emergency stop"};
		string playback_states[3] = {"idle", "prepared", "playing"};
		
		dac_response response;
		bool prepareSent;
		bool beginSent;
		
		deque<dac_point> bufferedPoints;
		int numPointsToSend;
		uint32_t pps, newPPS;
		int queuedPPSChangeMessages;
		bool connected; 
		

	};

}

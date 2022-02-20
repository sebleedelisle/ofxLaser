//
//  ofxLaserDacEtherDream.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#pragma once
#include "ofxLaserDacBase.h"
#include "ofxLaserDacEtherDreamCommand.h"
#include "ofxLaserDacEtherDreamDacPoint.h"
#include "ofxLaserDacEtherDreamData.h"
#include "ofxLaserDacEtherDreamResponse.h"
#include "ByteStreamUtils.h"

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/NetException.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif

#define ETHERDREAM_MIN -32768 
#define ETHERDREAM_MAX 32767

struct DacStateAtTime {
    uint64_t timeMicros = 0;
    uint32_t buffer = 0;
    uint32_t pointRate = 0;
    uint64_t roundTripTime = 0;
    bool playing = false;
};

class DacStateRecorder {
    public :
    DacStateRecorder() {
    }
    void recordState(uint64_t timemicros, int playbackstate, int bufferfullness, int roundtriptime, int numpointssent, int pointrate) {
        if(stateHistory.size()>=1000) {
            DacStateAtTime* spare = stateHistory.front();
            stateHistory.pop_front();
            delete spare;
        }
        stateHistory.push_back(new DacStateAtTime());
        DacStateAtTime& bufferState = *stateHistory.back();
        bufferState.timeMicros = timemicros;
        bufferState.buffer = bufferfullness;
        bufferState.playing = playbackstate;
        bufferState.pointRate = pointrate;
        bufferState.roundTripTime = roundtriptime;
        
        
    }
    
    const vector<DacStateAtTime*>& getStateHistoryForTimePeriod(uint64_t starttimemicros, uint64_t endtimemicros) {
        // assumes that history date is sorted by time
        stateHistoryForTimePeriod.clear();
        
        for(size_t i = 0; i<stateHistory.size(); i++) {
            DacStateAtTime* stateAtTime = stateHistory[i];
            bool firstfound = true;
            if((stateAtTime->timeMicros >= starttimemicros) && (stateAtTime->timeMicros <= endtimemicros)) {
                if(firstfound) {
                    if(i>0) stateHistoryForTimePeriod.push_back(stateHistory[i-1]);
                    firstfound = false;
                }
                stateHistoryForTimePeriod.push_back(stateAtTime);
            }
            
        }
        
        return stateHistoryForTimePeriod;
        
    }
    
    void getBufferSizeValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) {
        
        getStateHistoryForTimePeriod(starttimemicros, endtimemicros); // updates stateHistoryForTimePeriod
        
        int bufferIndex = 0;
        uint64_t visibledurationmicros = endtimemicros-starttimemicros;
        if(stateHistoryForTimePeriod.size()>0) {
            // get the first buffer index for the start time
                      
            DacStateAtTime* bufferstate = stateHistoryForTimePeriod[bufferIndex];
            float buffersize =  (float)bufferstate->buffer;;
            
            for (int i =0; i<numvalues; i++) {
                uint64_t timeMicros = visibledurationmicros;
                timeMicros *= i;
                timeMicros /= numvalues;
                timeMicros+=starttimemicros;
               
                
                while((bufferIndex+1<stateHistoryForTimePeriod.size()) && stateHistoryForTimePeriod[bufferIndex+1]->timeMicros < timeMicros) {
                    
                    bufferIndex++;
                    bufferstate = (stateHistoryForTimePeriod[bufferIndex]);
                    //buffersize = 5000;
                    buffersize = (float)bufferstate->buffer;
                }
                
                //buffersize = (float)bufferstate->buffer;
                uint64_t pointsused = (timeMicros - bufferstate->timeMicros) * bufferstate->pointRate / 1000000.0f;
                
                values[i] = buffersize - pointsused;
                
            }
        }
        
        
    }
    
    void getLatencyValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) {
        
        getStateHistoryForTimePeriod(starttimemicros, endtimemicros); // updates stateHistoryForTimePeriod
        
        int bufferIndex = 0;
        uint64_t visibledurationmicros = endtimemicros-starttimemicros;
        if(stateHistoryForTimePeriod.size()>0) {
            // get the first buffer index for the start time
            float latency = stateHistoryForTimePeriod[bufferIndex]->roundTripTime/1000.0f;
            
            DacStateAtTime* bufferstate = stateHistoryForTimePeriod[bufferIndex];
           
            
            for (int i =0; i<numvalues; i++) {
                uint64_t timeMicros = visibledurationmicros;
                timeMicros *= i;
                timeMicros /= numvalues;
                timeMicros+=starttimemicros;
               
                
                while((bufferIndex+1<stateHistoryForTimePeriod.size()) && stateHistoryForTimePeriod[bufferIndex+1]->timeMicros < timeMicros) {
                    
                    bufferIndex++;
                    bufferstate = (stateHistoryForTimePeriod[bufferIndex]);
                    //buffersize = 5000;
                    latency = (float)bufferstate->roundTripTime/1000.0f;
                }
                
                
                values[i] = latency;
                
            }
        }
        
    }
    
    
    deque<DacStateAtTime*> stateHistory;
    vector<DacStateAtTime*> stateHistoryForTimePeriod;
    float values[10000]; // used to store plot data, temporary storage
   
    
};



namespace ofxLaser {

	class DacEtherDream : public DacBase, ofThread {
	
	public:
		DacEtherDream();
		~DacEtherDream();
    
        int getMaxPointBufferSize();
		
		// DacBase functions
		bool sendFrame(const vector<Point>& points) override;
        bool sendPoints(const vector<Point>& points) override;
		bool setPointsPerSecond(uint32_t newpps) override;
		string getId() override;
		int getStatus() override;
		const vector<ofAbstractParameter*>& getDisplayData() override;
		
		void setup(string id, string ip, EtherDreamData& ed);
        
        OF_DEPRECATED_MSG("DACs are no longer set up in code, do it within the app instead",  bool setup(string ip));
       
		bool addPoint(const EtherDreamDacPoint &point );
		void closeWhileRunning();
		void close() override;
	
		
		void reset() override; 
        
        //output the data that we just sent
        void logData();
		
		// simple object pooling system
		EtherDreamDacPoint* getDacPoint();
		
		ofParameter<int> pointBufferDisplay;
		ofParameter<int> latencyDisplay;
		ofParameter<int> reconnectCount;
		uint64_t lastMessageTimeMicros;
        EtherDreamData etherDreamData;
        
		vector<EtherDreamDacPoint> framePoints;
        
        
        DacStateRecorder stateRecorder; 
//        int latencyHistorySize = 200;
//        int latencyHistoryOffset = 0;
//        float latencyHistory[200];
          int latencyMicros = 0;
//
//        deque<DacStateAtTime*> bufferStateHistory;
//
        ofParameter<int>pointBufferMinParam;
        
	private:
        
        void threadedFunction() override;
        //void addLatencyRecording(int latency);
      
        inline bool sendBegin();
        inline bool sendPrepare();
        inline bool sendData(int minPointsToSend, int maxPointsToSend);
        inline bool sendPing();
        bool sendEStop();
        bool sendStop();
        bool sendClear();
        inline bool sendPointRate(uint32_t rate);
        inline bool waitForAck(char command);
        bool sendCommand(DacEtherDreamCommand& command);
        
       
        
        // the maximum number of points the etherdream can hold.
        // in etherdream v1 it's 1799, but you may want it to be lower
        // for lower latency
        volatile int pointBufferCapacity;
        // the minimum number of points in the buffer before the etherdream
        // starts playing / we send a new frame.
        volatile int pointBufferMin;
        

      
		
		EtherDreamDacPoint lastpoint;
		EtherDreamDacPoint sendpoint;
		
		uint8_t inBuffer[1024];
		//uint8_t outBuffer[100000]; //  is this enough? It is checked in send data
        
        DacEtherDreamCommand dacCommand;
        
        int numBytesSent;
		
		Poco::Net::StreamSocket socket;
		
		
        DacEtherDreamResponse response;
	
		int prepareSendCount = 0;
        uint64_t lastCommandSendTime = 0; // to measure latency
		bool beginSent = false;
		
		string ipaddress;
        string id; 
		
		deque<EtherDreamDacPoint*> bufferedPoints;
		vector<EtherDreamDacPoint*> sparePoints;
		//int numPointsToSend;
		uint32_t pps, newPPS;
		int queuedPPSChangeMessages;
		bool connected; 
		//bool replayFrames = true;
		//bool isReplaying = false;
		bool newFrame = false; 
		bool frameMode = true;
		bool verbose = false;
         
        uint64_t lastAckTime = 0;
        uint64_t lastDataSentTime = 0;
		
	};

}

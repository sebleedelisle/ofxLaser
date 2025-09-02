//
//  ofxLaserDacEtherDream.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#pragma once
#include "ofxLaserDacNetworkBaseThreaded.h"

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
#define ETHERDREAM_MAX_PACKET_SIZE 65535
#define ETHERDREAM_MAX_POINTS_IN_PACKET 3640



namespace ofxLaser {


class DacEtherDream : public DacNetworkBaseThreaded {

public:
    DacEtherDream();
    ~DacEtherDream();

    // DacBase functions
    bool setPointsPerSecond(uint32_t newpps) override;

    virtual string getType() override { return "EtherDream"; };
    
    string getRawId() override;
    string getEdId(); 
    int getStatus() override;
    const vector<ofAbstractParameter*>& getDisplayData() override;
   
    void setup(string id, string ip, EtherDreamData& ed);
    
    void closeWhileRunning();
    void close() override;
    void reset() override;
   
    int getDacTotalPointBufferCapacity() override;
    // estimate the current dac buffer fullness based on the last time points were sent
    virtual int calculateBufferFullnessByTimeSent() override;
    // estimate the current dac buffer fullness based on the last time points were acknowledged
    virtual int calculateBufferFullnessByTimeAcked() override;
    
    virtual int getMinimumDacBufferFullnessForLatency() override;
    
//    int calculateMinimumPointsToSend() {
//
//        int maxEstimatedBufferFullness = calculateBufferFullnessByTimeAcked();
//        //int minEstimatedBufferFullness = calculateBufferFullnessByTimeSent();
//        
//        
//        // get min buffer fullness required to maintain latency
//        int minDacBufferFullness = maxLatencyMS * pps / 1000;
//        
//        // because the newest etherdreams use DMA transfer, they
//        // always need at least 256 bytes in the buffer otherwise
//        // they report a buffer under-run
//        if(etherDreamData.softwareRevision>=30) {
//            minDacBufferFullness = MAX(minDacBufferFullness, 256);
//        }
//        if(minDacBufferFullness>getDacTotalPointBufferCapacity()) {
//            minDacBufferFullness = getDacTotalPointBufferCapacity();
//        }
//        
//        int numPointsAvailable =  bufferedPoints.size();
//        int minPointsToAdd = MAX(0, minDacBufferFullness - maxEstimatedBufferFullness - numPointsAvailable);
//        int maxPointsToAdd = MAX(0, dacTotalPointBufferCapacity - maxEstimatedBufferFullness);
//        
//        
//    }
        
   
    //output the data that we just sent to the console - for debugging
    void logData();
    
    // information about the device, IP address, MAC address, version number etc
    EtherDreamData etherDreamData;
    
    EtherDreamData getEtherDreamData(); 
    int getLastReportedBufferFullness();
    string getEtherDreamStateString();
    
    string getHardwareName() {
        if(!isMercury) {
            string name = "Ether Dream v";
            if(versionNumber == 3) return name +"3/4";
            else return name + ofToString(versionNumber);

        } else {
            return "Mercury";
        }
        
    }
    
    
protected:
  
    // ofThread functions
    void threadedFunction() override;
  
    bool sendBegin();
    bool sendPrepare();
    bool sendPointsToDac();
    bool sendPing();
    bool sendEStop();
    bool sendStop();
    bool sendClear();
    bool sendPointRate(uint32_t rate);
    bool waitForAck(char command);
    bool sendCommand(DacEtherDreamCommand& command);
    
    // updates the frame buffer with new frames from the threadchannel,
    // adds frames to the frame queue until we have minPointsToQueue
    // and up to maxPointsToSend
    
    
    // the maximum number of points the etherdream can hold.
    // in etherdream v1 it's 1799, higher for later models.
    std::atomic<int> dacTotalPointBufferCapacity;
    
    // remember the last point sent (so we know where the scanners are in
    // case of a hold up)
    EtherDreamDacPoint lastPointSent;
    EtherDreamDacPoint sendPoint; // I think used as a spare?
    
    uint8_t inBuffer[1024]; // to receive data from the Ether Dream
    
    // the response from the last send. Used to keep track of play state,
   
    DacEtherDreamResponse responseThreaded;
    std::atomic<uint64_t> lastResponseTimeMS;
    
    // this is read in the getState message. 
    ofThreadChannel<string> networkErrors;
    string lastNetworkError = "";
    
    //ofThreadChannel<DacEtherDreamResponse> responseChannel;
    std::atomic<int> lastReportedPlaybackState = ETHERDREAM_PLAYBACK_IDLE;
    string playbackStateString;
    // stores command data, is replaced every time a command is sent
    // (it also manages the byte serialization process)
    // this could potentially be converted into a vector so as to
    // keep track of the command history
    DacEtherDreamCommand dacCommand;
    
    Poco::Net::StreamSocket socket;
    
    int prepareSendCount = 0;
    // last time any command was sent
    uint64_t lastCommandSendTime = 0; // to measure round trip time

   
    bool beginSent = false;
    int blankPointsToSend = 0; // System for making sure points are blank after reset
    int numBlankPointsToSendAfterReset = 10; 
    
    string ipAddress;
    string id;
    
    //vector<EtherDreamDacPoint*> sparePoints;

    
    int queuedPPSChangeMessages;
    std::atomic<bool> networkConnected;
    std::atomic<char> lastNackReceived = '\0';
   // int maxLatencyMS;

   // bool newFrame = false;


    int versionNumber;
    string versionString;
    
    bool isMercury = false; 
     
    
};

}

//
//  ofxLaserDacEtherDream.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#pragma once
#include "ofxLaserDacBaseThreaded.h"

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



namespace ofxLaser {


class DacEtherDream : public DacBaseThreaded {

public:
    DacEtherDream();
    ~DacEtherDream();

    // DacBase functions
    bool setPointsPerSecond(uint32_t newpps) override;
    
    string getId() override;
    int getStatus() override;
    const vector<ofAbstractParameter*>& getDisplayData() override;
   
    void setup(string id, string ip, EtherDreamData& ed);
    
    void closeWhileRunning();
    void close() override;
    void reset() override;
   
    int getMaxPointBufferSize();
    // estimate the current dac buffer fullness based on the last time points were sent
    virtual int calculateBufferSizeByTimeSent() override;
    // estimate the current dac buffer fullness based on the last time points were acknowledged
    virtual int calculateBufferSizeByTimeAcked() override;
   
    //output the data that we just sent to the console - for debugging
    void logData();
    
    // information about the device, IP address, MAC address, version number etc
    EtherDreamData etherDreamData;
    
protected:
  
    // ofThread functions
    void threadedFunction() override;
  
    inline bool sendBegin();
    inline bool sendPrepare();
    inline bool sendPointsToDac();
    inline bool sendPing();
    bool sendEStop();
    bool sendStop();
    bool sendClear();
    inline bool sendPointRate(uint32_t rate);
    inline bool waitForAck(char command);
    bool sendCommand(DacEtherDreamCommand& command);
    
    // updates the frame buffer with new frames from the threadchannel,
    // adds frames to the frame queue until we have minPointsToQueue
    // and up to maxPointsToSend
    
    
    // the maximum number of points the etherdream can hold.
    // in etherdream v1 it's 1799, higher for later models.
    int pointBufferCapacity;
    
    // remember the last point sent (so we know where the scanners are in
    // case of a hold up)
    EtherDreamDacPoint lastPointSent;
    EtherDreamDacPoint sendPoint; // I think used as a spare?
    
    uint8_t inBuffer[1024]; // to receive data from the Ether Dream
    
    // the response from the last send. Used to keep track of play state,
    // TODO - playbackstate should probably be stored somewhere else.
    DacEtherDreamResponse response;

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
    
    string ipAddress;
    string id;
    
    //vector<EtherDreamDacPoint*> sparePoints;

    
    int queuedPPSChangeMessages;
    bool connected;
   // int maxLatencyMS;

   // bool newFrame = false;

    bool verbose = false;
     
    
};

}

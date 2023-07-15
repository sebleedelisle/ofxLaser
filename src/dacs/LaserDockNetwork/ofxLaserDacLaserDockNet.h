//
//  ofxLaserDacLaserDockNet.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//
//

#pragma once
#include "ofxLaserDacBaseThreaded.h"

#include "ofxLaserDacLaserDockNetCommand.h"
#include "ofxLaserDacLaserDockNetDacPoint.h"
#include "ofxLaserDacLaserDockNetConsts.h"
#include "ofxLaserDacLaserDockNetResponse.h"
#include "ByteStreamUtils.h"

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/NetException.h"

#include "Poco/Net/DatagramSocket.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif

#define LaserDockNet_MIN 0
#define LaserDockNet_MAX 0xFFF

//#define CMD_GET_RINGBUFFER_EMPTY_SAMPLE_COUNT 0x8a
//#define LASERCUBE_CMD_SET_ILDA_RATE 0x82

namespace ofxLaser {


class DacLaserDockNet : public DacBaseThreaded {

public:
    DacLaserDockNet();
    ~DacLaserDockNet();

    // DacBase functions
    bool setPointsPerSecond(uint32_t newpps) override;
    
    string getId() override;
    int getStatus() override;
    const vector<ofAbstractParameter*>& getDisplayData() override;
   
    void setup(string id, string ip, DacLaserDockNetStatus status);
    
    void closeWhileRunning();
    void close() override;
    void reset() override;
    
    bool checkDataPortIncoming(); 
   
    int getMaxPointBufferSize() override;
//    // estimate the current dac buffer fullness based on the last time points were sent
//    virtual int calculateBufferSizeByTimeSent() override;
//    // estimate the current dac buffer fullness based on the last time points were acknowledged
//    virtual int calculateBufferSizeByTimeAcked() override;
   
    //output the data that we just sent to the console - for debugging
    void logData();
    
    // information about the device, IP address, MAC address, version number etc
    DacLaserDockNetStatus laserDockNetData;
    
    
    
    
protected:
  
    // ofThread functions
    void threadedFunction() override;
  
    inline bool sendPointsToDac();

    inline bool sendPointRate(uint32_t rate);

    bool sendData(DacLaserDockNetCommand& command);
    bool sendCommand(DacLaserDockNetCommand& command);
    
    // updates the frame buffer with new frames from the threadchannel,
    // adds frames to the frame queue until we have minPointsToQueue
    // and up to maxPointsToSend
    
    
    // the maximum number of points the LaserDockNet can hold.
    // in LaserDockNet v1 it's 1799, higher for later models.
    int pointBufferCapacity;
    
    uint8_t packetNumber;
    
    // remember the last point sent (so we know where the scanners are in
    // case of a hold up)
    LaserDockNetDacPoint lastPointSent;
    LaserDockNetDacPoint sendPoint; // I think used as a spare?
    
    uint8_t inBuffer[1024]; // to receive data from the Ether Dream
    
    // the response from the last send. Used to keep track of buffer size
    DacLaserDockNetResponse response;

    // stores command data, is replaced every time a command is sent
    // (it also manages the byte serialization process)
    // this could potentially be converted into a vector so as to
    // keep track of the command history
    DacLaserDockNetCommand dacCommand;
    
    Poco::Net::DatagramSocket dataUdpSocket;
    Poco::Net::DatagramSocket commandUdpSocket;
    
    // last time any command was sent
    uint64_t lastCommandSendTime = 0; // to measure round trip time

   
    int blankPointsToSend = 0; // System for making sure points are blank after reset
    int numBlankPointsToSendAfterReset = 10; 
    
    string ipAddress;
    string id;
    
    
    
   // int queuedPPSChangeMessages;
    bool connected;
   
    string versionString;
     
    
};

}

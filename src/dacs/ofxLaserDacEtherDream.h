//
//  ofxLaserDacEtherDream.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#pragma once
#include "ofxLaserDacBase.h"
#include "ofxLaserDacFrame.h"
#include "ofxLaserDacEtherDreamCommand.h"
#include "ofxLaserDacEtherDreamDacPoint.h"
#include "ofxLaserPointFactory.h"
#include "ofxLaserDacEtherDreamData.h"
#include "ofxLaserDacEtherDreamResponse.h"
#include "ofxLaserDacStateRecorder.h"
#include "ofxLaserDacFrameInfoRecorder.h"
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


class DacEtherDream : public DacBase, ofThread {

public:
    DacEtherDream();
    ~DacEtherDream();

    // DacBase functions
    bool sendFrame(const vector<Point>& points) override;
    bool sendPoints(const vector<Point>& points) override;
    bool setPointsPerSecond(uint32_t newpps) override;
    string getId() override;
    int getStatus() override;
    const vector<ofAbstractParameter*>& getDisplayData() override;
    
    void setup(string id, string ip, EtherDreamData& ed);
    
    OF_DEPRECATED_MSG("DACs are no longer set up in code, do it within the app instead",  bool setup(string ip));
   
    bool addPoint(const ofxLaser::Point& point );
    void closeWhileRunning();
    void close() override;
    void reset() override;
    bool isReadyForFrame(int maxLatencyMS) override; 
    int getMaxPointBufferSize();
    int getCurrentBufferFullness();
    //output the data that we just sent
    void logData();
    
    
    ofParameter<int> pointBufferDisplay;
    ofParameter<int> latencyDisplay;
    ofParameter<int> reconnectCount;
    uint64_t lastMessageTimeMicros;
    EtherDreamData etherDreamData;
    
    //DacEtherDreamFrame frame;
   // vector<EtherDreamDacPoint> framePoints;
    ofThreadChannel<DacFrame*> frameThreadChannel;
    deque<DacFrame*> bufferedFrames;
    deque<DacFrame*> queuedFrames;
    
    DacStateRecorder stateRecorder;
    DacFrameInfoRecorder frameRecorder; 
    
   // ofParameter<int>pointBufferMinParam;
    
private:
    
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
    int getNumPointsInQueuedFrames();
    int getNumPointsInBufferedFrames();
    int getNumPointsInAllBuffers();

    
    // the maximum number of points the etherdream can hold.
    // in etherdream v1 it's 1799, higher for later models.
    int pointBufferCapacity;
    // the minimum number of points in the buffer before the etherdream
    // starts playing / we send a new frame.
    //int pointBufferMin;
    
    // remember the last point sent (so we know where the mirrors are in
    // case of a hold up)
    EtherDreamDacPoint lastPointSent;
    EtherDreamDacPoint sendPoint; // I think used as a spare?
    
    uint8_t inBuffer[1024]; // to receive data from the Ether Dream
    DacEtherDreamResponse response;
    
    DacEtherDreamCommand dacCommand;
    
    int numBytesSent;
    
    Poco::Net::StreamSocket socket;
    
    int prepareSendCount = 0;
    uint64_t lastCommandSendTime = 0; // to measure round trip time
    uint64_t lastAckTime = 0;
    uint64_t lastDataSentTime = 0;
    int roundTripTimeMicros = 0;
 

    bool beginSent = false;
    
    string ipAddress;
    string id;
    
    deque<ofxLaser::Point*> bufferedPoints;
    //vector<EtherDreamDacPoint*> sparePoints;

    uint32_t pps, newPPS;
    int queuedPPSChangeMessages;
    bool connected;
   // int maxLatencyMS;

   // bool newFrame = false;

    bool verbose = false;
     
    
};

}

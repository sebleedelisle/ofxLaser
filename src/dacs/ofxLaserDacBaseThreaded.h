//
//  Header.h
//
//
//  Created by Seb Lee-Delisle on 11/05/2022.
//

#pragma once
#include "ofxLaserDacBase.h"
#include "ofxLaserDacFrame.h"
#include "ofxLaserDacStateRecorder.h"
#include "ofxLaserDacFrameInfoRecorder.h"
#include "ofxLaserPointFactory.h"
#include "ofMain.h"

namespace ofxLaser {

class DacBaseThreaded : public DacBase, public ofThread {
    
    public :
    
    // DacBase
    virtual bool sendFrame(const vector<Point>& points) override;
    virtual bool sendPoints(const vector<Point>& points) override;
    virtual bool setColourShift(float shiftSeconds) override;
    
    virtual string getId() override = 0;
    virtual int getStatus() override = 0;
    virtual bool setPointsPerSecond(uint32_t pps) override = 0;
    virtual int getMaxPointBufferSize() =0;
    
    virtual void reset() override = 0;
    virtual void close() override = 0;
    void cleanUpFramesAndPoints(); 
    
    bool isReadyForFrame(int maxLatencyMS) override;
 
    
    //ofThread
    void threadedFunction() override = 0;
    
    virtual int calculateBufferFullnessByTimeSent();
    virtual int calculateBufferFullnessByTimeAcked();
    
    // These two objects are for diagnostics...
    // stateRecorder periodically records the current buffer,
    // data speed, and roundtrip time
    DacStateRecorder stateRecorder;
    // frameRecorder records data about every frame
    // that is sent to the DAC
    DacFrameInfoRecorder frameRecorder;
    
    
    protected :
    
      
  #ifdef _MSC_VER
      void usleep(unsigned int usec)
      {
          HANDLE timer;
          LARGE_INTEGER ft;

          ft.QuadPart = -(10 * (__int64)usec);

          timer = CreateWaitableTimer(NULL, TRUE, NULL);
          SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
          WaitForSingleObject(timer, INFINITE);
          CloseHandle(timer);
      }
  #endif
    
    void waitUntilReadyToSend(int maxPointsToFillBuffer);
    
    void updateFrameQueue(int minPointsToQueue );
    int getNumPointsInFrames(deque<DacFrame*>& frames);
    // adds a point into the buffer ready to be sent to the DAC
    bool addPointToBuffer(const ofxLaser::Point& point );

    int getNumPointsInBufferedFrames();
    int getNumPointsInAllBuffers();
 
    ofThreadChannel<DacFrame*> frameThreadChannel;
    
    // buffered frames are all the frames sent but not yet
    // queued to be sent to the DAC
    deque<DacFrame*> bufferedFrames;
    deque<ofxLaser::Point*> bufferedPoints;
    
    uint32_t pps, newPPS;
    
    int lastReportedBufferFullness =0 ;
    bool dacIsRunning = false;
    
    // last time any command was acknowleged
    uint64_t lastAckTime = 0;
    // last time a data command was sent
    uint64_t lastDataSentTime = 0;
    uint64_t lastDataSentBufferSize= 0;
    
};

}

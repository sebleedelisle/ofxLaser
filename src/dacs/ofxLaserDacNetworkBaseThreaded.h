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

class DacNetworkBaseThreaded : public DacBase, public ofThread {
    
    public :
    
    // DacBase
    virtual bool sendFrame(const vector<Point>& points) override;
    //virtual bool sendPoints(const vector<Point>& points) override;
    virtual bool setColourShift(float shiftSeconds) override;
    
    virtual string getRawId() override = 0;
    virtual int getStatus() override = 0;
    virtual bool setPointsPerSecond(uint32_t pps) override = 0;
    virtual uint32_t getPointsPerSecond() override { return newPPS; } ;
    virtual int getDacTotalPointBufferCapacity() =0;
    
    virtual int getMinimumDacBufferFullnessForLatency();
    
    virtual void reset() override = 0;
    virtual void close() override = 0;
    void cleanUpFramesAndPoints(); 
    
    bool isReadyForFrame(int maxLatencyMS) override;
    
    virtual void logNotice(const string& msg) override {
       // if(logging && lock()) {
        if(logging ){
            logChannel.send(msg);
//            ofLogNotice() << msg;
            //unlock();
        }
    }
    void outputLogs() {
        string logmessage;
        while(logChannel.tryReceive(logmessage)) ofLogNotice() << logmessage;
        
    } 
 
    void setDiagnosticsRecording(bool state); 
    
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
    
    void waitUntilReadyToSend();
    
    void updateFrameQueue();
    int getNumPointsInFrames(deque<std::shared_ptr<DacFrame>>& frames) ;
    // adds a point into the buffer ready to be sent to the DAC
    bool addPointToBuffer(const ofxLaser::Point& point );

    //int getNumPointsInBufferedFrames();
    int getNumPointsInAllBuffers();
 
    ofThreadChannel<std::shared_ptr<DacFrame>> frameThreadChannel;
    
    // buffered frames are all the frames sent but not yet
    // queued to be sent to the DAC
    deque<std::shared_ptr<DacFrame>> bufferedFrames;
    std::atomic<int> numPointsInBufferedFrames =0;
    deque<ofxLaser::Point*> bufferedPoints;
    std::atomic<int> numBufferedPoints =0;
    std::shared_ptr<DacFrame> lastFrame = nullptr;
    
    std::atomic<uint32_t> pps, newPPS;
    
    std::atomic<int> lastReportedBufferFullness =0 ;

    bool dacIsRunning = false;
    
    // last time any command was acknowleged
    std::atomic<uint64_t> lastAckTimeMicros = 0;
    // last time a data command was sent
    std::atomic<uint64_t> lastDataSentTimeMicros = 0;
    std::atomic<uint64_t> lastDataSentBufferSize= 0;
    
    std::atomic<bool> readyForFrame = true;
    
    ofThreadChannel<string> logChannel;
    //std::atomic<bool> repeatingFrames = false;
//    ofThreadChannel<bool> readyForFrameChannel;
    
};

}

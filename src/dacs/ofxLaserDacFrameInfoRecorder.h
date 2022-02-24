//
//  ofxLaserDacFrameInfoRecorder.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 20/02/2022.
//

#pragma once
#include "ofMain.h"

struct FrameAtTime {
    uint64_t createdTimeMicros = 0;
    uint64_t sentTimeMicros = 0;
    uint32_t pointRate = 0;
    uint32_t numPoints = 0;
    int repeatCount = 1;
    bool skipped = false;
};

// TODO make the state history transfer a threadchannel
class DacFrameInfoRecorder {
    public :
    DacFrameInfoRecorder();
    
    void recordFrameInfoThreadSafe(uint64_t createdTimeMicros, uint64_t sentTimeMicros, uint32_t numPoints, int repeatCount, bool skipped);
    void update();
    //void recordState(uint64_t timemicros, int playbackstate, int bufferfullness, int roundtriptime, int numpointssent, int pointrate);
    
    const vector<FrameAtTime*>& getStateHistoryForTimePeriod(uint64_t starttimemicros, uint64_t endtimemicros);
    
    
    void getFrameLatencyValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues);
    
//    
    void getFrameRepeatValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) ;
    void getFrameSkipValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) ;
//    void getDataRateValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) ;
//   
    
    deque<FrameAtTime*> frameHistory;
    vector<FrameAtTime*> frameHistoryForTimePeriod;
    ofThreadChannel<FrameAtTime*> frameInfoChannel;
    
    float values[10000]; // used to store plot data, temporary storage
    
};


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
    uint32_t frameLatencyMicros = 0; 
    int repeatCount = 1;
    bool skipped = false;
};


class DacFrameInfoRecorder {
    public :
    DacFrameInfoRecorder();
    
    void recordFrameInfoThreadSafe(uint64_t createdTimeMicros, uint64_t sentTimeMicros, uint32_t numPoints, int repeatCount, bool skipped);
    void update();
    
    const vector<FrameAtTime*>& getStateHistoryForTimePeriod(uint64_t starttimemicros, uint64_t endtimemicros);
    
    void getFrameLatencyValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues);
    void getFrameLatencyValues( int numvalues, int widthpervalue);
    
    void getFrameRepeatValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) ;
    void getFrameRepeatValues(int numvalues, int widthpervalue) ;
    void getFrameSkipValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) ;
    void getFrameSkipValues(int numvalues, int widthpervalue) ;
   
    deque<FrameAtTime*> frameHistory;
    vector<FrameAtTime*> frameHistoryForTimePeriod;
    ofThreadChannel<FrameAtTime*> frameInfoChannel;
    
    float values[10000]; // used to store plot data, temporary storage
   
    bool recording;
};


//
//  ofxLaserDacStateRecorder.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 20/02/2022.
//

#pragma once
#include "ofMain.h"

// This system stores data about the last packet that was sent to the DAC

struct DacStateAtTime {
    uint64_t timeMicros = 0;
    uint32_t buffer = 0;
    uint32_t pointRate = 0;
    uint64_t roundTripTime = 0;
    // the number of bytes in the last send
    uint32_t numBytes = 0;
    float bytesPerSecond = 0; 
    bool playing = false;
};

// TODO make the state history transfer a threadchannel
class DacStateRecorder {
    public :
    DacStateRecorder();
    void recordStateThreadSafe(uint64_t timemicros, int playbackstate, int bufferfullness, int roundtriptime, int numpointssent, int pointrate, int numbytes);
    void update();
      
    const vector<DacStateAtTime*>& getStateHistoryForTimePeriod(uint64_t starttimemicros, uint64_t endtimemicros);

    void getBufferSizeValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues);

    void getLatencyValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) ;
    void getDataRateValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) ;
   
    
    deque<DacStateAtTime*> stateHistory;
    vector<DacStateAtTime*> stateHistoryForTimePeriod;
    ofThreadChannel<DacStateAtTime*> stateChannel;
    
    float values[10000]; // used to store plot data, temporary storage
    
};


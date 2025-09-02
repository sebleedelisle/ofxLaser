//
//  ofxLaserDacStateRecorder.cpp
//
//
//  Created by Seb Lee-Delisle on 20/02/2022.
//

#include "ofxLaserDacStateRecorder.h"


DacStateRecorder :: DacStateRecorder() {
    recording = false;
}

void DacStateRecorder :: recordStateThreadSafe(uint64_t timemicros, int playbackstate, int bufferfullness, int roundtriptime, int numpointssent, int pointrate, int numbytes) {
    if(!recording) return;
    DacStateAtTime* dacState = new DacStateAtTime();
    DacStateAtTime& bufferState = *dacState;
    bufferState.timeMicros = timemicros;
    bufferState.buffer = bufferfullness;
    bufferState.playing = playbackstate;
    bufferState.pointRate = pointrate;
    bufferState.roundTripTime = roundtriptime;
    bufferState.numBytes = numbytes;
    bufferState.bytesPerSecond = (float)numbytes *1000000.0f/  (float)roundtriptime ;
    
    stateChannel.send(dacState);
    
    
}
void DacStateRecorder :: update() {
    
    DacStateAtTime* dacState;
    while(stateChannel.tryReceive(dacState)) {
        stateHistory.push_back(dacState);
    }
    while(stateHistory.size()>=maxRecordCount) {
        DacStateAtTime* spare = stateHistory.front();
        stateHistory.pop_front();
        delete spare; // recycle?
    }
    
}


const vector<DacStateAtTime*>& DacStateRecorder :: getStateHistoryForTimePeriod(uint64_t starttimemicros, uint64_t endtimemicros) {
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

void DacStateRecorder :: getBufferSizeValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) {
    
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



void DacStateRecorder :: getDataRateValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) {
    
    getStateHistoryForTimePeriod(starttimemicros, endtimemicros); // updates stateHistoryForTimePeriod
    
    int bufferIndex = 0;
    uint64_t visibledurationmicros = endtimemicros-starttimemicros;
    if(stateHistoryForTimePeriod.size()>0) {
        // get the first buffer index for the start time
        float latency = stateHistoryForTimePeriod[bufferIndex]->bytesPerSecond/1000.0f;
        
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
                latency = (float)bufferstate->bytesPerSecond/1000.0f;
            }
            
            
            values[i] = latency;
            
        }
    }
    
}


void DacStateRecorder :: getLatencyValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) {
    
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


void DacStateRecorder :: getLatencyValues(int numvalues, int widthpervalue) {
   
    int startindex = stateHistory.size() - (numvalues/widthpervalue);
    int maxvalue = stateHistory.size()-1;
    
    for (int i =0; i<numvalues; i++) {
        
        int index = ofClamp(startindex + (i/widthpervalue), 0, maxvalue);
       
        values[i] = (maxvalue ==-1) ? 0 : stateHistory[index]->roundTripTime/1000.0f;
        
    }
    
    
}

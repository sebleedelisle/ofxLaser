//
//  ofxLaserDacFrameInfoRecorder.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 20/02/2022.
//

#include "ofxLaserDacFrameInfoRecorder.h"


DacFrameInfoRecorder :: DacFrameInfoRecorder() {
}

void DacFrameInfoRecorder :: recordFrameInfoThreadSafe (uint64_t createdtimemicros, uint64_t senttimemicros, uint32_t numpoints, int repeatcount, bool skipped) {
    
    FrameAtTime* frameInfoPointer = new FrameAtTime();
    FrameAtTime& frameInfo = *frameInfoPointer;
    frameInfo.createdTimeMicros = createdtimemicros;
    frameInfo.sentTimeMicros = senttimemicros;
    frameInfo.numPoints = numpoints;
    frameInfo.repeatCount = repeatcount;
    frameInfo.skipped = skipped;
    
    frameInfoChannel.send(frameInfoPointer);
    
    
}
void DacFrameInfoRecorder :: update() {
    
    FrameAtTime* frameInfo;
    while(frameInfoChannel.tryReceive(frameInfo)) {
        frameHistory.push_back(frameInfo);
    }
    while(frameHistory.size()>=1000) {
        FrameAtTime* spare = frameHistory.front();
        frameHistory.pop_front();
        delete spare; // recycle?
    }
    
}


const vector<FrameAtTime*>& DacFrameInfoRecorder :: getStateHistoryForTimePeriod(uint64_t starttimemicros, uint64_t endtimemicros) {
    // assumes that history date is sorted by time
    frameHistoryForTimePeriod.clear();
    
    for(size_t i = 0; i<frameHistory.size(); i++) {
        FrameAtTime* frameAtTime = frameHistory[i];
        bool firstfound = true;
        if((frameAtTime->sentTimeMicros >= starttimemicros) && (frameAtTime->sentTimeMicros <= endtimemicros)) {
            if(firstfound) {
                if(i>0) frameHistoryForTimePeriod.push_back(frameHistory[i-1]);
                firstfound = false;
            }
            frameHistoryForTimePeriod.push_back(frameAtTime);
        }
        
    }
    
    return frameHistoryForTimePeriod;
    
}

void DacFrameInfoRecorder :: getFrameLatencyValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) {
    
    getStateHistoryForTimePeriod(starttimemicros, endtimemicros); // updates stateHistoryForTimePeriod
    
    int bufferIndex = 0;
    uint64_t visibledurationmicros = endtimemicros-starttimemicros;
    if(frameHistoryForTimePeriod.size()>0) {
        // get the first buffer index for the start time
        
        FrameAtTime* frameInfo = frameHistoryForTimePeriod[bufferIndex];
        int latencyMicros =  0; //frameInfo->sentTimeMicros-frameInfo->createdTimeMicros;
        
        for (int i =0; i<numvalues; i++) {
            uint64_t timeMicros = visibledurationmicros;
            timeMicros *= i;
            timeMicros /= numvalues;
            timeMicros+=starttimemicros;
            
            latencyMicros = 0;
            while((bufferIndex+1<frameHistoryForTimePeriod.size()) &&  (frameHistoryForTimePeriod[bufferIndex+1]->createdTimeMicros < timeMicros)) {
                
                bufferIndex++;
                frameInfo = (frameHistoryForTimePeriod[bufferIndex]);
                //buffersize = 5000;
                if(!frameInfo->skipped) latencyMicros = frameInfo->sentTimeMicros-frameInfo->createdTimeMicros;
            }
            
            //buffersize = (float)bufferstate->buffer;
           
            values[i] = latencyMicros;
            
        }
    }
    
    
}

void DacFrameInfoRecorder :: getFrameRepeatValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) {
    
    getStateHistoryForTimePeriod(starttimemicros, endtimemicros); // updates stateHistoryForTimePeriod
    
    int bufferIndex = 0;
    uint64_t visibledurationmicros = endtimemicros-starttimemicros;
    if(frameHistoryForTimePeriod.size()>0) {
        // get the first buffer index for the start time
        
        FrameAtTime* frameInfo = frameHistoryForTimePeriod[bufferIndex];
        int repeatcount =  0; //frameInfo->sentTimeMicros-frameInfo->createdTimeMicros;
        
        for (int i =0; i<numvalues; i++) {
            uint64_t timeMicros = visibledurationmicros;
            timeMicros *= i;
            timeMicros /= numvalues;
            timeMicros+=starttimemicros;
            
            repeatcount = 0;
            while((bufferIndex+1<frameHistoryForTimePeriod.size()) &&  (frameHistoryForTimePeriod[bufferIndex+1]->createdTimeMicros < timeMicros)) {
                
                bufferIndex++;
                frameInfo = (frameHistoryForTimePeriod[bufferIndex]);
                //buffersize = 5000;
                repeatcount = frameInfo->repeatCount;
            }
            
            //buffersize = (float)bufferstate->buffer;
           
            values[i] = repeatcount;
            
        }
    }
    
    
}


void DacFrameInfoRecorder :: getFrameSkipValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) {
    
    getStateHistoryForTimePeriod(starttimemicros, endtimemicros); // updates stateHistoryForTimePeriod
    
    int bufferIndex = 0;
    uint64_t visibledurationmicros = endtimemicros-starttimemicros;
    if(frameHistoryForTimePeriod.size()>0) {
        // get the first buffer index for the start time
        
        FrameAtTime* frameInfo = frameHistoryForTimePeriod[bufferIndex];
        int skip =  0; //frameInfo->sentTimeMicros-frameInfo->createdTimeMicros;
        
        for (int i =0; i<numvalues; i++) {
            uint64_t timeMicros = visibledurationmicros;
            timeMicros *= i;
            timeMicros /= numvalues;
            timeMicros+=starttimemicros;
            
            skip = 0;
            while((bufferIndex+1<frameHistoryForTimePeriod.size()) &&  (frameHistoryForTimePeriod[bufferIndex+1]->createdTimeMicros < timeMicros)) {
                
                bufferIndex++;
                frameInfo = (frameHistoryForTimePeriod[bufferIndex]);
                //buffersize = 5000;
                skip = frameInfo->skipped ? 1 : 0;
            }
            
            //buffersize = (float)bufferstate->buffer;
           
            values[i] = skip;
            
        }
    }
    
    
}

//
//
//void DacFrameInfoRecorder :: getDataRateValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) {
//    
//    getStateHistoryForTimePeriod(starttimemicros, endtimemicros); // updates stateHistoryForTimePeriod
//    
//    int bufferIndex = 0;
//    uint64_t visibledurationmicros = endtimemicros-starttimemicros;
//    if(frameHistoryForTimePeriod.size()>0) {
//        // get the first buffer index for the start time
//        float latency = frameHistoryForTimePeriod[bufferIndex]->bytesPerSecond/1000.0f;
//        
//        FrameAtTime* bufferstate = frameHistoryForTimePeriod[bufferIndex];
//        
//        
//        for (int i =0; i<numvalues; i++) {
//            uint64_t timeMicros = visibledurationmicros;
//            timeMicros *= i;
//            timeMicros /= numvalues;
//            timeMicros+=starttimemicros;
//            
//            
//            while((bufferIndex+1<frameHistoryForTimePeriod.size()) && frameHistoryForTimePeriod[bufferIndex+1]->timeMicros < timeMicros) {
//                
//                bufferIndex++;
//                bufferstate = (frameHistoryForTimePeriod[bufferIndex]);
//                //buffersize = 5000;
//                latency = (float)bufferstate->bytesPerSecond/1000.0f;
//            }
//            
//            
//            values[i] = latency;
//            
//        }
//    }
//    
//}
//
//
//void DacFrameInfoRecorder :: getLatencyValuesForTime(uint64_t starttimemicros, uint64_t endtimemicros, int numvalues) {
//    
//    getStateHistoryForTimePeriod(starttimemicros, endtimemicros); // updates stateHistoryForTimePeriod
//    
//    int bufferIndex = 0;
//    uint64_t visibledurationmicros = endtimemicros-starttimemicros;
//    if(frameHistoryForTimePeriod.size()>0) {
//        // get the first buffer index for the start time
//        float latency = frameHistoryForTimePeriod[bufferIndex]->roundTripTime/1000.0f;
//        
//        FrameAtTime* bufferstate = frameHistoryForTimePeriod[bufferIndex];
//        
//        
//        for (int i =0; i<numvalues; i++) {
//            uint64_t timeMicros = visibledurationmicros;
//            timeMicros *= i;
//            timeMicros /= numvalues;
//            timeMicros+=starttimemicros;
//            
//            
//            while((bufferIndex+1<frameHistoryForTimePeriod.size()) && frameHistoryForTimePeriod[bufferIndex+1]->timeMicros < timeMicros) {
//                
//                bufferIndex++;
//                bufferstate = (frameHistoryForTimePeriod[bufferIndex]);
//                //buffersize = 5000;
//                latency = (float)bufferstate->roundTripTime/1000.0f;
//            }
//            
//            
//            values[i] = latency;
//            
//        }
//    }
//    
//}

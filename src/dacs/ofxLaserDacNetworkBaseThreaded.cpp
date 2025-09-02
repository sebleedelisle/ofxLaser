//
//  DacThreadedBase.cpp
//
//
//  Created by Seb Lee-Delisle on 11/05/2022.
//

#include "ofxLaserDacNetworkBaseThreaded.h"

using namespace ofxLaser;



bool DacNetworkBaseThreaded :: sendFrame(const vector<Point>& points){

    outputLogs();
    
    if(!isThreadRunning()) return false;
    
    // update stats if we're recording them
    stateRecorder.update();
    frameRecorder.update();
    
    // create a frame, set it's time to now
    std::shared_ptr<DacFrame> frame = std::make_shared<DacFrame>(ofGetElapsedTimeMicros());
        
    // add the points to the frame
    for(size_t i= 0; i<points.size(); i++) {
        frame->addPoint(points[i]);
    }
    
    // and pass it into the frame channel
    frameThreadChannel.send(frame);
    
    return true;

}

int DacNetworkBaseThreaded :: calculateBufferFullnessByTimeSent() {
    
    int elapsedMicros = ofGetElapsedTimeMicros() - lastDataSentTimeMicros;
    // figure out the current buffer
    return MAX(0, lastDataSentBufferSize - (((float)elapsedMicros/1000000.0f) * pps));
   
}

int DacNetworkBaseThreaded :: calculateBufferFullnessByTimeAcked() {
   
    int microsSinceLastReport = ofGetElapsedTimeMicros() - lastAckTimeMicros;
    // figure out the current buffer
    return MAX(0, lastReportedBufferFullness - (((float)microsSinceLastReport/1000000.0f) * pps));
   
}

void DacNetworkBaseThreaded :: waitUntilReadyToSend(){

    if(pps==0) return; // bit of a hack but let's make sure the thread keeps going
    
    int minPointsInBuffer = getMinimumDacBufferFullnessForLatency(); // MIN(getDacTotalPointBufferCapacity()-minPacketDataSize, maxLatencyMS * pps /1000);
    
    int bufferFullness = calculateBufferFullnessByTimeSent();
    int pointsUntilNeedsRefill = MAX(minPacketDataSize, bufferFullness - minPointsInBuffer);
    int microsToWait = pointsUntilNeedsRefill * (1000000.0f/pps);
    microsToWait-=30000; // buffer of 30ms
    
    if(microsToWait>0) {
        //ofLogNotice("DacBaseThreaded :: waitUntilReadyToSend -  ") << bufferFullness << " " << calculateBufferSizeByTimeAcked() << " " << maxPointsToFillBuffer << " " << pointsUntilEmpty;
        //ofLogNotice("Sleep : " ) << (float)microsToWait/1000.0f;
        usleep(microsToWait);
    }
    
}

bool DacNetworkBaseThreaded :: isReadyForFrame(int maxlatencyms) {
    
    maxLatencyMS = maxlatencyms;
    //return true;
    if(numPointsInBufferedFrames==0) return true;
    else {
        int queuedPointCount = getNumPointsInAllBuffers();
        return (queuedPointCount<((maxlatencyms+calculationTimeMS)*newPPS/1000));
        
    }
}

// updates the frame buffer with new frames from the threadchannel,
// adds frames to the frame queue until we have minPointsToQueue
// it's called right before we send points to the DAC.

void DacNetworkBaseThreaded ::  updateFrameQueue(){
    
    
    
    // CALCULATE HOW MANY POINTS TO SEND
    
    // BUFFER FULLNESS BY TIME ACKED :
    // is an overestimate because some points will have been processed by the time we receive it
    // BUFFER FULLNESS BY TIME SENT
    // is an underestimate because it will have taken some time to get to the DAC before it starts process
    
    // get current buffer
    int maxEstimatedBufferFullness = calculateBufferFullnessByTimeAcked();
    //int minEstimatedBufferFullness = calculateBufferFullnessByTimeSent();

    
    // get min buffer fullness required to maintain latency
    int minDacBufferFullness = getMinimumDacBufferFullnessForLatency(); // maxLatencyMS * pps / 1000;
    
    int numPointsAvailable =  bufferedPoints.size();
    int minPointsToAdd = MAX(0, minDacBufferFullness - maxEstimatedBufferFullness - numPointsAvailable);
        
    // get all the new frames in the channel
    std::shared_ptr<DacFrame> frame;
    while(frameThreadChannel.tryReceive(frame) && isThreadRunning()) {
        //frameMode = true;
        bufferedFrames.push_back(frame);
        numPointsInBufferedFrames+=frame->getNumPoints();
    }
    
    deque<std::shared_ptr<DacFrame>> queuedFrames;
    
    // buffer by time sent will be a higher number than by time acked as there will
    // always be a little latency
    int dacBufferFullness = calculateBufferFullnessByTimeSent();
    
    // go through the buffered frames and add them into the buffer until we have enough points
    // or we run out of frames
    
    
    int pointsInQueuedFrames = 0;
    
  
    // repeat last frame functionality... when should we do this? If we do it too early then we
    // are never ready for a new frame! Or if we do this then we definitely need to get a new frame
    
    // if we have no new frames and we're desperate for points then reuse the last frame
//    if((pointsInQueuedFrames< minPointsToQueue) && (queuedFrames.size()==0) && (lastFrame!=nullptr) ){
//        //ofLogNotice("We have no buffered frames so adding last Frame to queue");
//        queuedFrames.push_back(lastFrame);
//        lastFrame->repeatCount = 1;
//        //repeatingFrames = true;
//    } else {
//        //repeatingFrames = false;
//    }
//    
    // if we don't have any buffered frames then put the last frame in there, just in case
    if((bufferedFrames.size()==0) && (lastFrame!=nullptr)){
        if(verbose) ofLogNotice("Out of frames - repeating last frame, adding to queue");
        bufferedFrames.push_back(lastFrame);
    }
    
    
    while((pointsInQueuedFrames<minPointsToAdd) && (bufferedFrames.size()>0) && isThreadRunning()) {
        // calculate the time that the last point in the queue will be processed
        uint64_t lastPointTimeMicros = ((dacBufferFullness + bufferedPoints.size() + pointsInQueuedFrames) *1000000 / pps) + ofGetElapsedTimeMicros();
        
        // LOGIC should be...
        // get next frame...
        // is it too late? yes
        // keep going until it's not too late

        std::shared_ptr<DacFrame> nextframe;
        
        int skipcount = 0;
        
        // while the frames are too old, skip them!
        while((bufferedFrames.size()>1) && (bufferedFrames.front()->frameTime + ((maxLatencyMS)*1000) < lastPointTimeMicros)) {
            // skip frame! So record that a frame was skipped
            std::shared_ptr<DacFrame> skippedframe = bufferedFrames.front();
            frameRecorder.recordFrameInfoThreadSafe(skippedframe->frameTime, 0, skippedframe->framePoints.size(), 0, true);
            bufferedFrames.pop_front();
            numPointsInBufferedFrames-=skippedframe->getNumPoints();
            skipcount++;
            
        }
        if((skipcount>0)) {
            //ofLogNotice("skipping frames : ") << skipcount;
            
        }
        // now keep going until we get the newest frame
        while(bufferedFrames.size()>0) {
            nextframe = bufferedFrames.front();
            bufferedFrames.pop_front();
            numPointsInBufferedFrames-=nextframe->getNumPoints();
            // if we still have frames, then mark this one as skipped.
            if(bufferedFrames.size()>0) {
                frameRecorder.recordFrameInfoThreadSafe(nextframe->frameTime, 0, nextframe->framePoints.size(), 0, true);
            }
            
        }
        
        if(nextframe) {
            nextframe->repeatCount = 1;
            // add the frame to the queue and update the point count
            queuedFrames.push_back(nextframe);
            pointsInQueuedFrames+=nextframe->getNumPoints();
        }

    }
    
    // if we still don't have enough points then double up!
    // TODO make this better, spread the repeats better, although i suspect the vast majority of the time it doesn't matter
    int i = 0;
    // note that this will hang if there are no points in any frames! Edge case but look out for it.
    while((queuedFrames.size()>0) && (pointsInQueuedFrames<minPointsToAdd) && isThreadRunning()) {
        std::shared_ptr<DacFrame>& frame = queuedFrames[i];
        frame->repeatCount++;
        pointsInQueuedFrames+=frame->getNumPointsForSingleRepeat();
        i++;
        if(i>=queuedFrames.size()) i=0;
    }

    // add all queued frames points to the buffer
    
    for(int i = 0; i<queuedFrames.size(); i++ ) {
        
        if(!isThreadRunning()) break;
        
        shared_ptr<DacFrame> frame = queuedFrames[i];
        // record the frame :
        frameRecorder.recordFrameInfoThreadSafe(frame->frameTime, ofGetElapsedTimeMicros() + (( calculateBufferFullnessByTimeSent() + bufferedPoints.size()) * 1000000 / pps), frame->framePoints.size(), frame->repeatCount-1, false);
        // now add all the points
        while((frame->repeatCount>0)&&(isThreadRunning())) {
            for(ofxLaser::Point* point : frame->framePoints) {
               
                if((!armed) || (blankPointsAfterReArmRemaining>0) || (!isThreadRunning())) {
                    point->r = point->g = point->b = 0;
                    if(blankPointsAfterReArmRemaining>0) blankPointsAfterReArmRemaining--;
                    //ofLogNotice("blankPointsAfterReArmRemaining") << blankPointsAfterReArmRemaining;
                }
                addPointToBuffer(*point);
            }
            frame->repeatCount--;
        }
        // and store the last frame
        lastFrame = frame;
    }
}


inline bool DacNetworkBaseThreaded :: addPointToBuffer(const ofxLaser::Point &point ){
    ofxLaser::Point* p = PointFactory :: getPoint(point);
    //*p = point; // copy assignment hopefully!
    bufferedPoints.push_back(p);
    numBufferedPoints++;
    return true;
}


// must always be run in lock <<< ????? WTF
int DacNetworkBaseThreaded :: getNumPointsInFrames(deque<std::shared_ptr<DacFrame>>& frames) {
    if(frames.size()==0) return 0;
    int totalpoints = 0;
  
    for(auto frame : frames) {
        totalpoints+=(frame->getNumPoints());
    }
    //unlock();

    return totalpoints;
}



int DacNetworkBaseThreaded :: getNumPointsInAllBuffers() {
  
    int fullness = calculateBufferFullnessByTimeSent(); // < should be thread safe
    int size = numBufferedPoints; // bufferedPoints.size(); // <<  thread safe
    int pointsinframes = numPointsInBufferedFrames; // thread safe getNumPointsInBufferedFrames(); // < not thread safe
    //ofLogNotice("getNumPointsInAllBuffers : ") << fullness << " " << size <<  " " << pointsinframes;
    return fullness + size + pointsinframes;
}


//
//int DacBaseThreaded :: getNumPointsInBufferedFrames() {
//
//    return  getNumPointsInFrames(bufferedFrames);
//}


// set the colour shift in seconds
bool DacNetworkBaseThreaded::setColourShift(float shift)  {
    
    colourShift = shift;
    return true;

    
}

void DacNetworkBaseThreaded::cleanUpFramesAndPoints() {
    ofLogNotice("cleanUpFramesAndPoints");
    // NOTE thread must be stopped by now
    frameThreadChannel.close();
    
    // get rid of frames in the buffer
    // note that deleting the frame object
    // also recycles the points
    std::shared_ptr<DacFrame> frame;
    while(frameThreadChannel.tryReceive(frame)) {

    }
   
//    while(bufferedFrames.size()>0) {
//        //delete bufferedFrames[0];
//        bufferedFrames.pop_front();
//    }
    bufferedFrames.clear();
    numPointsInBufferedFrames = 0;
    for (size_t i= 0; i < bufferedPoints.size(); ++i) {
        PointFactory :: releasePoint(bufferedPoints[i]); // Calls ~object
    }
    bufferedPoints.clear();
    numBufferedPoints = 0;
    
}

int DacNetworkBaseThreaded::getMinimumDacBufferFullnessForLatency() {
    return MIN(maxLatencyMS * pps / 1000, getDacTotalPointBufferCapacity());
}


void DacNetworkBaseThreaded::setDiagnosticsRecording(bool state) {
    if(isThreadRunning()) {
        if(lock()) {
            stateRecorder.recording = state;
            frameRecorder.recording = state;
            unlock();
        }
    } else{
        stateRecorder.recording = state;
        frameRecorder.recording = state;
    } 
    
}



//
//  DacThreadedBase.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 11/05/2022.
//

#include "ofxLaserDacThreadedBase.h"

using namespace ofxLaser;



bool DacThreadedBase :: sendFrame(const vector<Point>& points){

        
    stateRecorder.update();
    frameRecorder.update();

    if(lock()) {
        frameMode = true;
        //newFrame = true;
        unlock();
    }
    
    DacFrame* frame = new DacFrame(ofGetElapsedTimeMicros());
        
    // add the points to the frame
    for(size_t i= 0; i<points.size(); i++) {
        frame->addPoint(points[i]);
    }
    
    frameThreadChannel.send(frame);
    
    return true;

}



// updates the frame buffer with new frames from the threadchannel,
// adds frames to the frame queue until we have minPointsToQueue
// and up to maxPointsToSend
void DacThreadedBase ::  updateFrameQueue(int minPointsToQueue){
    
    // get all the new frames in the channel
    DacFrame* frame;
    while(frameThreadChannel.tryReceive(frame)) {
        bufferedFrames.push_back(frame);
    }
    deque<DacFrame*> queuedFrames;
   
    int dacBufferFullness = calculateBufferSizeByTimeSent();

    // go through the buffered frames and add them into the buffer until we have enough points
    // or we run out of frames
    int skipcount = 0;
    int queuecount = 0;
    int pointsInQueuedFrames =getNumPointsInFrames(queuedFrames);
    while((pointsInQueuedFrames<minPointsToQueue) && (bufferedFrames.size()>0)) {
        // calculate the time that the last point in the queue will be processed
        uint64_t lastPointTimeMicros = ((dacBufferFullness + + bufferedPoints.size() + pointsInQueuedFrames) *1000000 / pps) + ofGetElapsedTimeMicros();
        DacFrame* frame = bufferedFrames[0];
        bufferedFrames.pop_front();
        // if we didn't get to the frame in time and it's more than 10ms late then skip it
        if(frame->frameTime + ((maxLatencyMS)*1000) < lastPointTimeMicros) {
            // skip frame!
            frameRecorder.recordFrameInfoThreadSafe(frame->frameTime, 0, frame->framePoints.size(), 0, true);
            delete frame;
            skipcount++;
        } else {
            queuedFrames.push_back(frame);
            queuecount++;

        }
        pointsInQueuedFrames =getNumPointsInFrames(queuedFrames);
    }
    //cout << "skipped : " << skipcount  << " queued : " << queuecount << endl;
    //cout << "queued frames : " << queuedFrames.size()  << " buffered frames : " << bufferedFrames.size() << endl;
    // if we still don't have enough points then double up!
    // TODO make this better, spread the repeats better
    int i = 0;
    while((i<queuedFrames.size()) && (getNumPointsInFrames(queuedFrames)<minPointsToQueue)) {
        queuedFrames[i]->repeatCount++;
        //cout << "+++ repeating " << i << " " << queuedFrames[i]->repeatCount << endl;
        i++;
        if(i>=queuedFrames.size()) i=0;
    }

    // add all queued frames points to the buffer
    
    for(int i = 0; i<queuedFrames.size(); i++ ) {
        DacFrame& frame = *queuedFrames[i];
        frameRecorder.recordFrameInfoThreadSafe(frame.frameTime, ofGetElapsedTimeMicros() + (( calculateBufferSizeByTimeSent() + bufferedPoints.size()) * 1000000 / pps), frame.framePoints.size(), frame.repeatCount, frame.repeatCount == 0);
        
      
        while(frame.repeatCount>0) {
            for(ofxLaser::Point* point : frame.framePoints) {
                addPointToBuffer(*point);
            }
            frame.repeatCount--;
        }
    }
    // now clear the frames!

    while(queuedFrames.size()>0) {
        delete queuedFrames[0];
        queuedFrames.pop_front();
    }

    
    
}


inline bool DacThreadedBase :: addPointToBuffer(const ofxLaser::Point &point ){
    ofxLaser::Point* p = PointFactory :: getPoint(point);
    //*p = point; // copy assignment hopefully!
    bufferedPoints.push_back(p);
    return true;
}


int DacThreadedBase :: getNumPointsInFrames(deque<DacFrame*>& frames) {
    int totalpoints = 0;
    for(DacFrame* frame : frames) {
        totalpoints+=(frame->getNumPoints());
    }
    return totalpoints;
}


bool DacThreadedBase :: isReadyForFrame(int maxlatencyms) {
   // return true;
    int queuedPoints = 0;
    if(lock()) {
        
        queuedPoints = getNumPointsInAllBuffers();
        maxLatencyMS = maxlatencyms;
        unlock();
    }
    bool ready = queuedPoints<(maxLatencyMS*newPPS/1000);

    return  ready;
}

int DacThreadedBase :: getNumPointsInAllBuffers() {
    // if not in thread then needs lock!
      return calculateBufferSizeByTimeSent() + bufferedPoints.size() + getNumPointsInBufferedFrames();
    
}



int DacThreadedBase :: getNumPointsInBufferedFrames() {
    return  getNumPointsInFrames(bufferedFrames);
}


// set the colour shift in seconds
bool DacThreadedBase::setColourShift(float shift)  {

    if(!isThreadRunning()){
        colourShift =shift;
        return true;
    } else {
        if(lock()){
            colourShift = shift;
            unlock();
            return true;
        } else {
            return false;
        }
    }
    
}

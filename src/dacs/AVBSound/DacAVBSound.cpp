#include "DacAVBSound.h"

namespace ofxLaser {

DacAVBSound::DacAVBSound() {
    pps = 48000; // TODO set with sound card, maybe an init?
}

DacAVBSound::~DacAVBSound() {
}

bool DacAVBSound::sendFrame(const vector<Point>& points) {
    
    if(!getActive()) return false;
    
    // update stats if we're recording them
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

// likely to be run from the audioIn thread
vector<std::shared_ptr<DacAVBSoundPoint>> DacAVBSound::getNextPoints(int numofpoints) {

    vector<std::shared_ptr<DacAVBSoundPoint>> pointsToReturn;
    if(!getActive()) return pointsToReturn; 


    updateFrameQueue();
    
    //ofLogNotice("DacAVBSound::getNextPoints : getting ") << numofpoints << " buffer size : " << bufferedPoints.size();
    
    for(int i =0; i<numofpoints; i++) {
        
        std::shared_ptr<ofxLaser::Point> bufferedPoint;
        
        if(bufferedPoints.size()>0) {
            bufferedPoint = bufferedPoints.front();
            bufferedPoints.pop_front();
            
            pointsToReturn.push_back(std::make_shared<DacAVBSoundPoint>(
                ofMap(bufferedPoint->x, 0,800,-1,1),
                ofMap(bufferedPoint->y, 0,800,1,-1),
                ofMap(bufferedPoint->r, 0,255,0,1),
                ofMap(bufferedPoint->g, 0,255,0,1),
                ofMap(bufferedPoint->b, 0,255,0,1),
                0,
                0,
                0
            ));
            
        } else {
            ofLogNotice("DacAVBSound::getNextPoints : run out of buffered points!");
            break;
        }
    }
    
    
    return pointsToReturn;
}
// likely to be run from the audioIn thread
void DacAVBSound::releasePoints(vector<std::shared_ptr<DacAVBSoundPoint>> pointstorelease) {

    //sparePoints.insert(sparePoints.end(), pointstorelease.begin(), pointstorelease.end());
    
}




// updates the frame buffer with new frames from the threadchannel,
// adds frames to the frame queue until we have minPointsToQueue
// it's called right before we send points to the DAC.

void DacAVBSound ::  updateFrameQueue(){
    
    // CALCULATE HOW MANY POINTS TO SEND
    
    // I guess I just have to decide how many points should be in the buffer.
    int maxBuffer = 5000; // arbitrary
    
    int numPointsAvailable =  bufferedPoints.size();
    
    int minPointsToAdd = MAX(0, maxBuffer - numPointsAvailable);
        
    // get all the new frames in the channel
    std::shared_ptr<DacFrame> frame;
    while(frameThreadChannel.tryReceive(frame) && getActive()) { // presumably active could be turned off mid way through?
        bufferedFrames.push_back(frame);
        numPointsInBufferedFrames+=frame->getNumPoints();
    }
    
    deque<std::shared_ptr<DacFrame>> queuedFrames;
    
    int dacBufferFullness = 0;
    
    
    int pointsInQueuedFrames = 0;

    // if we don't have any buffered frames then put the last frame in there, just in case
    if((bufferedFrames.size()==0) && (lastFrame!=nullptr)){
        if(verbose) ofLogNotice("Out of frames - repeating last frame, adding to queue");
        bufferedFrames.push_back(lastFrame);
    }
    
    
    while((pointsInQueuedFrames<minPointsToAdd) && (bufferedFrames.size()>0) && getActive()) {
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
            //frameRecorder.recordFrameInfoThreadSafe(skippedframe->frameTime, 0, skippedframe->framePoints.size(), 0, true);
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
//            if(bufferedFrames.size()>0) {
//                frameRecorder.recordFrameInfoThreadSafe(nextframe->frameTime, 0, nextframe->framePoints.size(), 0, true);
//            }
            
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
    while((queuedFrames.size()>0) && (pointsInQueuedFrames<minPointsToAdd) && getActive()) {
        std::shared_ptr<DacFrame>& frame = queuedFrames[i];
        frame->repeatCount++;
        pointsInQueuedFrames+=frame->getNumPointsForSingleRepeat();
        i++;
        if(i>=queuedFrames.size()) i=0;
    }

    // add all queued frames points to the buffer
    
    for(int i = 0; i<queuedFrames.size(); i++ ) {
        
        if(!getActive()) break;
        
        shared_ptr<DacFrame> frame = queuedFrames[i];
        // record the frame :
        //frameRecorder.recordFrameInfoThreadSafe(frame->frameTime, ofGetElapsedTimeMicros() + (( calculateBufferFullnessByTimeSent() + bufferedPoints.size()) * 1000000 / pps), frame->framePoints.size(), frame->repeatCount-1, false);
        // now add all the points
        while((frame->repeatCount>0)&&(getActive())) {
            for(ofxLaser::Point* point : frame->framePoints) {
                if(!armed) {
                    point->x = point->y = 400;
                }
                if((!armed) || (blankPointsAfterReArmRemaining>0) || (!getActive())) {
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
    
   // ofLogNotice("DacAVBSound :: updateFrameQueue - num points = ") <<bufferedPoints.size();
}

inline bool DacAVBSound :: addPointToBuffer(const ofxLaser::Point &point ){
    //ofxLaser::Point* p = PointFactory :: getPoint(point);
    //*p = point; // copy assignment hopefully!
    std::shared_ptr<ofxLaser::Point> p;
    if(sparePoints.size()>0) {
        p = sparePoints.back();
        sparePoints.pop_back();
    } else {
        p = std::make_shared<ofxLaser::Point>(point); // hopefully copy constructor?
    }
    bufferedPoints.push_back(p);
    numBufferedPoints++;
    return true;
}

bool DacAVBSound::setPointsPerSecond(uint32_t pps) {
    return false;
}

uint32_t DacAVBSound::getPointsPerSecond() {
    return pps;
}

bool DacAVBSound::setColourShift(float shiftSeconds) {
    return false;
}

string DacAVBSound::getRawId() {
    return dacname;
}

int DacAVBSound::getStatus() {
    return getActive() ? OFXLASER_DACSTATUS_GOOD : OFXLASER_DACSTATUS_NO_DAC; // todo probably also send the soundstream status
}

void DacAVBSound::reset() {
}

void DacAVBSound::close() {
    setActive(false);
}

bool DacAVBSound::isReadyForFrame(int maxLatencyms) {
    maxLatencyMS = maxLatencyms;
    return true;
}

}

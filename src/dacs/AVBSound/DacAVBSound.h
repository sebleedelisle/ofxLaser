#pragma once

#include "ofxLaserDacBase.h"
#include "ofxLaserDacFrame.h"

namespace ofxLaser {

// it's quite strange but this class doesn't really do much except figure out the point stream

struct DacAVBSoundPoint {
    
    DacAVBSoundPoint(float x, float y, float r, float g, float b, float i, float u1, float u2)
        : x(x), y(y), r(r), g(g), b(b), i(i), u1(u1), u2(u2) {}
    
    float x = 0;
    float y = 0;
    float r = 0;
    float g = 0;
    float b = 0;
    float i = 0;
    float u1 = 0;
    float u2 = 0;

};



class DacAVBSound : public DacBase {
public:
    DacAVBSound();
    ~DacAVBSound();

    virtual string getType() override { return "AVB/Sound"; };
    
    virtual bool sendFrame(const vector<Point>& points) override;
    
    virtual bool setPointsPerSecond(uint32_t pps) override;
    virtual uint32_t getPointsPerSecond() override;
    virtual bool setColourShift(float shiftSeconds) override;
    virtual string getRawId() override;
    
    virtual int getStatus() override;
    virtual void reset() override;
    virtual void close() override;
    
    void setDacName(string newname) {
        dacname = newname;
    }
    
    virtual bool isReadyForFrame(int maxLatencyMS) override;
    
    virtual bool setActive(bool state){
        if(state != active) {
            active= state;
            return true;
        } else {
            return false;
        }
    }
    virtual bool getActive() {
        return active;
    }
    
    vector<std::shared_ptr<DacAVBSoundPoint>>  getNextPoints(int numofpoints);
    void releasePoints(vector<std::shared_ptr<DacAVBSoundPoint>> pointstorelease);

    void updateFrameQueue();
    bool addPointToBuffer(const ofxLaser::Point& point );

    
protected :
    std::atomic<bool> active =false;
    string dacname = "";
    ofThreadChannel<std::shared_ptr<DacFrame>> frameThreadChannel;
    
    deque<std::shared_ptr<ofxLaser::Point>> bufferedPoints;
    vector<std::shared_ptr<ofxLaser::Point>> sparePoints; 
    deque<std::shared_ptr<DacFrame>> bufferedFrames;
    std::atomic<int> numPointsInBufferedFrames =0;
    std::atomic<int> numBufferedPoints =0;
    std::shared_ptr<DacFrame> lastFrame = nullptr;
    std::atomic<uint32_t> pps; 
    
private:
 
};

}

//
//  ofxLaserDacEmpty.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 30/03/2021.
//

#pragma once
#include "ofxLaserDacBase.h"

namespace ofxLaser {

    class DacEmpty : public DacBase{
    public:
        DacEmpty() {
            colourShiftImplemented = true;
        };
        
        virtual bool sendFrame(const vector<Point>& points) override { return true; } ;
        virtual bool sendPoints(const vector<Point>& points) override { return true; } ;
        virtual bool setPointsPerSecond(uint32_t pps) override { return true; };
        virtual bool setColourShift(float shiftSeconds) override { return true; };
        virtual string getId() override {return "";};
        
        virtual int getStatus() override {return OFXLASER_DACSTATUS_NO_DAC;};
        virtual bool isReadyForFrame(int maxLatencyMS) override{
            if(dontCalculate) return false;
            else {
                bool update = false;
                if(ofGetElapsedTimef() - lastUpdate > (1.0f / (float)framerate)) {
                    update = true;
                    lastUpdate = ofGetElapsedTimef();
                }
                return update;
            }
        };
       // virtual ofColor getStatusColour() override {return ofColor::white; };
    
        virtual void reset()  override  {};
        virtual void close() override {};

        int framerate = 60;
        float lastUpdate = 0;
        bool dontCalculate = false;
    protected :
    
        vector<ofAbstractParameter*> displayData;
        bool resetFlag = false;
        bool armed = false;

    };

}

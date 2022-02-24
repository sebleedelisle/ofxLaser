//
//  ofxLaserDacEtherDream.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#pragma once

#include "ofxLaserDacEtherDreamDacPointFactory.h"


#define ETHERDREAM_MIN -32768
#define ETHERDREAM_MAX 32767



namespace ofxLaser {

class DacEtherDreamFrame {
    public :
    
    DacEtherDreamFrame(uint64_t time) {
        setTime(time); 
    }
    ~DacEtherDreamFrame() {
        clear();
    }
    void setTime(uint64_t time) {
        frameTime = time;
    }
    
    void addPoint(const ofxLaser::Point& laserPoint) {
 
        EtherDreamDacPoint* framePoint = EtherDreamDacPointFactory :: getPoint();

        framePoint->x = ofMap(laserPoint.x,0,800,ETHERDREAM_MIN, ETHERDREAM_MAX);
        framePoint->y = ofMap(laserPoint.y,800,0,ETHERDREAM_MIN, ETHERDREAM_MAX); // Y is UP
        framePoint->r = laserPoint.r/255.0f*65535;
        framePoint->g = laserPoint.g/255.0f*65535;
        framePoint->b = laserPoint.b/255.0f*65535;
        framePoint->i = 0;
        framePoint->u1 = 0;
        framePoint->u2 = 0;
        framePoints.push_back(framePoint);
    }
    
    void clear() {
        for(EtherDreamDacPoint* point : framePoints) {
            EtherDreamDacPointFactory :: releasePoint(point);
        }
        framePoints.clear();
        repeatCount = 1;
        
    }
    int getNumPoints() {
        return framePoints.size()*repeatCount;
    } 
    vector<EtherDreamDacPoint*> framePoints;
    uint64_t frameTime;
    int repeatCount = 1; // number of times to repeat the frame
   
    
};


}

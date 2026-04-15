//
//  ofxLaserDacEtherDream.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#pragma once

#include "ofxLaserPointFactory.h"


namespace ofxLaser {

class DacFrame {
    public :
    
    DacFrame(uint64_t time);
    ~DacFrame() ;
    void setTime(uint64_t time) ;
    
    void addPoint(const ofxLaser::Point& laserPoint);
    
    void clear();
    int getNumPoints();
    int getNumPointsForSingleRepeat();
    vector<Point*> framePoints;
    uint64_t frameTime;
    int repeatCount = 1; // number of times to repeat the frame
   
    static int useCount;
    
};



}

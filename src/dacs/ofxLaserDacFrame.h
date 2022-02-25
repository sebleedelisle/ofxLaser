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
    
    DacFrame(uint64_t time) {
        setTime(time); 
    }
    ~DacFrame() {
        clear();
    }
    void setTime(uint64_t time) {
        frameTime = time;
    }
    
    void addPoint(const ofxLaser::Point& laserPoint) {
 
        Point* framePoint = PointFactory :: getPoint(laserPoint);
        framePoints.push_back(framePoint);
    }
    
    void clear() {
        for(Point* point : framePoints) {
            PointFactory :: releasePoint(point);
        }
        framePoints.clear();
        repeatCount = 1;
    }
    int getNumPoints() {
        return framePoints.size()*repeatCount;
    }
    vector<Point*> framePoints;
    uint64_t frameTime;
    int repeatCount = 1; // number of times to repeat the frame
   
    
};


}

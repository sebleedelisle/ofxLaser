//
//  ofxLaserDacEtherDream.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#include "ofxLaserDacFrame.h"


int ofxLaser::DacFrame::useCount = 0;


ofxLaser::DacFrame::DacFrame(uint64_t time) {
    setTime(time);
    useCount++;
}
ofxLaser::DacFrame::~DacFrame() {
    clear();
    useCount--;
    //ofLogNotice("~DacFrame() ") << useCount;
}
void ofxLaser::DacFrame::setTime(uint64_t time) {
    frameTime = time;
}

void ofxLaser::DacFrame::addPoint(const ofxLaser::Point& laserPoint) {

    Point* framePoint = PointFactory :: getPoint(laserPoint);
    framePoints.push_back(framePoint);
}

void ofxLaser::DacFrame::clear() {
    for(Point* point : framePoints) {
        PointFactory :: releasePoint(point);
    }
    framePoints.clear();
    repeatCount = 1;
}
int ofxLaser::DacFrame::getNumPoints() {
    return framePoints.size()*repeatCount;
}
int ofxLaser::DacFrame::getNumPointsForSingleRepeat() {
    return framePoints.size();
}

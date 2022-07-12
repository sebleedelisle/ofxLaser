//
//  ofxLaserFactory.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 15/01/2019.
//
//

#include "ofxLaserPointFactory.h"

using namespace ofxLaser;

// static class members
// allocate Point objects. Theoretically 100k points per second per laser
// x 100 lasers x 10 seconds
Poco::ObjectPool<ofxLaser::Point> ofxLaser::PointFactory::pointObjectPool(100000000,100000000);

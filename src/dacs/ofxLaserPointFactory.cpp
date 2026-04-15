//
//  ofxLaserPointFactory.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 15/01/2019.
//
//

#include "ofxLaserPointFactory.h"

using namespace ofxLaser;

std::vector<ofxLaser::Point*> PointFactory::pool;
std::mutex PointFactory::poolMutex;

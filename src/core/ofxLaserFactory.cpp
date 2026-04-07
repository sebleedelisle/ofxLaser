//
//  ofxLaserFactory.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 15/01/2019.
//
//

#include "ofxLaserFactory.h"

using namespace ofxLaser;

std::vector<ofPolyline*> Factory::pool;
std::mutex Factory::poolMutex;

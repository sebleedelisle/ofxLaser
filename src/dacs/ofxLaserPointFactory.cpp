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

Poco::ObjectPool<ofxLaser::Point> ofxLaser::PointFactory::pointObjectPool(500000,10000000);

//
//  ofxLaserFactory.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 15/01/2019.
//
//

#include "ofxLaserFactory.h"

using namespace ofxLaser;

// static class members

Poco::ObjectPool<ofPolyline> ofxLaser::Factory::polylineObjectPool(500,10000000);

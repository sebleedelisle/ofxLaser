//
//  ofxLaserFactory.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 15/01/2019.
//
//

#include "ofxLaserDacEtherDreamDacPointFactory.h"

using namespace ofxLaser;

// static class members

Poco::ObjectPool<EtherDreamDacPoint> ofxLaser::EtherDreamDacPointFactory::pointObjectPool(500000,1000000);

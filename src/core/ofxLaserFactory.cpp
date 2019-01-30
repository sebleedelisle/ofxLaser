//
//  ObjectFactory.cpp
//  LaserLightSynths2019
//
//  Created by Seb Lee-Delisle on 15/01/2019.
//
//

#include "ofxLaserFactory.h"

using namespace ofxLaser;

//// static class members
//vector<ofPolyline*> ofxLaser::Factory::polylinePool;
//vector<ofPolyline*> ofxLaser::Factory::polylineSpares;

Poco::ObjectPool<ofPolyline> ofxLaser::Factory::polylineObjectPool(500,10000000);


//vector<ofxLaser::Polyline*> ofxLaser::Factory::laserPolylinePool;
//vector<ofxLaser::Polyline*> ofxLaser::Factory::laserPolylineSpares;

//
//  ofxLaserDacLaserDockNetData.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//
#pragma once


namespace ofxLaser {

struct LaserDockNetData {
    int hardwareRevision;
    int softwareRevision;
    int bufferCapacity;
    int maxPointRate;
    string macAddress;
    string ipAddress;
    float lastUpdateTime;
};
}

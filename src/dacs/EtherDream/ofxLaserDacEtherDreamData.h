//
//  ofxLaserDacEtherDreamData.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/02/2022.
//
#pragma once


namespace ofxLaser {

struct EtherDreamData {
    int hardwareRevision;
    int softwareRevision;
    int bufferCapacity;
    int maxPointRate;
    string macAddress = "";
    string ipAddress;
    bool inUse;
    double lastUpdateTime;

};
}

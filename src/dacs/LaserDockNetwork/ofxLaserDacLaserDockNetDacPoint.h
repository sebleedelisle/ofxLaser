//
//  ofxLaserDacLaserDockNetDacPoint.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//
#pragma once
#include "ofMain.h"

class LaserDockNetDacPoint {

    public :
    
    static int createCount;
    static int destroyCount;
    
    LaserDockNetDacPoint();
    ~LaserDockNetDacPoint();
    void clear();
    int16_t x = 0;
    int16_t y = 0;
    uint16_t r = 0;
    uint16_t g = 0;
    uint16_t b = 0; 
    
};
//

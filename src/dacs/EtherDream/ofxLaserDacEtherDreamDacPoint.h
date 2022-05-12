//
//  ofxLaserDacEtherDreamDacPoint.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/02/2022.
//
#pragma once
#include "ofMain.h"

class EtherDreamDacPoint {

    public :
    
    static int createCount;
    static int destroyCount;
    
    EtherDreamDacPoint();
    ~EtherDreamDacPoint();
    void clear();
    uint16_t control = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t r = 0;
    uint16_t g = 0;
    uint16_t b = 0;
    uint16_t i = 0;
    uint16_t u1 = 0;
    uint16_t u2 = 0;

 
    
};
//

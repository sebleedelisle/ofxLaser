//
//  ofxLaserDacEtherDreamDacPoint.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/02/2022.
//
#pragma once

class EtherDreamDacPoint {
//    public:
//    dac_point() {
//        dac_point::createCount++;
//    }
//    ~dac_point() {
//        destroyCount++;
//    }
    public :
    EtherDreamDacPoint() {
        control = 0;
        x = 0;
        y = 0;
        r = 0;
        g = 0;
        b = 0;
        i = 0;
        u1 = 0;
        u2 = 0;
        
    }
    uint16_t control = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t r = 0;
    uint16_t g = 0;
    uint16_t b = 0;
    uint16_t i = 0;
    uint16_t u1 = 0;
    uint16_t u2 = 0;

//    static int createCount;
//    static int destroyCount;
    
};


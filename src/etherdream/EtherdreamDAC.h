//
//  EtherdreamDAC.hpp
//  etherdreamTest
//
//  Created by Seb Lee-Delisle on 08/03/2017.
//
//

#pragma once

#include "ofMain.h"
#include "etherdream.h"
#include "ofxIldaPoint.h"

#define MAX_POINTS 60000

class EtherdreamDAC : public ofThread {
    public :
    
    void setup();
    
    void threadedFunction();
    void setPoints(const vector<ofxIlda::Point>& _points, int _pps);
    
    void fill_circle(float phase, int mode);
    uint16_t colorsin(float pos);

    struct etherdream_point pointsA[MAX_POINTS];
    struct etherdream_point pointsB[MAX_POINTS];
    struct etherdream_point* pointsToSend;
    struct etherdream_point* pointsBuffer;
    int pps;
    int numPointsToSend;
    int numPointsInBuffer;
    bool pointsChanged; 
    
    
};

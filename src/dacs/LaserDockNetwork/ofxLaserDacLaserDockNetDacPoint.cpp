//
//  ofxLaserDacLaserDockNetDacPoint.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//
#include "ofxLaserDacLaserDockNetDacPoint.h"


int LaserDockNetDacPoint :: createCount = 0;
int LaserDockNetDacPoint :: destroyCount = 0;

LaserDockNetDacPoint :: LaserDockNetDacPoint() {
    clear();
    createCount++;
    
}

LaserDockNetDacPoint :: ~LaserDockNetDacPoint() {
    destroyCount++;
}

void LaserDockNetDacPoint :: clear() {
    x = 0;
    y = 0;
    r = 0;
    g = 0;
    b = 0;
}

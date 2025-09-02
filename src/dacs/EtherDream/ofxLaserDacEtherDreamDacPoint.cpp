//
//  ofxLaserDacEtherDreamDacPoint.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/02/2022.
//
#include "ofxLaserDacEtherDreamDacPoint.h"


int EtherDreamDacPoint :: createCount = 0;
int EtherDreamDacPoint :: destroyCount = 0;

EtherDreamDacPoint :: EtherDreamDacPoint() {
    clear();
    createCount++;
    
}

EtherDreamDacPoint :: ~EtherDreamDacPoint() {
    destroyCount++;
}

void EtherDreamDacPoint :: clear() {
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

//
//  ofxLaserDacLaserDockNetCommand.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//

#include "ofxLaserDacLaserDockNetCommand.h"

using namespace ofxLaser;

void DacLaserDockNetCommand :: clear()  {
    ByteBuffer :: clear();
    numPoints = 0;
}

void DacLaserDockNetCommand :: setCommand(char command) {
    clear();
    appendChar(command);
}

void DacLaserDockNetCommand :: setDataCommand (uint16_t numpoints) {
    clear();
    appendChar('d');
    appendUInt16(numpoints);
    numPointsExpected = numpoints;
    
}
void DacLaserDockNetCommand :: addPoint(LaserDockNetDacPoint& p) {
    appendUInt16(p.control);
    appendInt16(p.x);
    appendInt16(p.y);
    appendUInt16(p.r);
    appendUInt16(p.g);
    appendUInt16(p.b);
    appendUInt16(p.i);
    appendUInt16(p.u1);
    appendUInt16(p.u2);
    numPoints++;
}


void DacLaserDockNetCommand :: setBeginCommand(uint32_t pointRate) {
    clear();
    appendChar('b');
    appendUInt16(0);
    appendUInt32(pointRate);
    
}
void DacLaserDockNetCommand :: setPointRateCommand(uint32_t pointRate) {
    clear();
    appendChar('q');
    appendUInt32(pointRate);
    
}

void DacLaserDockNetCommand :: logData() {
    resetReadIndex();
    
    char command = readChar();
    printf( "---------------------------------------------------------------\n");
    cout << "command            :" << command << endl;
   
    if(command != 'd') {
        return;
        
    }
    
    int numpoints = readUInt16(); //bytesToUInt16(&outbuffer[1]);
    printf("num points         : %d\n", numpoints);
    
    for(int i = 0; i<numpoints; i++) {
        
        printf("%03d - " ,i );
        printf(" ctl : %s ", (std::bitset<16>(readUInt16()).to_string().c_str())) ;
        printf(" x   : %06d ",readInt16()) ;
        printf(" y   : %06d ",readInt16()) ;
        printf(" rgb   : %04X%04X%04X " ,readUInt16(),readUInt16(),readUInt16()) ;
        printf(" i   : %d ",readUInt16()) ;
        printf(" u1   : %d ",readUInt16()) ;
        printf(" u2   : %d \n",readUInt16()) ;
       
    }
    printf("size : %zu readIndex : %d", size(), readIndex);
    
        
        
        
    
}

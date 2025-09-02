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
void DacLaserDockNetCommand :: setPointRateCommand (uint32_t newrate){
    clear();
    appendUInt8(DacLaserDockNetConsts :: CMD_SET_ILDA_RATE);
    appendUInt32(newrate);
    
}



void DacLaserDockNetCommand :: setDataCommand (uint8_t messagenum, uint8_t framenum) {
    clear();
    appendUInt8(DacLaserDockNetConsts :: CMD_SAMPLE_DATA);
    appendUInt8(0x00);
    appendUInt8(messagenum);
    appendUInt8(framenum);  // frame number, not currently used
}
void DacLaserDockNetCommand :: addPoint(LaserDockNetDacPoint& p) {
   
    appendUInt16(p.x);
    appendUInt16(p.y);
    appendUInt16(p.r);
    appendUInt16(p.g);
    appendUInt16(p.b);
    numPoints++;
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

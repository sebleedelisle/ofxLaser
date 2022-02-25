//
//  ofxLaserDacEtherDreamCommand.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/02/2022.
//

#include "ofxLaserDacEtherDreamCommand.h"

using namespace ofxLaser;

void DacEtherDreamCommand :: clear()  {
    ByteBuffer :: clear();
    numPoints = 0;
}

void DacEtherDreamCommand :: setCommand(char command) {
    clear();
    appendChar(command);
}

void DacEtherDreamCommand :: setDataCommand (uint16_t numpoints) {
    clear();
    appendChar('d');
    appendUInt16(numpoints);
    numPointsExpected = numpoints;
    
}
void DacEtherDreamCommand :: addPoint(EtherDreamDacPoint& p) {
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


void DacEtherDreamCommand :: setBeginCommand(uint32_t pointRate) {
    clear();
    appendChar('b');
    appendUInt16(0);
    appendUInt32(pointRate);
    
}
void DacEtherDreamCommand :: setPointRateCommand(uint32_t pointRate) {
    clear();
    appendChar('q');
    appendUInt32(pointRate);
    
}

void DacEtherDreamCommand :: logData() {
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

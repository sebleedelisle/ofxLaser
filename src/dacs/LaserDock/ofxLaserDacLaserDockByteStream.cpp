//
//  ofxLaserDacEtherDreamCommand.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/02/2022.
//

#include "ofxLaserDacLaserDockByteStream.h"

using namespace ofxLaser;

void DacLaserdockByteStream :: clear()  {
    ByteBuffer :: clear();
    numPoints = 0;
}

void DacLaserdockByteStream :: addPoint(LaserdockSample& p) {
   
    appendUInt16(p.rg);
    appendUInt16(p.b);
    appendUInt16(p.x);
    appendUInt16(p.y);
    numPoints++;
}


void DacLaserdockByteStream :: logData() {
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

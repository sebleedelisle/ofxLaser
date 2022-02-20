//
//  ofxLaserDacEtherDreamCommand.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/02/2022.
//

#pragma once
#include "ByteBuffer.h"
#include "ofxLaserDacEtherDreamDacPoint.h"

namespace ofxLaser {
class DacEtherDreamCommand : public ByteBuffer {
 
    public :
    
    void clear() override {
        ByteBuffer :: clear();
        numPoints = 0;
    }
    
    void setCommand(char command) {
        clear();
        appendChar(command);
    }
    
    void setDataCommand (uint16_t numpoints) {
        clear();
        appendChar('d');
        appendUInt16(numpoints);
        numPointsExpected = numpoints;
        
    }
    void addPoint(EtherDreamDacPoint& p) {
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
    
    
    void setBeginCommand(uint32_t pointRate) {
        clear();
        appendChar('b');
        appendUInt16(0);
        appendUInt32(pointRate);
        
    }
    void setPointRateCommand(uint32_t pointRate) {
        clear();
        appendChar('q');
        appendUInt32(pointRate);
        
    }
    
    void logData() {
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
            
            printf("------------------------------ npoint # %d\n" ,i );
            printf(" ctl : %s\n", (std::bitset<16>(readUInt16()).to_string().c_str())) ;
            printf(" x   : %d\n",readInt16()) ;
            printf(" y   : %d\n",readInt16()) ;
            printf(" r   : %04X\n" ,readUInt16()) ;
            printf(" g   : %04X\n",readUInt16()) ;
            printf(" b   : %04X\n" ,readUInt16()) ;
            printf(" i   : %d\n",readUInt16()) ;
            printf(" u1   : %d\n",readUInt16()) ;
            printf(" u2   : %d\n",readUInt16()) ;
           
        }
        printf("size : %zu readIndex : %d", size(), readIndex);
        
            
            
            
        
    }
    
    
    int numPointsExpected = 0;
    int numPoints = 0;
    
} ;

}

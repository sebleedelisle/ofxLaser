//
//  ByteBuffer.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 03/02/2022.
//
#pragma once
#include "ofMain.h"

class ByteBuffer {
    
    public :
    ByteBuffer();
    
    void appendChar(char n);
    void appendUInt8(uint8_t n);
    void appendUInt16(uint16_t n);
    void appendInt16(int16_t& n);
    void appendUInt32(uint32_t& n);
    
    char readChar();
    uint16_t readUInt16();
    int16_t readInt16();
    uint32_t readUInt32();
    virtual void clear() ;
    void resetReadIndex();
    
    uint8_t* getBuffer();
    
    uint8_t& operator[](int i)
    {
        if (i >= 100000) {
            cout << "Array index out of bound, exiting";
            //exit(0);
        }
        return buffer[i];
    }
    size_t size() { return index; };
    
    uint8_t buffer[100000];
    size_t index = 0;
    
    unsigned int readIndex =0;
    
   
    
};

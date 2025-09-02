//
//  ByteBuffer.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 03/02/2022.
//
#pragma once
#include "ofMain.h"

class ByteStreamUtils {
    
    public :
    
    // least significant bit first
    static uint16_t bytesToUInt16(unsigned char* byteaddress) {
        return (uint16_t)(*(byteaddress+1)<<8)|*byteaddress;

    }
    
    // least significant bit first
    static int16_t bytesToInt16(unsigned char* byteaddress) {
        int16_t i = *(signed char *)(byteaddress);
        i *= 1 << CHAR_BIT;
        i |= (*byteaddress+1);
        return i;

    }
    // least significant bit first
    static uint32_t bytesToUInt32(unsigned char* byteaddress){
        return (uint32_t)(*(byteaddress+3)<<24)|(*(byteaddress+2)<<16)|(*(byteaddress+1)<<8)|*byteaddress;

    }
    
};

//
//  ByteBuffer.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 03/02/2022.
//
#pragma once

class ByteBuffer {
    
    public :
    ByteBuffer(){
        index =0;
    };
    
    void appendChar(char n) {
        buffer[index++] = n;
        
    }
    void appendUInt16(uint16_t& n){
        buffer[index++] = n&0xff;
        buffer[index++] = n>>8;
    }
    void appendInt16(int16_t& n){
        buffer[index++] = n&0xff;
        buffer[index++] = n>>8;
    }
    void appendUInt32(uint32_t& n){
       
        buffer[index++] = n&0xff;
        buffer[index++] = (n>>8) & 0xff;
        buffer[index++] = (n>>16) & 0xff;
        buffer[index++] = (n>>24) & 0xff;
       
    }
    
    char readChar() {
        return (char)buffer[++readIndex];
    }
    uint16_t readUInt16() {
        uint16_t n = buffer[++readIndex];
        n |= (buffer[++readIndex]<<8);
        return n;
    }
    uint16_t readInt16() {
        int16_t n = (signed char)(buffer[++readIndex]);
        n *= 1 << CHAR_BIT;
        n |= buffer[++readIndex];
        return n;
    }
    uint32_t readUInt32(){
        uint32_t n = buffer[++readIndex];
        n |= (buffer[++readIndex]<<8);
        n |= (buffer[++readIndex]<<16);
        n |= (buffer[++readIndex]<<24);
        return n;
        
    }
    void clear() {
        index = 0;
    }
    void resetReadIndex() {
        readIndex = 0;
    }
    
    uint8_t* getBuffer() { return &buffer[0]; };
    uint8_t buffer[100000];
    unsigned int index = 0;
    unsigned int readIndex =0;
    
    uint8_t& operator[](int i)
    {
        if (i >= 100000) {
            cout << "Array index out of bound, exiting";
            //exit(0);
        }
        return buffer[i];
    }
    size_t size() { return index; };
    
};

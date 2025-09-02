//
//  ByteBuffer.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 03/02/2022.
//

#include "ByteBuffer.h"

ByteBuffer :: ByteBuffer(){
    index =0;
};

void ByteBuffer :: appendChar(char n) {
    buffer[index++] = n;
    
}
void ByteBuffer :: appendUInt8(uint8_t n){
    buffer[index++] = n;
}
void ByteBuffer :: appendUInt16(uint16_t n){
    buffer[index++] = n&0xff;
    buffer[index++] = n>>8;
}
void ByteBuffer :: appendInt16(int16_t& n){
    buffer[index++] = n&0xff;
    buffer[index++] = n>>8;
}
void ByteBuffer :: appendUInt32(uint32_t& n){
   
    buffer[index++] = n&0xff;
    buffer[index++] = (n>>8) & 0xff;
    buffer[index++] = (n>>16) & 0xff;
    buffer[index++] = (n>>24) & 0xff;
   
}

char ByteBuffer :: readChar() {
    return (char)buffer[readIndex++];
}
uint16_t ByteBuffer :: readUInt16() {
    uint16_t n = buffer[readIndex++];
    n |= (buffer[readIndex++]<<8);
    return n;
}
int16_t ByteBuffer :: readInt16() {
    uint16_t n = (signed char)(buffer[readIndex++]);
    n *= 1 << CHAR_BIT;
    n |= buffer[readIndex++];
    return n;
}
uint32_t ByteBuffer :: readUInt32(){
    uint32_t n = buffer[readIndex++];
    n |= (buffer[readIndex++]<<8);
    n |= (buffer[readIndex++]<<16);
    n |= (buffer[readIndex++]<<24);
    return n;
    
}
void ByteBuffer :: clear() {
    index = 0;
}
void ByteBuffer :: resetReadIndex() {
    readIndex = 0;
}

uint8_t* ByteBuffer :: getBuffer() {
    return &buffer[0];
    
};


//
//  ofxLaserDacEtherDreamResponse.h
//
//  Created by Seb Lee-Delisle on 15/02/2022.
//
#pragma once

#include "ByteStreamUtils.h"

#define LIGHT_ENGINE_READY    0
#define LIGHT_ENGINE_WARMUP      1
#define LIGHT_ENGINE_COOLDOWN 2
#define LIGHT_ENGINE_ESTOP    3

#define ETHERDREAM_PLAYBACK_IDLE 0
#define ETHERDREAM_PLAYBACK_PREPARED 1
#define ETHERDREAM_PLAYBACK_PLAYING 2



namespace ofxLaser {


class DacEtherDreamStatus {
    public :
    uint8_t protocol;
    uint8_t light_engine_state;
    uint8_t playback_state;
    uint8_t source;
    uint16_t light_engine_flags;
    uint16_t playback_flags;
    uint16_t source_flags;
    uint16_t _buffer_fullness;
    uint32_t point_rate;
    uint32_t point_count;
    
    const string light_engine_states[4] = {"ready", "warmup", "cooldown", "emergency stop"};
    const string playback_states[3] = {"idle", "prepared", "playing"};
   
    
    void deserialize(unsigned char* buffer) {
        protocol = buffer[0];
        light_engine_state = buffer[1];
        playback_state = buffer [2];
        source = buffer[3];
        light_engine_flags =  ByteStreamUtils::bytesToUInt16(&buffer[4]);
        playback_flags =   ByteStreamUtils::bytesToUInt16(&buffer[6]);
        source_flags =   ByteStreamUtils::bytesToUInt16(&buffer[8]);
        _buffer_fullness =  ByteStreamUtils::bytesToUInt16(&buffer[10]);
        point_rate =  ByteStreamUtils::bytesToUInt32(&buffer[12]);
        point_count =  ByteStreamUtils::bytesToUInt32(&buffer[16]);
    }
    
    string toString() {
        string returnstring = "";
        returnstring+= "protocol           : " + to_string(protocol) + "\n";
        returnstring+= "light_engine_state : " + light_engine_states[light_engine_state]+" "+to_string(light_engine_state) + "\n";
        returnstring+= "playback_state     : " + playback_states[playback_state]+" "+to_string(playback_state) + "\n";
        returnstring+= "source             : " + to_string(source) + "\n";
        returnstring+= "light_engine_flags : " + std::bitset<5>(light_engine_flags).to_string() + "\n";
        returnstring+= "playback_flags     : " + std::bitset<3>(playback_flags).to_string() + "\n";
        returnstring+= "   shutter         : " ;
        returnstring += (playback_flags & 0b100) ? "closed\n" : "open\n";
        returnstring+= "   underflow       : " ;
        returnstring += (playback_flags & 0b010) ? "TRUE\n" : "false\n";
        returnstring+= "   e-stop          : ";
        returnstring+= (playback_flags & 0b001) ? "TRUE\n" : "false\n";

        returnstring+= "source_flags       : " + to_string(source_flags) + "\n";
        returnstring+= "buffer_fullness    : " + to_string(_buffer_fullness) + "\n";
        returnstring+= "point_rate         : " + to_string(point_rate) + "\n";
        returnstring+= "point_count        : " + to_string(point_count) + "\n";
        
        return returnstring;
        
    }
    

};
class DacEtherDreamResponse {
    public :
    
    uint8_t response;
    uint8_t command;
    DacEtherDreamStatus status;
    void deserialize(unsigned char* buffer) {
        response = buffer[0];
        command = buffer[1];
        status.deserialize(&buffer[2]);
        
    }
    
    string toString() {
        string data = "";
        data += "response        : " + ofToString(response) + "\n";
        data += "command         : " + ofToString(command);
        data+= status.toString();
        return data; 
        
    }
    
};
}

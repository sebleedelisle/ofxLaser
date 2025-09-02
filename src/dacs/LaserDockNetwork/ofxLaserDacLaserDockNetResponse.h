//
//  ofxLaserDacLaserDockNetResponse.h
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//
#pragma once

#include "ByteStreamUtils.h"



namespace ofxLaser {


class DacLaserDockNetStatus {
    public :
    
    uint8_t payload_version = 0;
    uint8_t firmware_major = 0;
    uint8_t firmware_minor = 0;
    string firmware_version;
    uint8_t status_flags = 0;
    
    bool output_enabled = 0;
    bool interlock_enabled = 0;
    bool temperature_warning = 0;
    bool over_temperature = 0;
    uint8_t packet_errors = 0;
    
    uint32_t point_rate = 0;
    uint32_t point_rate_max = 0;
    
    uint16_t buffer_free = 0;
    uint16_t buffer_max = 0;
    uint8_t battery_percent = 0;
    uint8_t temperature = 0;
    uint8_t connection_type = 0;
    
    string serial_number;
    
    string ip_address;
    uint8_t model_number = 0;
    string model_name;
    
    float lastUpdateTime = 0;
    
    
    void deserialize(unsigned char* buffer) {
        
        // so the response should be 64 bytes in the format :
        // 0 not used ?
        // 1 not used ?
        // 2 payload version id (should be 0, just allows for future changes)
        // 3 firmware version major
        // 4 firmware version minor
        // 5 flags byte
        //      oe = flats & 1
        
        //      firmware <= 0.12
        //          ilock = flags & 8
        //          twarn = flags & 16
        //          otemp = flags & 32
        //      firmware >0.12
        //          ilock = flags & 2
        //          twarn = flags & 4
        //          otemp = flags & 8
        //          packeterrors = flags>>4 & 0x0f
        //
        // oe = output enabled
        // ilock = interlock enabled
        // twarn = temperature warning
        // otemp = over temperature
        //
        // 6-9 ???
        //
        // 10 DAC rate uint32_t (lsb)
        // 11 ..
        // 12 ..
        // 13 ..
        //
        // 14 MAX DAC rate uint32_t (lsb)
        // 15 ..
        // 16 ..
        // 17 ..
        //
        // 18 ?
        //
        // 19 Free buffer uint16_t
        // 20 ..
        // 21 Buffer size uint16_t
        // 22 ..
        
        // 23 Battery percent byte
        // 24 Temperature byte
        // 25 Connection type << What is this? Wifi / Wire?
        //
        // 26 Serial number - not sure of format, seems like 12 hex digits
        // 27 converted to a string. 6 bytes.
        // 28
        // 29
        // 30
        // 31
        //
        // 32 IP address, 4 bytes
        // 33
        // 34
        // 35
        //
        // 37 Model number (byte)
        //
        // 38 Model name (char array, null terminated)
        // 39 ...
        
        
        
        payload_version = buffer[2];
        if(payload_version == 0) {
            firmware_major = buffer[3];
            firmware_minor = buffer[4];
            firmware_version = to_string(firmware_major) + "." + to_string(firmware_minor);
            
            status_flags = buffer[5];
           
            // below code from laserdocklib... not really sure why they
            // need the ternaries - the binary operations should return true
            // or false, otherwise the ternaries wouldn't work!
            output_enabled = (status_flags & 1) ? true : false;
           
            if ((firmware_major>0) || (firmware_minor>=13)) {
                interlock_enabled = (status_flags & 2) ? true : false;
                temperature_warning = (status_flags & 4) ? true : false;
                over_temperature = (status_flags & 8) ? true : false;
                packet_errors = (status_flags>>4) & 0x0f;
            } else { // <=v0.12
                interlock_enabled = (status_flags & 8) ? true : false;
                temperature_warning = (status_flags & 16) ? true : false;
                over_temperature = (status_flags & 32) ? true : false;
            }
            
            point_rate = ByteStreamUtils::bytesToUInt32(&buffer[10]);
            point_rate_max = ByteStreamUtils::bytesToUInt32(&buffer[14]);
            
            
            buffer_free = ByteStreamUtils::bytesToUInt16(&buffer[19]);
            buffer_max = ByteStreamUtils::bytesToUInt16(&buffer[21]);
            battery_percent  = buffer[23];
            temperature = buffer[24];
            connection_type = buffer[25]; // NOT sure what this is!
            
            serial_number = ofToString(ofToHex(buffer[26]), 2, '0') +
                ofToString(ofToHex(buffer[27]), 2, '0') +
                ofToString(ofToHex(buffer[28]), 2, '0') +
                ofToString(ofToHex(buffer[29]), 2, '0') +
                ofToString(ofToHex(buffer[30]), 2, '0') +
                ofToString(ofToHex(buffer[31]), 2, '0');
            serial_number = ofToUpper(serial_number);
            
            ip_address  = to_string(buffer[32]) + "." +
                to_string(buffer[33]) + "." +
                to_string(buffer[34]) + "." +
                to_string(buffer[35]);
                            
            model_number = buffer[37];
            model_name = (char*) (&buffer[38]);
            
        } else {
            // invalid payload version!
            ofLogError("ofxLaserDacDockNetResponse :: deserialize - future payload version! Code needs update! ");
        }
        lastUpdateTime = ofGetElapsedTimef();

    }
    
    string toString() {
        string returnstring = "" \
        "payload_version      : " + to_string(payload_version) +"\n" +
        "firmware_major       : " + to_string(firmware_major) +"\n" +
        "firmware_minor       : " + to_string(firmware_minor) +"\n" +
        "firmware_version     : " + firmware_version +"\n" +
        
        
        "output_enabled       : " + to_string(output_enabled) +"\n" +
        "interlock_enabled    : " + to_string(interlock_enabled) +"\n" +
        "temperature_warning  : " + to_string(temperature_warning) +"\n" +
        "over_temperature     : " + to_string(over_temperature) +"\n" +
        "packet_errors        : " + to_string(packet_errors) +"\n" +
        
        "point_rate           : " + to_string(point_rate) +"\n" +
        "point_rate_max       : " + to_string(point_rate_max) +"\n" +
        
        "buffer_free          : " + to_string(buffer_free) +"\n" +
        "buffer_max           : " + to_string(buffer_max) +"\n" +
        "battery_percent      : " + to_string(battery_percent) +"\n" +
        "temperature          : " + to_string(temperature) +"\n" +
        "connection_type      : " + to_string(connection_type) +"\n" +
        
        "serial_number        : " + serial_number +"\n" +
        
        "ip_address           : " + ip_address +"\n" +
        "model_number         : " + to_string(model_number) +"\n" +
        "model_name           : " + model_name +"\n";
        
        
        return returnstring;
        
    }
    bool operator== (DacLaserDockNetStatus &rhs){
        return this->toString() == rhs.toString();
    }
    bool operator!= (DacLaserDockNetStatus &rhs){
        return this->toString() != rhs.toString();
    }
    

};
class DacLaserDockNetResponse {
    public :
    
    uint8_t command;
    uint8_t messageNum;
    uint16_t bufferSize;
    
  
    void deserialize(unsigned char* buffer) {
        
        command = buffer[0];
        messageNum = buffer[1];
        bufferSize = ByteStreamUtils::bytesToUInt16(&buffer[2]);
        
    }
    
    string toString() {
        string data = "";
        data += "command    : " + ofToString(command) + "\n";
        data += "messageNum : " + ofToString(messageNum);
        data += "bufferSize : " + ofToString(bufferSize);
      
        return data; 
        
    }
    
};
}

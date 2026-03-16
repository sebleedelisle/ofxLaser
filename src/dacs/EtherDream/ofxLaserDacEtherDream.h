//
//  ofxLaserDacEtherDream.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/11/2017.
//
//

#pragma once
#include "ofxLaserDacBase.h"
#include <optional>

#include "ofxLaserDacEtherDreamData.h"
#include "libera/etherdream/EtherDreamDevice.hpp"
#include "libera/etherdream/EtherDreamDeviceInfo.hpp"


namespace ofxLaser {


class DacEtherDream : public DacBase {

public:
    DacEtherDream();
    ~DacEtherDream();

    // DacBase functions
    bool sendFrame(const vector<Point>& points) override;
    bool setPointsPerSecond(uint32_t newpps) override;
    uint32_t getPointsPerSecond() override { return pps; }
    bool setColourShift(float shiftSeconds) override;
    void setArmed(bool armed) override;
    bool isReadyForFrame(int maxLatencyMS) override;
    
    virtual string getType() override { return "EtherDream"; };
    
    string getRawId() override;
    string getEdId(); 
    int getStatus() override;
    const vector<ofAbstractParameter*>& getDisplayData() override;
   
    void setup(string id, string ip, EtherDreamData& ed);
    
    void closeWhileRunning();
    void close() override;
    void reset() override;
   
    int getDacTotalPointBufferCapacity() { return dacTotalPointBufferCapacity; }
   
    // information about the device, IP address, MAC address, version number etc
    EtherDreamData etherDreamData;
    
    EtherDreamData getEtherDreamData(); 
    string getEtherDreamStateString();
    
    string getHardwareName() {
        if(!isMercury) {
            string name = "Ether Dream v";
            if(versionNumber == 3) return name +"3/4";
            else return name + ofToString(versionNumber);

        } else {
            return "Mercury";
        }
        
    }
    
    
protected:
    // the maximum number of points the etherdream can hold.
    std::atomic<int> dacTotalPointBufferCapacity;
    std::atomic<uint32_t> pps{30000};

    string ipAddress;
    string id;

    int versionNumber;
    string versionString;
    
    bool isMercury = false; 
    std::atomic<bool> networkConnected;

    // libera adapter
    std::shared_ptr<libera::etherdream::EtherDreamDevice> liberaDevice;
    std::optional<libera::etherdream::EtherDreamDeviceInfo> liberaInfo;
    libera::core::Frame makeLiberaFrame(const vector<Point>& points);
    
};

}

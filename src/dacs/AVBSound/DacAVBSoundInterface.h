#pragma once

#include "ofMain.h"
#include <vector>
#include <memory>
#include <optional>

// the sound interface owns the dacs! 

namespace ofxLaser {

class DacAVBSound;
struct DacAVBSoundPoint;

class DacAVBSoundInterface {
public:
    DacAVBSoundInterface();
    ~DacAVBSoundInterface();
    
    bool setup(ofSoundDevice& device, std::optional<int> samplerate);
    bool setup(string devicename, std::optional<int> samplerate);
    
    void disconnect();
    
    bool initDacs(int sampleRate);
    const std::vector<std::shared_ptr<DacAVBSound>>& getDacs() {
        return dacs;
    }
    ofSoundDevice getDevice() {
        return device;
    }
    int getSampleRate() {
        return currentSampleRate;
    }
    bool setSampleRate(int newRate) {
        if(newRate != currentSampleRate) {
            return setup(device, newRate);
            
        } else {
            return false;
        }

    }
    
    bool getConnected() {
        if(ofGetElapsedTimeMillis()-lastUpdate>1000) connected = false;
        else connected = true;

        return connected;
    }
    void audioOut(ofSoundBuffer& buffer);
    
    
    void serialize(ofJson&json) const ;
    bool deserialize(ofJson&jsonGroup) ;
 
    
private:
    std::vector<std::shared_ptr<DacAVBSound>> dacs;
    ofSoundDevice device;
    std::atomic<bool> connected = false;
    int currentSampleRate; 
    //std::atomic<int> deviceIndex;
    //string interfaceName;
    ofSoundStream soundStream;
    uint64_t lastUpdate = 0; 
    
};
}

#pragma once

#include "ofMain.h"
#include <vector>
#include <memory>
//#include "DacAVBSound.h"

// the sound interface owns the dacs! 

namespace ofxLaser {

class DacAVBSound;
struct DacAVBSoundPoint;

class DacAVBSoundInterface {
public:
    DacAVBSoundInterface();
    ~DacAVBSoundInterface();
    
    bool setup(ofSoundDevice& device);
    bool setup(string devicename);
    
    void disconnect();
    
    bool initDacs();
    const std::vector<std::shared_ptr<DacAVBSound>>& getDacs() {
        return dacs;
    }
    ofSoundDevice getDevice() {
        return device;
    } 
    bool getConnected() {
        return connected;
    }
    void audioOut(ofSoundBuffer& buffer);
    
    
    void serialize(ofJson&json) const ;
    bool deserialize(ofJson&jsonGroup) ;
 
    
private:
    std::vector<std::shared_ptr<DacAVBSound>> dacs;
    ofSoundDevice device;
    std::atomic<bool> connected = false;
    //std::atomic<int> deviceIndex;
    //string interfaceName;
    ofSoundStream soundStream;
    
};
}

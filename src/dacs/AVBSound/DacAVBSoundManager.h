#pragma once

#include "ofxLaserDacManagerBase.h"
#include "DacAVBSoundInterface.h"


// Settings :
// List of sound interface names
// is it just a list of serialized sound interfaces ?


namespace ofxLaser {

class DacAVBSoundManager : public DacManagerBase {
public:
    DacAVBSoundManager();
    ~DacAVBSoundManager();

    std::vector<DacData> updateDacList() override;
    std::shared_ptr<DacBase> getAndConnectToDac(const std::string& id) override;
    std::string getType() override {
        return "AVB/Sound";
    }
    
    bool connectToInterface(ofSoundDevice& device, std::optional<int>samplerate);
    bool connectToInterface(const std::string& devicename, std::optional<int>samplerate = {});
    
    int getSampleRate(ofSoundDevice& device);
    bool setSampleRate(ofSoundDevice& device, int newSampleRate);
    
    std::shared_ptr<DacAVBSoundInterface> getInterfaceForDevice(const std::string& devicename); 
    bool getIsUsingDevice(const std::string& devicename);
    
    ofColor getStatusForDevice(const std::string& devicename);
    
    bool disconnectFromInterface(const std::string& devicename);
    
    bool disconnectAllInterfaces(const std::string& devicename); 
    
    void exit() override;

    
    
    virtual void serialize(ofJson&json) const override;
    virtual bool deserialize(ofJson&jsonGroup) override;
    
    protected :
    // for now just make one for AVB network
    vector<std::shared_ptr<DacAVBSoundInterface>> interfaces;
    
};

}

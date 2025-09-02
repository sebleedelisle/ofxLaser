#include "DacAVBSoundInterface.h"
#include "DacAVBSound.h"

namespace ofxLaser {

DacAVBSoundInterface::DacAVBSoundInterface() {
    
    //interfaceName = name;
//    ofLogNotice("DacAVBSoundInterface::DacAVBSoundInterface ") << device.name ;
//    start();
    
}

DacAVBSoundInterface::~DacAVBSoundInterface() {
    disconnect();
}


bool DacAVBSoundInterface::setup(ofSoundDevice& sounddevice) {
    
    device = sounddevice;
    
    if(device.sampleRates.size()==0) {
        return false;
    }
    
    ofSoundStreamSettings settings;
    settings.setOutListener(this);
    settings.numOutputChannels = device.outputChannels;
    settings.sampleRate = device.sampleRates[0];
    settings.bufferSize = 512;
    settings.numBuffers = 4;
    settings.setOutDevice(device);
   // settings.api =ofSoundDevice::Api::OSX_CORE;
    
    if(soundStream.setup(settings)) {
        ofLogNotice("AVB sound stream set up! ");
        connected = true;
        initDacs();
        return true;
    } else {
        ofLogNotice("AVB sound stream failed :( ");
        return false;
    }
}

bool DacAVBSoundInterface :: setup(string devicename) {
    
    std::vector<ofSoundDevice> devices = ofSoundStreamListDevices();
    for(ofSoundDevice& device : devices) {
        if (device.name == devicename) {
            return setup(device);  // Call the overload that takes an ofSoundDevice
        }
    }
    return false;
}
    
    



void DacAVBSoundInterface::disconnect() {
    soundStream.close();
    connected = false;
}

bool DacAVBSoundInterface :: initDacs() {
    
    int channelsPerDac = 8; // assume 8 channels per output
    
    if(!connected) {
        dacs = {};
    } else {
        int numdacs = soundStream.getNumOutputChannels()/channelsPerDac;
        int streamnum = 0;
        if (dacs.size()!=numdacs) {
            dacs = {};
            for(int channel = 0; channel< soundStream.getNumOutputChannels(); channel+=channelsPerDac) {
                // i suppose eventually we could add channel information to the DAC but let's just assume it's every 8!
                std::shared_ptr<DacAVBSound> dac = std::make_shared<DacAVBSound>();
                dac->setDacName(ofToString(streamnum));
                ofLogNotice("Setting dacname : ") << streamnum;
                ofLogNotice("Getting dacname : ") << dac->getFullId();
                dacs.push_back(dac);
                streamnum++;
            }
        }
    }
    return true;
    
}
void DacAVBSoundInterface::audioOut(ofSoundBuffer& buffer) {
    
    const int totalSamples = buffer.getNumFrames();
    const int numChannels = buffer.getNumChannels();
    const int laserChannels = 8;

    const uint64_t tickCount = buffer.getTickCount();

    static const bool enableLogging = false;
    static const int logEveryNTicks = 50;

    if (enableLogging && tickCount % logEveryNTicks == 0) {
        ofLogNotice("DacAVBSoundInterface")
            << "audioOut tick=" << tickCount
            << ", totalSamples=" << totalSamples
            << ", numChannels=" << numChannels
            << ", dacs=" << dacs.size();
    }

    vector<vector<std::shared_ptr<DacAVBSoundPoint>>> pointsByDac;

    for (size_t j = 0; j < dacs.size(); j++) {
        std::shared_ptr<DacAVBSound>& dac = dacs[j];
        auto points = dac->getNextPoints(totalSamples);
        if (enableLogging && tickCount % logEveryNTicks == 0 && points.size() != totalSamples) {
           // ofLogWarning("DacAVBSoundInterface")
           //     << "DAC " << j << " returned " << points.size() << " points (expected " << totalSamples << ")";
        }
        pointsByDac.push_back(std::move(points));
    }
    
    
    for (int i = 0; i < totalSamples; ++i) {
        int baseIndex = i * numChannels;
        for (size_t j = 0; j < pointsByDac.size(); j++) {
            if (pointsByDac[j].size() > i) {
                std::shared_ptr<DacAVBSoundPoint> p = pointsByDac[j][i];
                int dacIndex = baseIndex + (j * laserChannels);
                if (p) {
                    if((dacIndex + 7) > buffer.size()) {
                        // TO DO - warning?
                        break;
                    }
                    
                    buffer[dacIndex + 0] = p->x;
                    buffer[dacIndex + 1] = p->y;
                    buffer[dacIndex + 2] = p->r;
                    buffer[dacIndex + 3] = p->g;
                    buffer[dacIndex + 4] = p->b;
                    buffer[dacIndex + 5] = 0.0f;
                    buffer[dacIndex + 6] = 0.0f;
                    buffer[dacIndex + 7] = 0.0f;

                    // Log first few samples for first DAC only
                    if (enableLogging && j == 0 && i < 5 && tickCount % logEveryNTicks == 0) {
                        ofLogNotice("DacAVBSoundInterface")
                            << "Sample " << i
                            << " [DAC " << j << "]: x=" << p->x
                            << ", y=" << p->y
                            << ", r=" << p->r
                            << ", g=" << p->g
                            << ", b=" << p->b;
                    }
                }
            }
        }
    }
}


void DacAVBSoundInterface :: serialize(ofJson&json) const {
    
    json["devicename"] = device.name;
    
}
bool DacAVBSoundInterface :: deserialize(ofJson&jsonGroup) {
    disconnect();
    if(jsonGroup.contains("devicename")) {
        setup(jsonGroup["devicename"].get<string>());
        return true;
    } else {
        return false;
    }
}


}

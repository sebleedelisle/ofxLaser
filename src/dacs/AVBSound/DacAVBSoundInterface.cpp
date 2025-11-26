#include "DacAVBSoundInterface.h"
#include "DacAVBSound.h"
#include "DacAVBUtils.h"

namespace ofxLaser {

DacAVBSoundInterface::DacAVBSoundInterface() {
    
    //interfaceName = name;
//    ofLogNotice("DacAVBSoundInterface::DacAVBSoundInterface ") << device.name ;
//    start();
    
}

DacAVBSoundInterface::~DacAVBSoundInterface() {
    disconnect();
}


bool DacAVBSoundInterface::setup(ofSoundDevice& sounddevice, std::optional<int> samplerate) {
    
    device = sounddevice;
    
    if(device.sampleRates.size()==0) {
        ofLogError("No samplerates found for device!");
        return false;
    }
    

    int preferredRate = ofxLaser :: DacAVBUtils :: getPreferredSampleRateForDevice(device);
    if(samplerate) preferredRate = samplerate.value();
        
    ofSoundStreamSettings settings;
    settings.setOutListener(this);
    settings.numOutputChannels = device.outputChannels;
    settings.sampleRate = (preferredRate > 0) ? preferredRate : device.sampleRates[0];
    settings.bufferSize = 512;
    settings.numBuffers = 4;
    settings.setOutDevice(device);
    
    // Log available and chosen sample rates
    std::ostringstream ss;
    ss << "Available sample rates: ";
    for (size_t i = 0; i < device.sampleRates.size(); ++i) {
        ss << device.sampleRates[i];
        if (i < device.sampleRates.size() - 1) ss << ", ";
    }
    ofLogNotice("AudioSetup") << ss.str();
    ofLogNotice("AudioSetup") << "Preferred sample rate: " << preferredRate;
    ofLogNotice("AudioSetup") << "Using sample rate: " << settings.sampleRate;
    
   // settings.api =ofSoundDevice::Api::OSX_CORE;
    
    // make sure soundstream is closed
    soundStream.close();
    
    if(soundStream.setup(settings)) {
        ofLogNotice("AVB sound stream set up! ");
        connected = true;
        currentSampleRate = soundStream.getSampleRate();
        initDacs(currentSampleRate);
        return true;
    } else {
        ofLogNotice("AVB sound stream failed :( ");
        return false;
    }
}

bool DacAVBSoundInterface :: setup(string devicename, std::optional<int> samplerate) {
    
    std::vector<ofSoundDevice> devices = ofSoundStreamListDevices();
    for(ofSoundDevice& device : devices) {
        if (device.name == devicename) {
            return setup(device, samplerate);  // Call the overload that takes an ofSoundDevice
        }
    }
    return false;
}
    
    



void DacAVBSoundInterface::disconnect() {
    for(std::shared_ptr<DacAVBSound>& dac : dacs) {
        dac->setActive(false);
    }
    soundStream.close();
    connected = false;
}

bool DacAVBSoundInterface :: initDacs(int sampleRate) {
    
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
                std::shared_ptr<DacAVBSound> dac = std::make_shared<DacAVBSound>(sampleRate);
                dac->setDacName(ofToString(streamnum));
                ofLogNotice("Setting dacname : ") << streamnum;
                ofLogNotice("Getting dacname : ") << dac->getFullId();
                dac->setConnected(true);
                dac->setActive(false);
                dacs.push_back(dac);
                
                streamnum++;
            }
        }
    }
    return true;
    
}

void DacAVBSoundInterface::audioOut(ofSoundBuffer& buffer) {
    // Record the last time we pushed audio
    lastUpdate = ofGetElapsedTimeMillis();

    // How many frames the in the buffer
    const int totalSamples = buffer.getNumFrames();
    // Output channel count in the audio buffer
    const int numChannels = buffer.getNumChannels();
    // Channels per laser DAC in our layout: X, Y, R, G, B, I, U1, U2
    const int laserChannels = 8;

    // A running tick counter from ofSoundBuffer for occasional logging
    const uint64_t tickCount = buffer.getTickCount();

    // Debug logging controls
    static const bool enableLogging = false;
    static const int logEveryNTicks = 50;

    if (enableLogging && tickCount % logEveryNTicks == 0) {
        ofLogNotice("DacAVBSoundInterface")
            << "audioOut tick=" << tickCount
            << ", totalSamples=" << totalSamples
            << ", numChannels=" << numChannels
            << ", dacs=" << dacs.size();
    }


    std::vector<std::vector<std::shared_ptr<DacAVBSoundPoint>>> pointsByDac;
    pointsByDac.reserve(dacs.size());

    // A reusable zero point for inactive DACs or missing samples.
    // We treat points as read-only in the mixing loop, so reusing a single
    // shared instance is safe and avoids allocating thousands of objects.
    // If you prefer strict uniqueness, switch to creating per-sample instances.
    static const std::shared_ptr<DacAVBSoundPoint> kZeroPoint =
        std::make_shared<DacAVBSoundPoint>(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);

    // Helper to create a vector of size totalSamples filled with empty points
    auto makeEmptyPoints = [&](int count) {
        std::vector<std::shared_ptr<DacAVBSoundPoint>> v;
        v.resize(count, kZeroPoint);
        return v;
    };

    // Gather points from each DAC.
    // Important change: do NOT call dac->getNextPoints(...) if the DAC is inactive.
    // Instead, provide a buffer of empty points so downstream code can write zeros.
    for (size_t j = 0; j < dacs.size(); ++j) {
        std::shared_ptr<DacAVBSound>& dac = dacs[j];

        if (dac && dac->getActive()) {
            // Active DAC: fetch rendered points for this audio block
            auto points = dac->getNextPoints(totalSamples);

            if (enableLogging && tickCount % logEveryNTicks == 0 && static_cast<int>(points.size()) != totalSamples) {
                // Some DACs might underrun or overrun. We will guard when reading.
                // ofLogWarning("DacAVBSoundInterface")
                //     << "DAC " << j << " returned " << points.size() << " points (expected " << totalSamples << ")";
            }

            pointsByDac.push_back(std::move(points));
        } else {
            // Inactive or null DAC: fill with zeros without calling into the DAC
            pointsByDac.push_back(makeEmptyPoints(totalSamples));
        }
    }

    // Mix points into the audio buffer.
    // We write per-sample, per-DAC, into a contiguous bank of 8 channels per DAC.
    for (int i = 0; i < totalSamples; ++i) {
        const int baseIndex = i * numChannels;

        for (size_t j = 0; j < pointsByDac.size(); ++j) {
            // Defensive check in case a DAC returned fewer points than requested
            if (pointsByDac[j].size() <= static_cast<size_t>(i)) {
                // Not enough points for this sample from this DAC, treat as zero
                continue;
            }

            // Pointer can legally be null if a DAC uses sparse output.
            // We fallback to the zero point in that case.
            std::shared_ptr<DacAVBSoundPoint> p = pointsByDac[j][i];
            if (!p) {
                p = kZeroPoint;
            }

            // Compute where this DAC's 8-channel bank starts inside the interleaved buffer
            const int dacIndex = baseIndex + static_cast<int>(j) * laserChannels;

            // Bounds guard to avoid writing past the end of the buffer
            if ((dacIndex + 7) >= static_cast<int>(buffer.size())) {
                // Optional: warn once if layout is inconsistent
                // ofLogWarning("DacAVBSoundInterface") << "Buffer overrun risk. Check channel layout.";
                break;
            }
            if(p->x>1) ofLogNotice() << p->x << " " << p->y; 
            // Write the point into the audio buffer
            buffer[dacIndex + 0] = p->x;
            buffer[dacIndex + 1] = p->y;
            buffer[dacIndex + 2] = p->r;
            buffer[dacIndex + 3] = p->g;
            buffer[dacIndex + 4] = p->b;
            buffer[dacIndex + 5] = p->i;   // intensity
            buffer[dacIndex + 6] = p->u1;  // user/aux 1
            buffer[dacIndex + 7] = p->u2;  // user/aux 2

            // Log first few samples for the first DAC only
            if (enableLogging && j == 0 && i < 5 && tickCount % logEveryNTicks == 0) {
                ofLogNotice("DacAVBSoundInterface")
                    << "Sample " << i
                    << " [DAC " << j << "]: x=" << p->x
                    << ", y=" << p->y
                    << ", r=" << p->r
                    << ", g=" << p->g
                    << ", b=" << p->b
                    << ", i=" << p->i
                    << ", u1=" << p->u1
                    << ", u2=" << p->u2;
            }
        }
    }
}


void DacAVBSoundInterface :: serialize(ofJson&json) const {
    
    json["devicename"] = device.name;
    json["samplerate"] = currentSampleRate;
    
}
bool DacAVBSoundInterface :: deserialize(ofJson&jsonGroup) {
    disconnect();
    if(jsonGroup.contains("devicename")) {
        std::optional<int> samplerate;
        if(jsonGroup.contains("samplerate")) {
            samplerate = jsonGroup["samplerate"].get<int>();
        }
        setup(jsonGroup["devicename"].get<string>(), samplerate);
        return true;
    } else {
        return false;
    }
}


}

//
//  DACAVBUtils.cpp
//
//  Created by Seb Lee-Delisle on 26/11/2025.
//
#include "RtAudio.h"
#include "ofLog.h"
#include <algorithm>
#include <cmath>

#include "DacAVBUtils.h"

namespace ofxLaser {
namespace DacAVBUtils {

int getPreferredSampleRateForDevice(const ofSoundDevice& device) {
    RtAudio audio;
    RtAudio::DeviceInfo info;
    
    try {
        info = audio.getDeviceInfo(device.deviceID);
    } catch (RtAudioError& e) {
        ofLogError() << "SoundInputManager :: getPreferredSampleRateForDevice : Failed to get device info: " << e.getMessage();
        return -1;
    }
    
    if (!info.probed || device.sampleRates.empty()) {
        ofLogError() << "SoundInputManager :: getPreferredSampleRateForDevice : Device info could not be probed or has no sample rates.";
        return -1;
    }
    
    int preferredRate = info.preferredSampleRate;
    
    // Check if preferred sample rate is supported
    if (std::find(device.sampleRates.begin(), device.sampleRates.end(), preferredRate) != device.sampleRates.end()) {
        return preferredRate;
    }
    
    // Fallback: find closest to 44100
    int fallbackTarget = 44100;
    auto it = std::min_element(
                               device.sampleRates.begin(),
                               device.sampleRates.end(),
                               [fallbackTarget](int a, int b) {
                                   return std::abs(a - fallbackTarget) < std::abs(b - fallbackTarget);
                               }
                               );
    
    if (it != device.sampleRates.end()) {
        ofLogWarning() << "SoundInputManager :: getPreferredSampleRateForDevice : Preferred rate not supported, using closest to 44100: " << *it;
        return *it;
    } else {
        ofLogError() << "SoundInputManager :: getPreferredSampleRateForDevice : No usable sample rates found.";
        return -1;
    }
}


}
}

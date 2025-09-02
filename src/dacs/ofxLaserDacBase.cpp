
#include "ofxLaserDacBase.h"
using namespace ofxLaser;

void DacBase::setArmed(bool _armed){
    armed = _armed;
    if(armed) {
        blankPointsAfterReArmRemaining = blankTimeAfterReArm*getPointsPerSecond();
    }
};
const vector<ofAbstractParameter*>& DacBase::getDisplayData() {
    return displayData;
    
};

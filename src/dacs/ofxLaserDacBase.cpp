
#include "ofxLaserDacBase.h"
using namespace ofxLaser;

void DacBase::setArmed(bool _armed){
   armed = _armed;
};
const vector<ofAbstractParameter*>& DacBase::getDisplayData() {
    return displayData;
    
};

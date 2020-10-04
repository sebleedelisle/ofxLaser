
#include "ofxLaserDacBase.h"
using namespace ofxLaser;

void DacBase::setActive(bool active){
   armed = active;
};
const vector<ofAbstractParameter*>& DacBase::getDisplayData() { return displayData;};

//
//  ofxLaserDacAssigner.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//
// The DacAssigner is a complex object that is responsible for keeping track
// of available DACs and which projectors they are connected to.
//
// It keeps a collection of DacManagers, one for each type of DAC that we
// support. The DAC objects themselves are owned by the managers, but
// the DacAssigner is responsible for giving and taking them from
// projectors.

#pragma once

#include "ofMain.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserProjector.h"
#include "ofxLaserDacData.h"
#include "ofxLaserDacManagerBase.h"
#include "ofxLaserDacManagerLaserdock.h"
#include "ofxLaserDacManagerEtherdream.h"
#include "ofxLaserDacManagerHelios.h"

namespace ofxLaser {

class DacAssigner {
    
    public :
    
    // it's a Singleton so shouldn't ever have more than one.
    static DacAssigner * instance();
    static DacAssigner * dacAssigner;
    
    DacAssigner();
    ~DacAssigner();
    
    const vector<DacData>& getDacList();
    const vector<DacData>& updateDacList();
    
    bool assignToProjector(const string& label, Projector& projector);
    bool disconnectDacFromProjector(Projector& projector);
    DacData& getDacDataForLabel(const string& label);
    DacData& getDacDataForProjector(Projector& projector);
    
    DacManagerBase* getManagerForType(string type); 

    vector<DacManagerBase*> dacManagers;
    vector<DacData> dacDataList;
    //vector<DacBase*> dacs; 
    DacData emptyDacData;

    private:
    
    
};
}


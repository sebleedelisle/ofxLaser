//
//  ofxLaserDacAssigner.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//
// The DacAssigner is a complex object that is responsible for keeping
// track of available DACs and which laser objects they are connected to.
//
// It keeps a collection of DacManagers, one for each type of DAC that we
// support. The DAC objects themselves are owned by the managers, but
// the DacAssigner is responsible for giving and taking them from
// laser objects.

//
// When a laser loads, if it has a dacLabel parameter, the LaserManager
// will attempt to attach the DAC to the laser using
// dacAssigner.assignToLaser.
//
// The dacAssigner will search for the DAC but if it can’t find it, it’ll
// make a dacdata object, add it to the list, but disable its available
// flag.
//
// Then, when the dacassigner finds actual DACs, it compares them to the
// empty DACs in its list.

#pragma once

#include "ofMain.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserLaser.h"
#include "ofxLaserDacData.h"
#include "ofxLaserDacManagerBase.h"
#include "ofxLaserDacManagerLaserdock.h"
#include "ofxLaserDacManagerEtherDream.h"
#include "ofxLaserDacManagerHelios.h"

namespace ofxLaser {

class DacAssigner {
    
    public :
    
    // it's a Singleton so shouldn't ever have more than one.
    static DacAssigner * instance();
    static DacAssigner * dacAssigner;
    
    DacAssigner();
    ~DacAssigner();
    
    bool update(); 
    
    const vector<DacData>& getDacList();
    const vector<DacData>& updateDacList();
    string getAliasForLabel(const string& label);
    
    
    bool assignToLaser(const string& label, Laser& laser);
    bool disconnectDacFromLaser(Laser& laser);
    DacData& getDacDataForLabel(const string& label);
    DacData& getDacDataForLaser(Laser& laser);
    
    DacManagerBase* getManagerForType(string type); 

    vector<DacManagerBase*> dacManagers;
    vector<DacData> dacDataList;
    //vector<DacBase*> dacs; 
    DacData emptyDacData;
    map<string, string> aliasByLabel; 
    private:
    
    
};
}


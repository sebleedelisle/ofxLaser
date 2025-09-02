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
#include "ofxLaserDacManagerLaserDockNet.h"
#include "ofxLaserDacManagerEtherDream.h"
#include "ofxLaserDacManagerHelios.h"
#include "ofxLaserDacAliasManager.h"

#ifdef TARGET_OSX
#include "DacAVBSoundManager.h"
#endif

namespace ofxLaser {

class DacAssigner {
    
    public :
    
    // it's a Singleton so shouldn't ever have more than one.
    static DacAssigner * instance();
    static DacAssigner * dacAssigner;
    static void destroy();
    
    DacAssigner() ;
    ~DacAssigner();
    
    bool update(); 
    
    const vector<std::shared_ptr<DacData>>& getAvailableDacList();
    
    void updateDacList();
    string getAliasForLabel(const string& label);
    bool addAliasForLabel(string alias, const string& daclabel, bool force); 

    bool assignToLaser(const string& label, std::shared_ptr<Laser>& laser);
    bool disconnectDacFromLaser(std::shared_ptr<Laser>& laser);
    std::shared_ptr<DacData> getDacDataForLabel(const string& label);
    std::shared_ptr<DacData> getDacDataForLaser(std::shared_ptr<Laser>& laser);
    
    virtual void serialize(ofJson&json) const;
    virtual bool deserialize(ofJson&jsonGroup);
    
    std::shared_ptr<DacManagerBase> getManagerForType(string type);

    vector<std::shared_ptr<DacManagerBase>> dacManagers;
    
    vector<std::shared_ptr<DacData>> dacDataList;
    vector<std::shared_ptr<DacData>> availableDacDataList;
   
    DacAliasManager dacAliasManager;
    
    private:
    
   
    
    
};
}


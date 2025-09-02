//
//  ofxLaserDacManagerBase.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#pragma once
#include "ofxLaserDacManagerBase.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserDacEtherDream.h"
#include "ofxLaserDacEtherDreamData.h"
#include "ofxNetwork.h"
#include "ofThread.h"


namespace ofxLaser {


class DacManagerEtherDream : public DacManagerBase, ofThread{
    
    public :
    DacManagerEtherDream();
    ~DacManagerEtherDream();
    
    virtual vector<DacData> updateDacList() override;
    virtual std::shared_ptr<DacBase> getAndConnectToDac(const string& id) override;
    
    EtherDreamData getEtherDreamDataByMacAddress(string id) {
        if(etherdreamDataByMacAddress.find(id)!=etherdreamDataByMacAddress.end()) {
            return etherdreamDataByMacAddress.at(id);
        } else {
            return EtherDreamData();
        }
        
    }
    
    virtual string getType() override {
        return "EtherDream";
    }
    virtual void exit() override; 

    void threadedFunction() override; 
    
    int startUpDelayMS = 3000; 
    protected :
    
    bool connected = false;
    const bool checkInUse = true;
    ofxUDPManager udpConnection;

    map<string, EtherDreamData> etherdreamDataByMacAddress;
    double lastCheckTime = 0; 
    
};
}



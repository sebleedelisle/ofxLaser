//
//  ofxLaserDacManagerBase.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//

#pragma once
#include "ofxLaserDacManagerBase.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserDacLaserDockNet.h"
#include "ofxNetwork.h"
#include "ofThread.h"



namespace ofxLaser {


class DacManagerLaserDockNet : public DacManagerBase, ofThread{
    
    public :
    DacManagerLaserDockNet();
    ~DacManagerLaserDockNet();
    
    
    virtual vector<DacData> updateDacList() override;
    virtual std::shared_ptr<DacBase>  getAndConnectToDac(const string& id) override;
    virtual string getType() override {
        return "LaserCubeNet";
    }
    
    DacLaserDockNetStatus getLaserDockStatusByMacAddress(string id) {
        if(dacStatusById.find(id)!=dacStatusById.end()) {
            return dacStatusById.at(id);
        } else {
            return DacLaserDockNetStatus();
        }
    }
    
    // bit of a hack, maybe don't need any more 
    void setChanged() {
        dacsChanged = true;
    }
    
    virtual void exit() override;

    void threadedFunction() override; 
    
    protected :
    
    bool connected = false;
    ofxUDPManager udpSender;
   // ofxUDPManager udpReceiver;

    map<string, DacLaserDockNetStatus> dacStatusById;
    float lastCheckTime = 0; 
    
};
}



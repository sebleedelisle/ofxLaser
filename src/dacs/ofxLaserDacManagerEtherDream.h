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
    virtual DacBase* getAndConnectToDac(const string& id) override;
    virtual bool disconnectAndDeleteDac(const string& id) override;
    virtual string getType() override {
        return "EtherDream";
    }
    virtual void exit() override; 

    void threadedFunction() override; 
    
    protected :
    
    bool connected = false;
    ofxUDPManager udpConnection;

    map<string, EtherDreamData> etherdreamDataByMacAddress;
   // map<string, string> labelById; 
    
};
}



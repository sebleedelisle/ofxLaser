//
//  ofxLaserDacManagerBase.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#pragma once
#include "ofxLaserDacManagerBase.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserDacEtherdream.h"
#include "ofxNetwork.h"
#include "ofThread.h"


namespace ofxLaser {

struct EtherdreamData {
    int hardwareRevision;
    int softwareRevision;
    int bufferCapacity;
    int maxPointRate;
    string macAddress;
    string ipAddress;
    float lastUpdateTime;
};
    

class DacManagerEtherdream : public DacManagerBase, ofThread{
    
    public :
    DacManagerEtherdream();
    ~DacManagerEtherdream();
    
    virtual vector<DacData> updateDacList() override;
    virtual DacBase* getAndConnectToDac(const string& id) override;
    virtual bool disconnectAndDeleteDac(const string& id) override;
    virtual string getType() override {
        return "Etherdream";
    }
    virtual void exit() override; 

    void threadedFunction() override; 
    
    protected :
    
    bool connected = false;
    ofxUDPManager udpConnection;

    map<string, EtherdreamData> etherdreamDataByMacAddress;
   // map<string, string> labelById; 
    
};
}



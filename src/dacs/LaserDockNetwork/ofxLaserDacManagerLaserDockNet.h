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
    virtual DacBase* getAndConnectToDac(const string& id) override;
    virtual bool disconnectAndDeleteDac(const string& id) override;
    virtual string getType() override {
        return "LaserDockNet";
    }
    virtual void exit() override; 

    void threadedFunction() override; 
    
    protected :
    
    bool connected = false;
    //ofxUDPManager udpConnection;
    Poco::Net::DatagramSocket commandUdpSocket;
 

    

    map<string, DacLaserDockNetStatus> dacStatusById;
    float lastCheckTime = 0; 
    
};
}



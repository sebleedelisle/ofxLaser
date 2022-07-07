//
//  ofxLaserDacManagerBase.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//
#pragma once
#include "ofxLaserDacData.h"
#include "ofxLaserDacBase.h"

namespace ofxLaser {

class DacManagerBase {
    
    public :
    virtual vector<DacData> updateDacList() = 0;
    DacBase* getDacById(string id) {
        if(dacsById.count(id) == 1) {
            return dacsById.at(id);
        } else {
            return nullptr;
        }
    
    };
    
    bool checkDacsChanged() {
        if(dacsChanged) {
            dacsChanged = false;
            return true;
        } else {
            return false;
        }
        
    }
    virtual DacBase* getAndConnectToDac(const string& dacdata) = 0;
    virtual bool disconnectAndDeleteDac(const string& dacdata) = 0;
    virtual string getType() = 0; 
    virtual void exit() = 0; 
    
    protected :
    map<string, DacBase*>dacsById;
    // TODO thread safe??? 
    bool dacsChanged;
    
    private :
    
    
    
    
};
}

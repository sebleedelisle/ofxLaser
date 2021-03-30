//
//  ofxLaserDacDetectorBase.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//
#pragma once
#include "ofxLaserDacData.h"
#include "ofxLaserDacBase.h"

namespace ofxLaser {

class DacDetectorBase {
    
    public :
    virtual vector<DacData> updateDacList() = 0;
    virtual DacBase* getAndConnectToDac(const string& dacdata) = 0;
    virtual bool disconnectAndDeleteDac(const string& dacdata) = 0;
    virtual string getType() = 0; 

    private :
    
    
    
    
};
}

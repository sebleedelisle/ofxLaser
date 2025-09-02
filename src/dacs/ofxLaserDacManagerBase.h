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
    
    virtual ~DacManagerBase() {}
    
    virtual vector<DacData> updateDacList() = 0;
    std::shared_ptr<DacBase> getDacById(string id) {
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
    
    virtual void serialize(ofJson&json) const {};
    virtual bool deserialize(ofJson&jsonGroup) { return true;};
    
    virtual std::shared_ptr<DacBase> getAndConnectToDac(const string& dacdata) = 0;
    virtual bool disconnectAndDeleteDac(const string& id){
        
        std::shared_ptr<DacBase> dac = getDacById(id);
        if(!dac) {
            ofLogError("DacManagerBase::disconnectAndDeleteDac("+id+") - dac not found");
            return false;
        }
       
        dac->close();
        auto it=dacsById.find(id);
        dacsById.erase(it);
        //dac.reset(); // see if it deletes it? 
        ofLogNotice("DacManagerBase :: disconnectAndDeleteDac - dac deleted. Use count : ") << dac.use_count();
        return true;

        
    };
    virtual string getType() = 0;
    virtual void exit() = 0; 
    bool verbose = false;
    
    protected :
    
    map<string, std::shared_ptr<DacBase>>dacsById;
    std::atomic<bool> dacsChanged = false;

    private :
    
    
    
    
};
}

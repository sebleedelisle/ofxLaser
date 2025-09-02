//
//  ofxLaserDacAssigner.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserDacAssigner.h"

using namespace ofxLaser;

DacAssigner * DacAssigner :: dacAssigner = NULL;

DacAssigner * DacAssigner::instance() {
	if(dacAssigner == NULL) {
		dacAssigner = new DacAssigner();
	}
	return dacAssigner;
}


DacAssigner :: DacAssigner() {

    if(dacAssigner == NULL) {
		dacAssigner = this;
	} else {
		ofLog(OF_LOG_ERROR, "Multiple ofxLaser::DacManager instances created");
        throw;
	}
    
    aliasByLabel = {
       
        {"Etherdream 66E647A5986A", "SebLee ED A01"},
        {"Etherdream 5EE67D9E3666", "SebLee ED A02"},
        {"Etherdream 4AE1B1A5006A", "SebLee ED A03"},
        {"Etherdream 66E62D9EFE66", "SebLee ED A04"},
        {"Etherdream 66E653A5686A", "SebLee ED A05"},
        {"Etherdream 76E6069EE666", "SebLee ED A06"},
        {"Etherdream 4EE1A5A5246A", "SebLee ED A07"},
        {"Etherdream 76E6FA9D2267", "SebLee ED A08"},
        {"Etherdream 3AE1D9A5386A", "SebLee ED A09"},
        {"Etherdream 66E641A5BC6A", "SebLee ED A10"},
        {"Etherdream 4AE1AAA51C6A", "SebLee ED A11"},
        {"Etherdream 36E1E6A5346A", "SebLee ED A12"},
        {"Etherdream 124DACB1EF44", "SebLee ED B01"},
        {"Etherdream 124DACE7DAFB", "SebLee ED B02"},
        {"Etherdream 124DAC75882B", "SebLee ED B03"},
        {"Etherdream 124DACCE68EA", "SebLee ED B04"},
        {"Etherdream 124DAC730C99", "SebLee ED B05"},
        {"Etherdream 124DACD146EB", "SebLee ED B06"},
        {"Etherdream 124DACF2D99A", "SebLee ED B07"},
        {"Etherdream 124DACE6D0CC", "SebLee ED B08"},
        {"Etherdream 124DAC193A1B", "SebLee ED B09"},
        {"Etherdream 124DAC1BBFD3", "SebLee ED B10"},
        {"Etherdream 124DAC62CB62", "SebLee ED B11"},
        {"Etherdream 124DACC90945", "SebLee ED B12"}
    };

    
    dacManagers.push_back(new DacManagerLaserdock());
    dacManagers.push_back(new DacManagerHelios());
    dacManagers.push_back(new DacManagerEtherdream());
    updateDacList();
	
}

DacAssigner :: ~DacAssigner() {
    //dacAssigner = NULL;
}

const vector<DacData>& DacAssigner ::getDacList(){
    return dacDataList;
}

const vector<DacData>& DacAssigner ::updateDacList(){
    
    // get a new list of dacdata
    vector<DacData> newdaclist;
    
    for(DacManagerBase* dacManager : dacManagers) {
        // ask every dac manager for an updated list of DacData objects
        // and insert them into our new vector.
        vector<DacData> newdacs = dacManager->updateDacList();
        newdaclist.insert( newdaclist.end(), newdacs.begin(), newdacs.end() );
        
    }
    for(DacData& newdacdata : newdaclist) {
        if(aliasByLabel.find(newdacdata.label)!=aliasByLabel.end()) {
            newdacdata.alias = aliasByLabel[newdacdata.label];
            
        }
    }
    
    // go through the existing list, check against the new
    // list and if it can't find it any more, mark it as
    // unavailable.
    
    for(DacData& dacdata : dacDataList) {
        bool nowavailable = false;
        // Look up alias here!
       
        for(DacData& newdacdata : newdaclist) {
            // compare the new dac to the existing one
            if(newdacdata.id == dacdata.id) {
                
                // Store the new dac's availability
                // (the new dac should always be
                // available but just in case...)
                nowavailable = newdacdata.available;
                
                // We have a dacdata that is not available
                // but has an assigned laser which means that
                // the laser object is waiting for that dac to
                // become available.
                // So let's get the dac and assign it to the laser!
                if(!dacdata.available && (dacdata.assignedLaser!=nullptr)) {
                    DacBase* dacToAssign = getManagerForType(dacdata.type)->getAndConnectToDac(dacdata.id);
                    if(dacToAssign!=nullptr) {
                        dacToAssign->alias = dacdata.alias;
                        dacdata.assignedLaser->setDac(dacToAssign);
                        dacdata.available = true;
                    }
                }
                break;
            }
        }
        
        dacdata.available = nowavailable;
        
    }
    
    // now go through the new dac list again, and find
    // dacs that are not already in the existing list
    for(DacData& newdacdata : newdaclist) {
        bool isnew = true;
        for(DacData& dacdata : dacDataList) {
            if(dacdata.id == newdacdata.id) {
                isnew = false;
                break;
            }
        }
        
        // if it's new, add it to the list
        if(isnew) {
           
            dacDataList.push_back(newdacdata);
        }
    }
    
    // sort the list (the DacData class has overloaded operators
    // that make the list sortable alphanumerically by their IDs
	std::sort(dacDataList.begin(), dacDataList.end());
    
    return dacDataList; 
    
}


bool DacAssigner ::assignToLaser(const string& daclabel, Laser& laser){
    
    DacData* dacdataptr = &getDacDataForLabel(daclabel);
    
    if(&emptyDacData==dacdataptr) {
        
        // no dacdata found! This usually means that we're loading
        // and the new dac hasn't been found yet. So we need to reserve
        // one.
        
        // extract the type and id out of the daclabel (perhaps make this
        // a separate function?)
        string dactype = daclabel.substr(0, daclabel.find(" "));
        string dacid = daclabel.substr(daclabel.find(" ")+1, string::npos);
        
        dacDataList.emplace_back(dactype, dacid, "", &laser);
        dacdataptr = &dacDataList.back();
        dacdataptr->available = false;
        if(aliasByLabel.find(dacdataptr->label)!=aliasByLabel.end()) {
            dacdataptr->alias = aliasByLabel[dacdataptr->label];
            
        }
       
        return false;
        
    }
    DacData& dacdata = *dacdataptr;
    
    ofLogNotice("DacAssigner::assignToLaser - " + dacdata.label, ofToString(laser.laserIndex));
    
  
    // get manager for type
    DacManagerBase* manager = getManagerForType(dacdata.type);
    if(manager==nullptr) {
        ofLogError("DacAssigner ::assignToLaser - invalid type " + dacdata.type);
        return false;
    }
    
    
    // if laser already has a dac then delete it!
    disconnectDacFromLaser(laser);
    
    DacBase* dacToAssign = nullptr;
    
    if(dacdata.assignedLaser!=nullptr) {
        // remove from current laser
        
        // Is this bad? Maybe better to get the dac
        // from its manager?
        dacToAssign = dacdata.assignedLaser->getDac();
        dacdata.assignedLaser->removeDac();
        dacdata.assignedLaser = nullptr;
        
    } else {
    
        // get dac from manager
        dacToAssign = manager->getAndConnectToDac(dacdata.id);
        
    }
    // if success
    if(dacToAssign!=nullptr) {
        
        // is there a better place to assign this?
        dacToAssign->alias = dacdata.alias;
        
        // give the dac to the laser
        laser.setDac(dacToAssign);
        // store a reference to the laser in the
        // dacdata
        dacdata.assignedLaser = &laser;
        
        
        // clear the reference to this laser from the other dac data
        for(DacData& dacdataToCheck : dacDataList) {
            if(&dacdata == &dacdataToCheck) continue;
            else if(dacdataToCheck.assignedLaser == &laser) {
                dacdataToCheck.assignedLaser = nullptr;
            }
        }
        
    } else {
        // if we can't get a dac object for the label
        // the dac must have disconnected since we updated
        // the list!
        // Maybe we should store the laser in the
        // DacData anyway it can be connected if / when
        // it's found?
        dacdata.available = false;
        return false;
    }
    
    return true; 
}

bool DacAssigner :: disconnectDacFromLaser(Laser& laser) {
    DacData& dacData = getDacDataForLaser(laser);
    if(dacData.assignedLaser!=nullptr) {
        dacData.assignedLaser = nullptr;
        laser.removeDac();
        getManagerForType(dacData.type)->disconnectAndDeleteDac(dacData.id);
        return true;
    } else {
        return false;
    }
}
DacManagerBase* DacAssigner :: getManagerForType(string type){
    for(DacManagerBase* manager : dacManagers) {
        if(manager->getType() == type) {
            return manager;
            
            break;
        }
    }
    return nullptr;
    
}

DacData& DacAssigner ::getDacDataForLabel(const string& label){
    for(DacData& dacData : dacDataList) {
        if(dacData.label == label) {
            return dacData;
        }
    }
    
    
    return emptyDacData ;
}


DacData& DacAssigner ::getDacDataForLaser(Laser& laser){
    return getDacDataForLabel(laser.getDacLabel());
}


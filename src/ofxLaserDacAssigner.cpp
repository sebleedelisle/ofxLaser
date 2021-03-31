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
    
    // TODO - keep track of connected projectors!
    // TODO - get list of dacs as a new vector
    //        go through old list
    //        if element in old list isn't in new list
    //            if no projector attached, delete it
    //            if projector attached, mark it as unavailable
    //               (delete dac from projector?)
    //        if element in new list isn't in old list, add it!
    
    vector<DacData> newdaclist;
    
    for(DacManagerBase* dacManager : dacManagers) {
        vector<DacData> newdacs = dacManager->updateDacList();
        newdaclist.insert( newdaclist.end(), newdacs.begin(), newdacs.end() );
        
    }
    
    for(DacData& dac : dacDataList) {
        bool stillavailable = false;
        for(DacData& newdac : newdaclist) {
            if(newdac.id == dac.id) {
                stillavailable = true;
                break;
            }
        }
        
        dac.available = stillavailable;
        
    }
    for(DacData& newdac : newdaclist) {
        bool isnew = true;
        for(DacData& dac : dacDataList) {
            if(dac.id == newdac.id) {
                isnew = false;
                break;
            }
        }
        
        if(isnew) {
            dacDataList.push_back(newdac);
        }
    }
    

    return dacDataList; 
    
}


bool DacAssigner ::assignToProjector(const string& daclabel, Projector& projector){
    
    DacData& dacdata = getDacDataForLabel(daclabel);
    
    ofLogNotice("DacAssigner::assignToProjector - " + dacdata.label, projector.label);
    
    // TODO perhaps check to see if the dacdata is def in the list still ?
    
    if(&dacdata==&emptyDacData) return false;

    // get manager for type
    DacManagerBase* manager = getManagerForType(dacdata.type);
    if(manager==nullptr) {
        ofLogError("DacAssigner ::assignToProjector - invalid type " + dacdata.type);
        return false;
    }
    
    
    // if projector already has a dac then delete it! ******************************
    disconnectDacFromProjector(projector);
    
    
    DacBase* dacToAssign = nullptr;
    
    if(dacdata.assignedProjector!=nullptr) {
        // remove from current projector
        // TODO remove data from dacdata in list - make sure we're using
        // references to the same objects
        
        // ALERT!!!! THIS IS BAD. DAC SHOULD BE GOT FROM THE DAC MANAGERS
        dacToAssign = dacdata.assignedProjector->getDac();
        dacdata.assignedProjector->removeDac();
        dacdata.assignedProjector = nullptr;
        
    } else {
    
        // get dac from manager
        dacToAssign = manager->getAndConnectToDac(dacdata.id);
        
    }
    // if success
    if(dacToAssign!=nullptr) {
        //TODO better setDac in projector
        projector.setDac(dacToAssign);
        // projector.setDac(dac)
        dacdata.assignedProjector = &projector;
        //dacdata.available = false;
    }
    
    //dacdata.assignedProjector = &projector
    
    
    return true; 
}
bool DacAssigner :: disconnectDacFromProjector(Projector& projector) {
    DacData& dacData = getDacDataForProjector(projector);
    if(dacData.assignedProjector!=nullptr) {
        dacData.assignedProjector = nullptr;
        projector.removeDac();
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


DacData& DacAssigner ::getDacDataForProjector(Projector& projector){
    return getDacDataForLabel(projector.getDacLabel());
}


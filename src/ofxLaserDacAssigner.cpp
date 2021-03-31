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
    
    dacDetectors.push_back(new DacDetectorLaserdock());
    dacDetectors.push_back(new DacDetectorHelios());
    dacDetectors.push_back(new DacDetectorEtherdream());
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
    
    dacDataList.clear();
    
    for(DacDetectorBase* dacDetector : dacDetectors) {
        vector<DacData> newdacs = dacDetector->updateDacList();
        dacDataList.insert( dacDataList.end(), newdacs.begin(), newdacs.end() );
        
    }

    return dacDataList; 
    
}


bool DacAssigner ::assignToProjector(const string& daclabel, Projector& projector){
    
    DacData& dacdata = getDacDataForLabel(daclabel);
    
    ofLogNotice("DacAssigner::assignToProjector - " + dacdata.label, projector.label);
    
    // TODO perhaps check to see if the dacdata is def in the list still ?
    
    if(&dacdata==&emptyDacData) return false;

    // get detector for type
    DacDetectorBase* detector = getDetectorForType(dacdata.type);
    if(detector==nullptr) {
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
        
        // ALERT!!!! THIS IS BAD. DAC SHOULD BE GOT FROM THE DAC DETECTORS
        dacToAssign = dacdata.assignedProjector->getDac();
        dacdata.assignedProjector->removeDac();
        dacdata.assignedProjector = nullptr;
        
    } else {
    
        // get dac from manager
        dacToAssign = detector->getAndConnectToDac(dacdata.id);
        
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
        getDetectorForType(dacData.type)->disconnectAndDeleteDac(dacData.id);
        return true;
    } else {
        return false;
    }
}
DacDetectorBase* DacAssigner :: getDetectorForType(string type){
    for(DacDetectorBase* detector : dacDetectors) {
        if(detector->getType() == type) {
            return detector;
            
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


//
//  ofxLaserDacManager.cpp
//  ofxLaserRewrite
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
	//ofLog(OF_LOG_NOTICE, "ofxLaser::DacManager constructor");
	if(dacAssigner == NULL) {
		dacAssigner = this;
	} else {
		ofLog(OF_LOG_ERROR, "Multiple ofxLaser::DacManager instances created");
        throw;
	}
    
    dacDetectors.push_back(new DacDetectorLaserdock());
    updateDacList();
	
}

DacAssigner :: ~DacAssigner() {
    //dacAssigner = NULL;
}

const vector<DacData>& DacAssigner ::getDacList(){
    return dacDataList; 
}

const vector<DacData>& DacAssigner ::updateDacList(){
    
    //TODO - keep track of connected projectors!
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
    
    // TODO if projector already has a dac then delete it! ******************************
    
    DacBase* dac = nullptr;
    if(dacdata.assignedProjector!=nullptr) {
        // remove from current projector
        // TODO remove data from dacdata in list - make sure we're using
        // references to the same objects
        //detector->disconnectAndDeleteDac(dacdata.id);
        dac = dacdata.assignedProjector->getDac();
        dacdata.assignedProjector->removeDac();
    } else {
    
        // get dac from manager
        dac = detector->getAndConnectToDac(dacdata.id);
        
    }
    // if success
    if(dac!=nullptr) {
        //TODO better setDac in projector
        projector.setDac(dac);
        // projector.setDac(dac)
        dacdata.assignedProjector = &projector;
        //dacdata.available = false;
    }
    
    //dacdata.assignedProjector = &projector
    
    
    return true; 
}

DacDetectorBase* DacAssigner ::getDetectorForType(string type){
    for(DacDetectorBase* detector : dacDetectors) {
        if(detector->getType() == type) {
            return detector;
            
            break;
        }
    }
    return nullptr;
    
}

DacBase* DacAssigner ::getDac(string label){
    
    
}

bool DacAssigner ::releaseDac(string label){
    
    
}

DacData& DacAssigner ::getDacDataForLabel(const string& label){
    for(DacData& dacData : dacDataList) {
        if(dacData.label == label) {
            return dacData;
        }
    }
    return emptyDacData ;
}

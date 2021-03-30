//
//  ofxLaserDacAssigner.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//


#pragma once

#include "ofMain.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserProjector.h"
#include "ofxLaserDacData.h"
#include "ofxLaserDacDetectorBase.h"
#include "ofxLaserDacDetectorLaserdock.h"


namespace ofxLaser {

class DacAssigner {
    
    public :
    
    // it's a Singleton so shouldn't ever have more than one.
    static DacAssigner * instance();
    static DacAssigner * dacAssigner;
    
    DacAssigner();
    ~DacAssigner();
    
    const vector<DacData>& getDacList();
    const vector<DacData>& updateDacList();
    
    DacBase* getDac(string label);
    bool releaseDac(string label);
    
    DacData& getDacDataForLabel(const string& label);
    bool assignToProjector(const string& label, Projector& projector);
    DacDetectorBase* getDetectorForType(string type); 

    vector<DacDetectorBase*> dacDetectors;
    vector<DacData> dacDataList;
    //vector<DacBase*> dacs; 
    DacData emptyDacData;
    
    private:
    
    
};
}


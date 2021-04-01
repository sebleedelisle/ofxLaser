//
//  ofxLaserDacData.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//
#pragma once

#include "ofxLaserProjector.h"

class DacData {

    public :
    
    DacData() {
        type = "";
        label = "EMPTY";
        available = false;
        id = "" ;
        assignedProjector = nullptr;
    }
    
    // maybe have a label identifier for the projector? might be safer than the object
    DacData(string _type, string _id, string _address="", ofxLaser::Projector* projector = nullptr){
        type = _type;
        id = _id;
        address = _address;
        assignedProjector = projector;
        label = type + " " + id;
        available = true;
        
    }

	inline bool operator < ( const DacData& rhs) const {
		return label<rhs.label; 

	}
	inline bool operator > (const DacData& rhs) const{
		return label > rhs.label;

	}
    string label;   // display label ie "Etherdream #efbd59" - maybe not necessary as we can compile this
    string type;    // type ie "Etherdream", "Helios" etc - should this be a class?
    string id;      // id is the Mac address or the serial number
    string address; // IP address for network DACs
    bool available; // is it still available? Useful for if a dac connects and then becomes unavailable
    ofxLaser::Projector* assignedProjector; // which projector it's currently assigned to 
    
    
};

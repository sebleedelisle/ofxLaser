//
//  ofxLaserDacData.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//
#pragma once

#include "ofMain.h"

namespace ofxLaser {

class Laser; 
class DacData {

    public :
    
    DacData() {
        type = "";
        label = "EMPTY";
        available = false;
        id = "" ;
        assignedLaser = nullptr;
    }
    
    DacData(string _type, string _id, string _address="", ofxLaser::Laser* laser = nullptr){
        type = _type;
        id = _id;
        address = _address;
        assignedLaser = laser;
        label = type + " " + id;
        available = true;
        
    }
    

	inline bool operator < ( const DacData& rhs) const {
        const string* string1 = (alias!="") ? &alias : &label;
        const string* string2 = (rhs.alias!="") ? &rhs.alias : &rhs.label;

		return *string1<*string2;
	}
	inline bool operator > (const DacData& rhs) const{
        const string* string1 = (alias!="") ? &alias : &label;
        const string* string2 = (rhs.alias!="") ? &rhs.alias : &rhs.label;

        return *string1>*string2;
	}
    string label;   // display label ie "Ether Dream #efbd59" - maybe not necessary as we can compile this
    string alias;   // a friendly name for the DAC 
    string type;    // type ie "EtherDream", "Helios" etc - should this be a class?
    string id;      // id is the Mac address or the serial number
    string address; // IP address for network DACs
    bool available; // is it still available? Useful for if a dac connects and then becomes unavailable
    ofxLaser::Laser* assignedLaser; // which laser it's currently assigned to 
    
    
};
}

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
        //label = "EMPTY";
        available = false;
        id = "" ;
        assignedLaser = nullptr;
        alias = "";
        unavailable = false; 
    }
    
    DacData(string _type, string _id, string _address="", bool _unavailable = false, std::shared_ptr<Laser> laser = nullptr){
        type = _type;
        id = _id;
        unavailable = _unavailable;
        address = _address;
        assignedLaser = laser;
        //label = type + " " + id;
        available = true;
        alias = "";
        
    }
    
    const string getLabel() const  {
        return type + " " + id;
    }
    
    
	inline bool operator < ( const DacData& rhs) const {
        string string1 = getAlias(); // (alias!="") ? &alias : &label;
        string string2 = rhs.getAlias(); // (rhs.alias!="") ? &rhs.alias : &rhs.label;

		return string1<string2;
	}
	inline bool operator > (const DacData& rhs) const{
        string string1 = getAlias(); // (alias!="") ? &alias : &label;
        string string2 = rhs.getAlias(); //(rhs.alias!="") ? &rhs.alias : &rhs.label;

        return string1>string2;
	}
    
    //string alias;   // a friendly name for the DAC
    string type;    // type ie "EtherDream", "Helios" etc - should this be a class?
    string id;      // id is the Mac address or the serial number
    string address; // IP address for network DACs
    string alias;
    bool available; // is it still available? Useful for if a dac connects and then becomes unavailable
    bool unavailable; // is the DAC in use? Useful to see if someone else is using it
    std::shared_ptr<Laser> assignedLaser; // which laser it's currently assigned to
    private :
    //string label;   // display label ie "Ether Dream #efbd59" - maybe not necessary as we can compile this
    const string getAlias() const {
        if(alias=="") return getLabel();
        else return alias;
    }
};
}

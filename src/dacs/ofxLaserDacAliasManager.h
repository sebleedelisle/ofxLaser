//
//  ofxLaserDacAliasManager.h
//  Liberation
//
//  Created by Seb Lee-Delisle on 15/07/2022.
//

#pragma once
#include "ofMain.h"

namespace ofxLaser {
class DacAliasManager {
    
    public :
    
    bool addAliasForLabel(string alias, const string& daclabel, bool force = false);
    string getAliasForLabel(const string& daclabel);
      
    bool load();
    bool save();
    
    void serialize(ofJson& json) const;
    bool deserialize(ofJson& json) ;
   
    
    
    map<string, string> aliasByLabel;
   
    
};
}

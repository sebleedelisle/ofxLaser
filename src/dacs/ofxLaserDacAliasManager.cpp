//
//  ofxLaserDacAliasManager.cpp
//
//  Created by Seb Lee-Delisle on 15/07/2022.
//

#include "ofxLaserDacAliasManager.h"

using namespace ofxLaser;

string DacAliasManager :: getAliasForLabel(const string& daclabel) {
  
    if(aliasByLabel.find(daclabel)!=aliasByLabel.end()) {
        return aliasByLabel[daclabel];
    } else {
        return daclabel;
    }
    
}

bool DacAliasManager :: addAliasForLabel(string alias, const string& daclabel, bool force){
    if((!force) && (aliasByLabel.find(daclabel)!=aliasByLabel.end())) {
        return false;
    } else {
        aliasByLabel[daclabel]=alias;
        save();
        return true;
    }
}

bool DacAliasManager :: load() {
    ofJson json = ofLoadJson("ofxLaser/dacAliases.json");
    
    for (auto it : json.items()) {
        aliasByLabel[it.key()] = it.value();
       // it.value();
    //for(int i = 0; i<json.size(); i++) {
    //    auto it = json.items();
    //for(auto jsonobject : json) {
        //cout << jsonobject.dump() << endl;
        //jsonobject.
    
    }
    
}

bool DacAliasManager :: save() {
    ofJson json;
    for(auto it : aliasByLabel) {
        json[it.first] =  // label
            it.second; // alias
    }
    ofSavePrettyJson("ofxLaser/dacAliases.json", json);
    
}

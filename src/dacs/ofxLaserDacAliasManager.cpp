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
    
    string filename ="ofxLaser/dacAliases.json";
    if(!ofFile(filename).exists()) return false;
    
    ofJson json = ofLoadJson(filename);
    
    for (auto it : json.items()) {
        string value = it.value(); 
        aliasByLabel[it.key()] = value; //  it.value();
       // it.value();
    //for(int i = 0; i<json.size(); i++) {
    //    auto it = json.items();
    //for(auto jsonobject : json) {
        //cout << jsonobject.dump() << endl;
        //jsonobject.
    
    }
    return true;
    
}

bool DacAliasManager :: save() {
    ofJson json;
    for(auto it : aliasByLabel) {
        json[it.first] =  // label
            it.second; // alias
    }
    return ofSavePrettyJson("ofxLaser/dacAliases.json", json);
    
}

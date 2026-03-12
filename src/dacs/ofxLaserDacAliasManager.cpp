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
    try {
        if(!ofFile(filename).exists()) return false;
        
        ofJson json = ofLoadJson(filename);
        return deserialize(json);
    } catch(const std::exception& e) {
        ofLogError("DacAliasManager::load") << "failed to read " << filename << " : " << e.what();
        return false;
    } catch(...) {
        ofLogError("DacAliasManager::load") << "failed to read " << filename << " : unknown exception";
        return false;
    }
//
//    for (auto it : json.items()) {
//        string value = it.value(); 
//        aliasByLabel[it.key()] = value; //  it.value();
//    }
//    return true;
//    
}

bool DacAliasManager :: save() {
    try {
        ofJson json;
        serialize(json);
//    for(auto it : aliasByLabel) {
//        json[it.first] =  // label
//            it.second; // alias
//    }
        return ofSavePrettyJson("ofxLaser/dacAliases.json", json);
    } catch(const std::exception& e) {
        ofLogError("DacAliasManager::save") << "failed to write ofxLaser/dacAliases.json : " << e.what();
        return false;
    } catch(...) {
        ofLogError("DacAliasManager::save") << "failed to write ofxLaser/dacAliases.json : unknown exception";
        return false;
    }
    
}

bool DacAliasManager :: deserialize(ofJson& json)  {
    
    for (auto it : json.items()) {
        string value = it.value();
        aliasByLabel[it.key()] = value; //  it.value();
    }
    return true;
    
}

void DacAliasManager :: serialize(ofJson& json) const{
    
    for(auto it : aliasByLabel) {
        json[it.first] =  // label
            it.second; // alias
    }
   
    
}

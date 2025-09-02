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
void DacAssigner::destroy() {
    delete dacAssigner;
    dacAssigner = NULL;
    
}


DacAssigner :: DacAssigner() {

    if(dacAssigner == NULL) {
		dacAssigner = this;
	} else {
		ofLog(OF_LOG_ERROR, "Multiple ofxLaser::DacManager instances created");
        throw;
	}
    dacAliasManager.load();
    
    dacManagers.push_back(std::unique_ptr<DacManagerBase>(new DacManagerLaserDock()));
    dacManagers.push_back(std::unique_ptr<DacManagerBase>(new DacManagerHelios()));
    dacManagers.push_back(std::unique_ptr<DacManagerBase>(new DacManagerEtherDream()));
    dacManagers.push_back(std::unique_ptr<DacManagerBase>(new DacManagerLaserDockNet()));
#ifdef TARGET_OSX
    dacManagers.push_back(std::unique_ptr<DacManagerBase>(new DacAVBSoundManager()));
#endif
    updateDacList();
	
}

DacAssigner :: ~DacAssigner() {
    dacManagers.clear();
}


bool DacAssigner :: update() {
    bool changed = false;
    for(std::shared_ptr<DacManagerBase>& dacManager : dacManagers) {
        if(dacManager->checkDacsChanged()) changed = true;
    }
    
    if(changed) updateDacList();
    
    return changed;
}

const vector<std::shared_ptr<DacData>>& DacAssigner ::getAvailableDacList(){
    
    return availableDacDataList;
}


void DacAssigner ::updateDacList(){
    
    // get a new list of dacdata
    vector<DacData> newdaclist;
    
    for(std::shared_ptr<DacManagerBase>& dacManager : dacManagers) {
        // ask every dac manager for an updated list of DacData objects
        // and insert them into our new vector.
        vector<DacData> newdacs = dacManager->updateDacList();
        newdaclist.insert( newdaclist.end(), newdacs.begin(), newdacs.end() );
    }
    
    for(DacData& newdacdata : newdaclist) {
        newdacdata.alias = dacAliasManager.getAliasForLabel(newdacdata.getLabel());

        ofLogNotice(" DacAssigner ::updateDacList " ) << newdacdata.getLabel() << " " << newdacdata.alias;
        if(newdacdata.alias==" ") {
            ofLogError("MISSING DAC ALIAS!");
        }
        

        //
//        if(aliasByLabel.find(newdacdata.getLabel())!=aliasByLabel.end()) {
//            newdacdata.alias = aliasByLabel[newdacdata.getLabel()];
//        } else {
//            newdacdata.alias = "";
//        }

    }
    
    // go through the existing list, check against the new
    // list and if it can't find it any more, mark it as
    // unavailable.
    
    for(std::shared_ptr<DacData>& dacdata : dacDataList) {
        bool nowavailable = false;
        // Look up alias here!
       
        for(DacData& newdacdata : newdaclist) {
            // compare the new dac to the existing one
            if(newdacdata.id == dacdata->id) {
                
                // Store the new dac's availability
                // (the new dac should always be
                // available but just in case...)
                nowavailable = newdacdata.available;
                
                // We have a dacdata that is not available
                // but has an assigned laser which means that
                // the laser object is waiting for that dac to
                // become available.
                // So let's get the dac and assign it to the laser!
                if(((!dacdata->available) || (dacdata->unavailable) ) && (dacdata->assignedLaser!=nullptr)) {
                    if(!newdacdata.unavailable) {
                        std::shared_ptr<DacManagerBase>manager = getManagerForType(dacdata->type);
                        std::shared_ptr<DacBase> dacToAssign = nullptr;
                        if(manager) dacToAssign = manager->getAndConnectToDac(dacdata->id);
                        if(dacToAssign!=nullptr) {
                            //dacToAssign->setAlias(dacdata.alias);
                            dacdata->assignedLaser->setDac(dacToAssign);
                        }
                        dacdata->available = true;
                    }
                }
                break;
            }
        }
        
        dacdata->available = nowavailable;
        
    }
    
    // now go through the new dac list again, and find
    // dacs that are not already in the existing list
    for(DacData& newdacdata : newdaclist) {
        bool isnew = true;
        for(std::shared_ptr<DacData>& dacdata : dacDataList) {
            if(dacdata->id == newdacdata.id) {
                // trying this because sometimes the ip address is lost when
                // passing back and forth
                dacdata->address = newdacdata.address;
                // update state in case it's changed
                dacdata->unavailable = newdacdata.unavailable;
                isnew = false;
                break;
            }
        }
        
        // if it's new, add it to the list
        if(isnew) {
           
            dacDataList.push_back(std::make_shared<DacData>(newdacdata)); // copy constructor
        }
    }
    
    // sort the list (the DacData class has overloaded operators
    // that make the list sortable alphanumerically by their IDs
	std::sort(dacDataList.begin(), dacDataList.end());
    std::sort(dacDataList.begin(), dacDataList.end(), [](std::shared_ptr<DacData>& a, std::shared_ptr<DacData>& b) {
        return (*a.get() < *b.get());
        });
    
    
    
    availableDacDataList.clear();
    for(std::shared_ptr<DacData>& dacdata : dacDataList) {
        if(dacdata->available) availableDacDataList.push_back(dacdata);
    }
    
    //return dacDataList;
    
}

string DacAssigner :: getAliasForLabel(const string& daclabel) {
    return dacAliasManager.getAliasForLabel(daclabel);
    
}
bool DacAssigner :: addAliasForLabel(string alias, const string& daclabel, bool force) {
    return dacAliasManager.addAliasForLabel(alias, daclabel, force);
    
}

bool DacAssigner ::assignToLaser(const string& daclabel, std::shared_ptr<Laser>& laser){
    ofLogNotice("DacAssigner ::assignToLaser " ) << daclabel; 
    std::shared_ptr<DacData> dacdata = getDacDataForLabel(daclabel);
    
    if(dacdata == nullptr) {
        
        // no dacdata found! This usually means that we're loading
        // and the new dac hasn't been found yet. So we need to reserve
        // one.
        
        // extract the type and id out of the daclabel (perhaps make this
        // a separate function?)
        string dactype = daclabel.substr(0, daclabel.find(" "));
        string dacid = daclabel.substr(daclabel.find(" ")+1, string::npos);
        
        
        dacdata = std::make_shared<DacData>(dactype, dacid, "", false, laser);
        dacDataList.push_back(dacdata);
        
        dacdata->alias = dacAliasManager.getAliasForLabel(dacdata->getLabel());
        dacdata->available = false;

        return false;
        
    }
    
    
    ofLogNotice("DacAssigner::assignToLaser - " + dacdata->getLabel(), ofToString(laser->laserIndex));
    
  
    // get manager for type
    std::shared_ptr<DacManagerBase> manager = getManagerForType(dacdata->type);
    if(manager==nullptr) {
        ofLogError("DacAssigner ::assignToLaser - invalid type " + dacdata->type);
        return false;
    }
    
    
    // if laser already has a dac then delete it!
    disconnectDacFromLaser(laser);
    
    std::shared_ptr<DacBase> dacToAssign;
    
    if(dacdata->assignedLaser!=nullptr) {
        // remove from current laser
        
        // Is this bad? Maybe better to get the dac
        // from its manager?
        dacToAssign = dacdata->assignedLaser->getDac();
        dacdata->assignedLaser->removeDac();
        dacdata->assignedLaser = nullptr;
        
    } else {
    
        // get dac from manager
        dacToAssign = manager->getAndConnectToDac(dacdata->id);
        
    }
    // if success
    if(dacToAssign!=nullptr) {
        
        // is there a better place to assign this?
        //dacToAssign->setAlias(dacdata.alias);
        
        // give the dac to the laser
        laser->setDac(dacToAssign);
        // store a reference to the laser in the
        // dacdata
        dacdata->assignedLaser = laser;
        
        
        // clear the reference to this laser from the other dac data
        for(std::shared_ptr<DacData> dacdataToCheck : dacDataList) {
            if(dacdata.get() == dacdataToCheck.get()) continue;
            else if(dacdataToCheck->assignedLaser.get() == laser.get()) {
                dacdataToCheck->assignedLaser = nullptr;
            }
        }
        
    } else {
        // if we can't get a dac object for the label
        // the dac must have disconnected since we updated
        // the list!
        // Maybe we should store the laser in the
        // DacData anyway it can be connected if / when
        // it's found?
        dacdata->assignedLaser = laser;
        laser->dacLabel = dacdata->getLabel();
        dacdata->available = false;
        return false;
    }
    
    return true; 
}

bool DacAssigner :: disconnectDacFromLaser(std::shared_ptr<Laser>& laser) {
    std::shared_ptr<DacData> dacData = getDacDataForLaser(laser);
    if((dacData!=nullptr) && (dacData->assignedLaser!=nullptr)) {
        dacData->assignedLaser = nullptr;
        laser->removeDac();
        std::shared_ptr<DacManagerBase> manager = getManagerForType(dacData->type);
        if(manager) manager->disconnectAndDeleteDac(dacData->id);
        else {
            ofLogNotice("Error - disconnected non existent dac ");
        }
        return true;
    } else {
        return false;
    }
}
std::shared_ptr<DacManagerBase> DacAssigner :: getManagerForType(string type){
    for(std::shared_ptr<DacManagerBase>& dacManager : dacManagers) {
        if(dacManager->getType() == type) {
            return dacManager;
            
            break;
        }
    }
    return nullptr;
    
}

std::shared_ptr<DacData> DacAssigner ::getDacDataForLabel(const string& label){
    for(std::shared_ptr<DacData>& dacData : dacDataList) {
        if(dacData->getLabel() == label) {
            return dacData;
        }
    }
    
    
    return nullptr ;
}


std::shared_ptr<DacData> DacAssigner ::getDacDataForLaser(std::shared_ptr<Laser>& laser){
    return getDacDataForLabel(laser->getDacLabel());
}



void DacAssigner::serialize(ofJson&json) const {
    for(const std::shared_ptr<DacManagerBase>& manager : dacManagers) {
        ofJson& managerJson = json[manager->getType()];
        manager->serialize(managerJson);
    }
}
bool DacAssigner::deserialize(ofJson&jsonGroup) {
    for(const std::shared_ptr<DacManagerBase>& manager : dacManagers) {
        if(jsonGroup.contains(manager->getType())) {
            manager->deserialize(jsonGroup[manager->getType()]);
        }
    }
    return true;
}

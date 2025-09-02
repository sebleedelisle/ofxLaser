#include "DacAVBSoundManager.h"
#include "DacAVBSound.h"

namespace ofxLaser {

DacAVBSoundManager::DacAVBSoundManager() {

    //EVENTUALLY THERE WILL BE SETTINGS FOR :
    //  Adding / removing sound interfaces
    //  Sound channel set up ? 
    
//    std::vector<ofSoundDevice> devices = ofSoundStreamListDevices();
//    
//    string devicename = "";
//    for (size_t i = 0; i<devices.size(); i++) {
//        ofLogNotice() << devices[i].name;
//        if(devices[i].outputChannels == 64) {
//            connectToInterface(devices[i]);
//        }
//    }
    
}

DacAVBSoundManager::~DacAVBSoundManager() {
   // exit();
}

bool DacAVBSoundManager::connectToInterface(ofSoundDevice& device) {
    
    std::shared_ptr<DacAVBSoundInterface> interface = std::make_shared<DacAVBSoundInterface>();
    if(interface->setup(device)) {
        interfaces.push_back(interface);
        const std::vector<std::shared_ptr<DacAVBSound>>& dacs = interface->getDacs();
        
        for(const std::shared_ptr<DacAVBSound>& dac : dacs ) {
            dacsById.emplace(std::make_pair(dac->getFullId(), dac));
            ofLogNotice("adding dac ") << dac->getFullId();
        }
        dacsChanged = true;
        return true;
    } else {
        ofLogNotice("DacAVBSoundManager::connectToInterface Could not connect to interface : ") << device.name;
        
        return false; 
    }
    
    
}


bool DacAVBSoundManager::connectToInterface(const std::string& devicename) {
    std::vector<ofSoundDevice> devices = ofSoundStreamListDevices();
    for(ofSoundDevice& device : devices) {
        if (device.name == devicename) {
            return connectToInterface(device);  // Call the overload that takes an ofSoundDevice
        }
    }
    return false;
    
}

std::shared_ptr<DacAVBSoundInterface> DacAVBSoundManager::getInterfaceForDevice(const std::string& devicename){
    for(std::shared_ptr<DacAVBSoundInterface>& interface : interfaces) {
        if(interface->getDevice().name == devicename) {
            return interface;
        }
    }
    return nullptr;
    
}


bool DacAVBSoundManager::getIsUsingDevice(const std::string& devicename) {
    
    return getInterfaceForDevice(devicename)!=nullptr;
    
}
bool DacAVBSoundManager::disconnectFromInterface(const std::string& devicename) {
    // get interface
    
    std::shared_ptr<DacAVBSoundInterface> interface = getInterfaceForDevice(devicename);
    if(interface) {
        vector<std::shared_ptr<DacAVBSound>> dacs = interface->getDacs();
        // get dacs
        for(std::shared_ptr<DacAVBSound>& dac : dacs) {
            // close and delete
            disconnectAndDeleteDac(dac->getFullId());
        }
        // delete interface
        interfaces.erase(
                         std::remove_if(interfaces.begin(), interfaces.end(),
                                        [&interface](const std::shared_ptr<DacAVBSoundInterface>& ptr) {
                                            return ptr == interface;
                                        }),
                         interfaces.end()
                         );
        dacsChanged = true;
        return true;
    } else {
        return false;
    }
}



std::vector<DacData> DacAVBSoundManager::updateDacList() {
    std::vector<DacData> daclist;
    
    for(std::shared_ptr<DacAVBSoundInterface>& interface : interfaces) {
        const std::vector<std::shared_ptr<DacAVBSound>>& dacs = interface->getDacs();
        for(const std::shared_ptr<DacAVBSound>& dac : dacs ) {
            daclist.emplace_back(getType(), dac->getRawId());
            ofLogNotice("adding daclist ") << getType() << " " << daclist.back().getLabel();
        }
    }

    return daclist;
}


std::shared_ptr<DacBase> DacAVBSoundManager::getAndConnectToDac(const std::string& id) {
    
    std::shared_ptr<DacBase> dac =  getDacById(getType() + " " + id);
    std::shared_ptr<DacAVBSound> avbDac = std::dynamic_pointer_cast<DacAVBSound>(dac);
    if(avbDac) {
        // not really sure why i'm checking, if it's not an AVB dac then something has
        // gone really wrong!
        avbDac->setActive(true);
    }
    return dac;
}





void DacAVBSoundManager::exit() {
    for (auto& [id, dac] : dacsById) {
        dac->close();
    }
    dacsById.clear();
}

void DacAVBSoundManager::serialize(ofJson&json) const {
    ofJson& jsonInterfaces = json["interfaces"];
    for(size_t i= 0; i<interfaces.size(); i++) {
        interfaces[i]->serialize(jsonInterfaces[i]);
    }
    
}
bool DacAVBSoundManager::deserialize(ofJson&jsonGroup) {
    
    ofJson& jsonInterfaces = jsonGroup["interfaces"];
    if(jsonInterfaces.size()>0) {
        interfaces.clear(); // TODO check if this closes down ok!
        //interfaces.resize(jsonInterfaces.size());
        for (size_t i = 0; i<jsonInterfaces.size(); i++ ) {
            ofJson& interfacejson =jsonInterfaces[i];
            if(interfacejson.contains("devicename")) {
                connectToInterface(interfacejson["devicename"].get<string>());
            }
            //interfaces[i] = std::make_shared<DacAVBSoundInterface>();
            //interfaces[i]->deserialize(jsonInterfaces[i]);
        }
    }
    return true;
}


}

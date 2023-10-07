//
//  ofxLaserDacManagerBase.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#include "ofxLaserDacManagerEtherDream.h"

using namespace ofxLaser;

DacManagerEtherDream :: DacManagerEtherDream()  {
    ofxUDPSettings settings;
    settings.bindPort = 7654;
    settings.blocking = false;

    udpConnection.Setup(settings);
    startThread();
}

DacManagerEtherDream :: ~DacManagerEtherDream()  {
    stopThread();
    waitForThread();
    // TODO wait for all DACs threads to stop
    udpConnection.Close();
}

void DacManagerEtherDream :: threadedFunction() {
    
    const int packetSize = 50;
    char udpMessage[packetSize];
    
    auto & thread = getNativeThread();
    //unsigned int n = std::thread::hardware_concurrency();
    //    std::cout << n << " concurrent threads are supported.\n";
   
#ifndef _MSC_VER
    // only linux and osx
    //http://www.yonch.com/tech/82-linux-thread-priority
    struct sched_param param;
    param.sched_priority = 1; // (highest) sched_get_priority_max(SCHED_FIFO);//89; // - higher is faster
    pthread_setschedparam(thread.native_handle(), SCHED_RR, &param );
#else
    // windows implementation
    SetThreadPriority( thread.native_handle(), THREAD_PRIORITY_LOWEST);
#endif
    
    while(isThreadRunning()) {
        
        // LET'S ASSUME FOR NOW...
        // that every packet is a complete message from a single dac.
        
        int numBytesReceived = 0;
        do {
            memset(udpMessage,0,sizeof(udpMessage));
            numBytesReceived = udpConnection.Receive(udpMessage,packetSize); //returns number of bytes received
            
            // ofLogNotice("Received "+ ofToString(numBytesReceived) + " bytes from UDP connection");
            if(numBytesReceived >=36)  {
                string address;
                int port;
                // Honestly I'm not sure what happens with multiple etherdreams...
                udpConnection.GetRemoteAddr(address, port);
                //            std::cout << "----------------------------------------------------------------------"<< std::endl;
                //            std::cout << "ip: " << address << " " << port << std::endl;
                //            std::cout << "Packet Size: " << numBytesReceived << std::endl;
                //            std::cout << "UDP Packet: " << std::endl;
                //
                uint64_t macAddress=0;
                int i = 0;
                for(i = 0; i < 6 ; i++) {
                    macAddress = macAddress<<8;
                    macAddress|=(unsigned char)udpMessage[i];
                    //printf("addingbyte : %llx\n", (unsigned char)udpMessage[i]);
                    //printf("Mac Address : %llx\n", macAddress);
                    
                }
                //cout << endl;
                //printf("Mac Address : %llx\n", macAddress);
                uint16_t hardwareRevision, softwareRevision, bufferCapacity;
                uint32_t maxPointRate;
                unsigned char* byteaddress = (unsigned char*)&udpMessage[i];
                hardwareRevision = ByteStreamUtils::bytesToUInt16(byteaddress);
                byteaddress+=2;
                softwareRevision = ByteStreamUtils::bytesToUInt16(byteaddress);
                byteaddress+=2;
                bufferCapacity = ByteStreamUtils::bytesToUInt16(byteaddress);
                byteaddress+=2;
                maxPointRate = ByteStreamUtils::bytesToUInt32(byteaddress);
                byteaddress+=4;
                
                
                //unsigned char* buffer = byteaddress-2;
                DacEtherDreamStatus status;
                status.deserialize(byteaddress);
                
                //            cout << "Hardware version :" << hardwareRevision << endl;
                //            cout << "Software version :" << softwareRevision << endl;
                //            cout << "Buffer capacity  :" << bufferCapacity << endl;
                //            cout << "Max point rate   :" << maxPointRate << endl;
                //            cout << "Buffer           :" << status.buffer_fullness << endl;
                //            cout << "Point count      :" << status.point_count << endl;
                //
                char idchar[100];
                int part0 = macAddress & 0xffff;
                int part1 = (macAddress>>16) & 0xffff;
                int part2 = (macAddress>>32) & 0xffff;
                
                sprintf(idchar, "%04X%04X%04X", part2, part1, part0);
                //sprintf(idchar, "%llX", macAddress);
                string id(idchar);
                
                // if we haven't already got this etherdream, then add it
                if(etherdreamDataByMacAddress.find(id) == etherdreamDataByMacAddress.end()) {
                    EtherDreamData ed = {hardwareRevision, softwareRevision, bufferCapacity, (int) maxPointRate, id, address, ofGetElapsedTimef()};
                    ofLogNotice("Adding etherdream "+ id)<< " " << hardwareRevision << " " << softwareRevision << " " << id;
                    //ofLogNotice(status.toString());
                    if(lock()) {
                        etherdreamDataByMacAddress[id] = ed;
                        dacsChanged = true;
                        unlock();
                    }
                } else {
                    
                    if(lock()) {
                        etherdreamDataByMacAddress[id].lastUpdateTime = ofGetElapsedTimef();
                        unlock();
                    }
                }
                
            }
            sleep(10); 
        } while (numBytesReceived>0);
        
//        if(ofGetElapsedTimef()-lastCheckTime > 2) {
//
//        }
//
        if(lock()) {
            
            for (auto it = etherdreamDataByMacAddress.cbegin(); it != etherdreamDataByMacAddress.cend() /* not hoisted */; /* no increment */)  {
                const EtherDreamData& ed = it->second;
                if ((ofGetElapsedTimef() - ed.lastUpdateTime)>2){
                    etherdreamDataByMacAddress.erase(it++);    // or "it = m.erase(it)" since C++11
                    dacsChanged = true;
                    
                } else {
                    ++it;
                }
            }

            lastCheckTime = ofGetElapsedTimef();
            unlock();
        }
        sleep(1000);
            
    }
}
    
vector<DacData> DacManagerEtherDream :: updateDacList(){
    
    vector<DacData> daclist;
    
    for(auto etherdreampair : etherdreamDataByMacAddress) {
        EtherDreamData& ed = etherdreampair.second;
       // ofLogNotice(ed.macAddress);
        
        string id = ed.macAddress;
        daclist.emplace_back(getType(), id, ed.ipAddress);

    }
    return daclist;
}


DacBase* DacManagerEtherDream :: getAndConnectToDac(const string& id){
    
    // returns a dac - if failed returns nullptr.
    
    DacEtherDream* dac = (DacEtherDream*) getDacById(id);
    if(dac!=nullptr) {
        ofLogNotice("DacManagerEtherDream :: getAndConnectToDac(...) - Already a dac made with id "+ofToString(id));
        return dac;
    }
    // todo check it exists!
    if(etherdreamDataByMacAddress.find(id)!=etherdreamDataByMacAddress.end()) {
        EtherDreamData& ed = etherdreamDataByMacAddress.at(id);
        // MAKE DAC
        dac = new DacEtherDream();
        dac->setup(id, ed.ipAddress, ed);
        dacsById[id] = dac;
        return dac;
    } else {
        return nullptr;
    }
}

bool DacManagerEtherDream :: disconnectAndDeleteDac(const string& id){
    
    DacEtherDream* dac = (DacEtherDream*)getDacById(id);
    if(dac==nullptr) {
        ofLogError("DacManagerEtherDream::disconnectAndDeleteDac("+id+") - dac not found");
        return false;
    }
   
    dac->close();
    auto it=dacsById.find(id);
    dacsById.erase(it);
    delete dac;
    return true;
    
}



void DacManagerEtherDream :: exit() {
    
}

//
//  ofxLaserDacManagerBase.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#include "ofxLaserDacManagerEtherDream.h"
#include "ByteStreamUtils.h"
#include "ofxLaserDacEtherDreamResponse.h"

using namespace ofxLaser;

DacManagerEtherDream :: DacManagerEtherDream()  {
    ofxUDPSettings settings;
    settings.bindPort = 7654;
    settings.blocking = false;
    settings.reuse = true;
    verbose = false;
    if(!udpConnection.Setup(settings)) {
        ofLogError("DacManagerEtherDream")
            << "UDP bind failed on *:7654 (Ether Dream discovery). "
            << "Will keep retrying in the background. Another app may already own this port "
            << "(for example MadMapper). Check with: lsof -nP -iUDP:7654";
    } else {
        ofLogNotice("DacManagerEtherDream")
            << "Ether Dream discovery listening on UDP :7654 (reuse enabled)";
    }
    startThread();
}

DacManagerEtherDream :: ~DacManagerEtherDream()  {
    float stoptime = ofGetElapsedTimef();
    stopThread();
    waitForThread();
    // TODO wait for all DACs threads to stop
    udpConnection.Close();
    ofLogNotice("DacManagerEtherDream :: ~DacManagerEtherDream() ") << (ofGetElapsedTimef()-stoptime) << " seconds";
    
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
    // sleep a bit the first time! Make sure not to overwhelm everything
    sleep(startUpDelayMS);
    
    const uint64_t rebindRetryIntervalMs = 2000;
    const uint64_t rebindLogIntervalMs = 10000;
    uint64_t nextRebindAttemptMs = 0;
    uint64_t lastRebindLogMs = 0;
    bool receiveErrorLogged = false;

    auto tryBindDiscoverySocket = [&]() -> bool {
        if(udpConnection.HasSocket()) {
            return true;
        }
        ofxUDPSettings settings;
        settings.bindPort = 7654;
        settings.blocking = false;
        settings.reuse = true;
        if(udpConnection.Setup(settings)) {
            ofLogNotice("DacManagerEtherDream")
                << "Recovered Ether Dream discovery socket on UDP :7654";
            return true;
        }
        return false;
    };

    while(isThreadRunning()) {
        if(!udpConnection.HasSocket()) {
            uint64_t nowMs = ofGetElapsedTimeMillis();
            if(nowMs >= nextRebindAttemptMs) {
                if(!tryBindDiscoverySocket() && ((lastRebindLogMs == 0) || ((nowMs - lastRebindLogMs) >= rebindLogIntervalMs))) {
                    ofLogError("DacManagerEtherDream")
                        << "Still unable to bind UDP :7654 for Ether Dream discovery. Retrying...";
                    lastRebindLogMs = nowMs;
                }
                nextRebindAttemptMs = nowMs + rebindRetryIntervalMs;
            }
            sleep(100);
            continue;
        }
        
        // LET'S ASSUME FOR NOW...
        // that every packet is a complete message from a single dac.
        
        int numBytesReceived = 0;
        do {
            memset(udpMessage,0,sizeof(udpMessage));
            numBytesReceived = udpConnection.Receive(udpMessage,packetSize); //returns number of bytes received
            if(numBytesReceived == SOCKET_ERROR) {
                if(!receiveErrorLogged) {
                    ofLogError("DacManagerEtherDream")
                        << "UDP receive failed on discovery socket (:7654). "
                        << "Closing socket and retrying bind.";
                    receiveErrorLogged = true;
                }
                udpConnection.Close();
                nextRebindAttemptMs = 0;
            } else {
                receiveErrorLogged = false;
            }
            
            // ofLogNotice("Received "+ ofToString(numBytesReceived) + " bytes from UDP connection");
            if(numBytesReceived >=36)  {
                const double now = ofGetElapsedTimef();
                
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
                
                
                DacEtherDreamStatus status;
                status.deserialize(byteaddress);
                
//                            cout << "Hardware version :" << hardwareRevision << endl;
//                            cout << "Software version :" << softwareRevision << endl;
//                            cout << "Buffer capacity  :" << bufferCapacity << endl;
//                            cout << "Max point rate   :" << maxPointRate << endl;
//                            cout << "Buffer           :" << status._buffer_fullness << endl;
//                            cout << "Point count      :" << status.point_count << endl;
                //
                char idchar[100];
                int part0 = macAddress & 0xffff;
                int part1 = (macAddress>>16) & 0xffff;
                int part2 = (macAddress>>32) & 0xffff;
                
                snprintf(idchar, sizeof(idchar), "%04X%04X%04X", part2, part1, part0);
                //sprintf(idchar, "%llX", macAddress);
                string id(idchar);
                
                const bool inUse = checkInUse ? (status.playback_state != 0) : false;
                if(lock()) {
                    auto it = etherdreamDataByMacAddress.find(id);
                    if(it == etherdreamDataByMacAddress.end()) {
                        EtherDreamData ed = {hardwareRevision, softwareRevision, bufferCapacity, static_cast<int>(maxPointRate), id, address, inUse, now};
                        etherdreamDataByMacAddress[id] = ed;
                        dacsChanged = true;
                        if(verbose) {
                            ofLogNotice("Adding etherdreamData "+ id) << " " << hardwareRevision << " " << softwareRevision << " " << id << " in use : " << ed.inUse;
                        }
                    } else {
                        EtherDreamData& ed = it->second;
                        ed.lastUpdateTime = now;
                        if(ed.inUse != inUse) {
                            ed.inUse = inUse;
                            dacsChanged = true;
                        }
                        if(verbose) {
                            ofLogNotice("Updating etherdreamData "+ id) << " " << hardwareRevision << " " << softwareRevision << " " << id << " in use : " << ed.inUse;
                        }
                    }
                    unlock();
                }
                
            }
            sleep(10); 
        } while (isThreadRunning() && (numBytesReceived>0));
  
        // delete dacs from the list that we haven't seen for a while
        if(isThreadRunning() && lock()) {
            
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
        for(int i = 0; (i<20) && isThreadRunning(); i++) {
            sleep(50);
        }
    }
}
    
vector<DacData> DacManagerEtherDream :: updateDacList(){
    
    vector<DacData> daclist;
    if(lock()) {
        for(auto etherdreampair : etherdreamDataByMacAddress) {
            EtherDreamData& ed = etherdreampair.second;
            // ofLogNotice(ed.macAddress);
            
            string id = ed.macAddress;
            if(verbose) ofLogNotice("DacManagerEtherDream :: updateDacList() ipaddress : ") << ed.ipAddress;
            daclist.emplace_back(getType(), id, ed.ipAddress, ed.inUse);
            
        }
        unlock();
    }
    return daclist;
}


std::shared_ptr<DacBase> DacManagerEtherDream :: getAndConnectToDac(const string& id){
    
    // returns a dac - if failed returns nullptr.
    
    std::shared_ptr<DacBase> dac = getDacById(id);
    if(dac) {
        if(verbose)  ofLogNotice("DacManagerEtherDream :: getAndConnectToDac(...) - Already a dac made with id "+ofToString(id));
        return dac;
    }
    EtherDreamData ed;
    bool found = false;
    if(lock()) {
        auto it = etherdreamDataByMacAddress.find(id);
        if(it != etherdreamDataByMacAddress.end()) {
            ed = it->second;
            found = true;
        }
        unlock();
    }

    if(!found) {
        return nullptr;
    }

    // MAKE DAC
    auto edac = std::make_shared<DacEtherDream>();
    edac->setup(id, ed.ipAddress, ed);
    dacsById[id] = edac;
    return edac;
}



void DacManagerEtherDream :: exit() {
    
}

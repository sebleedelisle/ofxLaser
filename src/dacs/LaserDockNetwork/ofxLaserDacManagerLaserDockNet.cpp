//
//  ofxLaserDacManagerBase.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//

#include "ofxLaserDacManagerLaserDockNet.h"

using namespace ofxLaser;

DacManagerLaserDockNet :: DacManagerLaserDockNet()  {
    
    verbose = false;
   // udpConnection.Setup(settings);
    startThread();
}

DacManagerLaserDockNet :: ~DacManagerLaserDockNet()  {
    float stoptime = ofGetElapsedTimef();
    stopThread();
    waitForThread();
    
    ofLogNotice("DacManagerLaserDockNet :: ~DacManagerLaserDockNet() ") << (ofGetElapsedTimef()-stoptime) << " seconds";

    udpSender.Close();
    ofLogNotice("DacManagerLaserDockNet :: ~DacManagerLaserDockNet() ") << (ofGetElapsedTimef()-stoptime) << " seconds";
}

void DacManagerLaserDockNet :: threadedFunction() {
    
    while(ofGetElapsedTimeMillis() < 2000) {
        sleep(50);
    }
    
    const int packetSize = 10000; // should only need 64
    char udpMessage[packetSize];
    
    auto & thread = getNativeThread();
    //unsigned int n = std::thread::hardware_concurrency();
    //    std::cout << n << " concurrent threads are supported.\n";
    
#ifndef _MSC_VER
    // only linux and osx
    //http://www.yonch.com/tech/82-linux-thread-priority
    struct sched_param param;
    param.sched_priority = 10; // (highest) sched_get_priority_max(SCHED_FIFO);//89; // - higher is faster
    int result =  pthread_setschedparam(thread.native_handle(), SCHED_RR, &param );
    
    if (result != 0) {
        std::cerr << "Failed to set thread priority: " << strerror(result) << "\n";
    }
#else
    // windows implementation
    SetThreadPriority( thread.native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
#endif
    
    
    while(isThreadRunning()) {
        int numBytesSent = 0;
        
        char cmd[] = {DacLaserDockNetConsts::CMD_GET_FULL_INFO};
        
        try {
            ofxUDPSettings udpsendsettings;
            udpsendsettings.sendTo("255.255.255.255", DacLaserDockNetConsts::CMD_PORT);
            udpsendsettings.blocking = false;
            udpsendsettings.broadcast = true;
            udpsendsettings.sendTimeout = 1;
            udpsendsettings.receiveTimeout = 0.2;
            udpsendsettings.reuse = true;
        
            udpSender.Setup(udpsendsettings);
            udpSender.SendAll( (char*)&cmd[0],1);

        }
        catch (Poco::TimeoutException& exc) {
            //Handle your network errors.
            ofLogError() << "DacManagerLaserDockNet :: threadedFunction() sendBytes : Timeout error: " << exc.displayText() << endl;
            //    isOpen = false;
            //failed = true;
        } catch (Poco::Exception& exc) {
            //Handle your network errors.
            ofLogError() << "DacManagerLaserDockNet :: threadedFunction() sendBytes : Network error: " << exc.displayText() <<" " << exc.code() << " "  << exc.message() << " " <<  endl;
            //networkerror = true;
            //failed = true;
            //ofLogNotice() << commandUdpSocket.address().toString();
       
        } catch (...) {
            
            ofLogError() << "DacManagerLaserDockNet :: threadedFunction() sendBytes : unspecified error " << endl;
        }
        
        
        
        // LET'S ASSUME FOR NOW...
        // that every packet is a complete message from a single dac.
        
        float sendTime = ofGetElapsedTimef();
        
        int numBytesReceived = 0;
        do {
            memset(udpMessage,0,sizeof(udpMessage));
            int peekbytes = udpSender.PeekReceive();
             
//            numBytesReceived = udpSender.Receive(udpMessage, packetSize);
//            auto t0 = std::chrono::steady_clock::now();
            int numBytesReceived = udpSender.Receive(udpMessage, packetSize);
//            auto t1 = std::chrono::steady_clock::now();
//            std::cout << "Receive took: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms\n";

            if(numBytesReceived>0) {

                if(verbose) {
                    ofLogNotice() << "DacManagerLaserDockNet thread ----------------------------------------------------------------------";

                    ofLogNotice("PeekReceive before: ") <<  peekbytes ;//
                    ofLogNotice("PeekReceive after : ") << udpSender.PeekReceive();
                    ofLogNotice() << "Packet Size: " << numBytesReceived ;
                    ofLogNotice() << "Buffer Size: " << udpSender.GetReceiveBufferSize() ;
                    
                }
                
                
                if(numBytesReceived>=64) {
                    DacLaserDockNetStatus status;
                    status.deserialize((unsigned char*)(&udpMessage));
                    if(verbose) {
                        ofLogNotice() << status.toString();
                    }
                    string id = status.serial_number;
                    
                   // if we haven't already got this DAC, then add it
                   if(dacStatusById.find(id) == dacStatusById.end()) {
    
                       if(lock()) {
                            dacStatusById[id] = status;
                            dacsChanged = true;
                            unlock();
                        }
                    } else {
    
                        if(lock()) {
                            dacStatusById[id] = status;
                            dacStatusById[id].lastUpdateTime = ofGetElapsedTimef();
                            unlock();
                            std::shared_ptr<DacBase> dac = getDacById(id);
                            std::shared_ptr<DacLaserDockNet> daclaserdock = std::dynamic_pointer_cast<DacLaserDockNet>(dac);
                            if(daclaserdock) {
                                if(daclaserdock->pushStatus(status)) {
                                    dacsChanged = true;
                                }
                                     
                            }
                        }
                    }
                    
                }
            }
            sleep(100);
        } while (isThreadRunning() && ((ofGetElapsedTimef()-sendTime)<1));
        
        udpSender.Close();

        // delete devices we haven't seen for a while
        if(isThreadRunning() && lock()) {

            for (auto it = dacStatusById.cbegin(); it != dacStatusById.cend() /* not hoisted */; /* no increment */)  {
                const DacLaserDockNetStatus& status = it->second;
                if ((ofGetElapsedTimef() - status.lastUpdateTime)>3){
                   dacStatusById.erase(it++);    // or "it = m.erase(it)" since C++11
                    if(verbose) {
                        ofLogNotice("Erasing DAC : ") << it->first << " " << status.serial_number << " " << (ofGetElapsedTimef() - status.lastUpdateTime);
                    }
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
    
vector<DacData> DacManagerLaserDockNet :: updateDacList(){
    
    vector<DacData> daclist;

    for(auto LaserDockNetpair : dacStatusById) {
        DacLaserDockNetStatus& status = LaserDockNetpair.second;
       // ofLogNotice(ed.macAddress);
        string id = status.serial_number;
        status.connection_type;
        daclist.emplace_back(getType(), id, status.ip_address, false);
    }
    
    //daclist.emplace_back(getType(), "LaserDockNetTest", "192.168.0.162");

    return daclist;
}


std::shared_ptr<DacBase> DacManagerLaserDockNet :: getAndConnectToDac(const string& id){
    
    // returns a dac - if failed returns nullptr.

    std::shared_ptr<DacBase> dac = getDacById(id);
    if(dac) {
        ofLogNotice("DacManagerLaserDockNet :: getAndConnectToDac(...) - Already a dac made with id "+ofToString(id));
        return dac;
    }
    // check it exists!
    if(dacStatusById.find(id)!=dacStatusById.end()) {
        DacLaserDockNetStatus& status = dacStatusById.at(id);
        // MAKE DAC
        DacLaserDockNet* edac = new DacLaserDockNet();
        edac->setup(id, status.ip_address, status);
        dacsById.emplace(std::make_pair(id, edac));
        return dacsById[id];
    } else {
        return nullptr;
    }
}

void DacManagerLaserDockNet :: exit() {
    
}

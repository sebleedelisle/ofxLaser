//
//  ofxLaserDacManagerBase.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//

#include "ofxLaserDacManagerLaserDockNet.h"

using namespace ofxLaser;

DacManagerLaserDockNet :: DacManagerLaserDockNet()  {


    
    try {

       // auto ip = address.get<Poco::Net::NetworkInterface::BROADCAST_ADDRESS>();
        //commandUdpSocket.bind(
        Poco::Net::SocketAddress sa("255.255.255.255", DacLaserDockNetConsts::ALIVE_PORT);
        Poco::Net::SocketAddress la("192.168.1.2", DacLaserDockNetConsts::ALIVE_PORT);
        //commandUdpSocket.bind(la);
        
        //commandUdpSocket = Poco::Net::DatagramSocket(sa);
        //commandUdpSocket.setBlocking(false);
       //commandUdpSocket.setBroadcast(true);
        commandUdpSocket.bind(la);
        commandUdpSocket.connect(sa);
        commandUdpSocket.setBroadcast(true);
        commandUdpSocket.setBlocking(false);
        
        ofLogNotice("DacManagerLaserDockNet socket : ") << commandUdpSocket.address().toString();

        connected = true;
    } catch (Poco::Exception& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "DacLaserDockNet setup failed - Network error: " + exc.displayText());
        connected = false;

    }catch (Poco::Net::HostNotFoundException& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "DacLaserDockNet setup failed - host not found: " + exc.displayText());
        connected = false;
        
    }catch (Poco::TimeoutException& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "DacLaserDockNet setup failed - Timeout error: " + exc.displayText());
        connected = false;
        
    }
    catch(...){
        ofLog(OF_LOG_ERROR, "DacLaserDockNet setup failed - unknown error");
        //std::rethrow_exception(current_exception);
        connected = false;
    }
    
   // udpConnection.Setup(settings);
    startThread();
}

DacManagerLaserDockNet :: ~DacManagerLaserDockNet()  {
    stopThread();
    waitForThread();
    // TODO wait for all DACs threads to stop
    commandUdpSocket.close();
}

void DacManagerLaserDockNet :: threadedFunction() {
    
    const int packetSize = 80; // should only need 64
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
        int numBytesSent = 0;
        
        char cmd[] = {DacLaserDockNetConsts::CMD_GET_ALIVE, 0};
        
        try {
            //Poco::Net::SocketAddress sa("10.0.1.255", DacLaserDockNetConsts::ALIVE_PORT);
            //commandUdpSocket.sendTo(cmd, 1, sa);
            commandUdpSocket.sendBytes(cmd, 1);
        }
        catch (Poco::Exception& exc) {
            //Handle your network errors.
            std::cerr << "sendBytes : Network error: " << exc.displayText() << endl;
            //networkerror = true;
            //failed = true;
            ofLogNotice() << commandUdpSocket.address().toString();
        }
        catch (Poco::TimeoutException& exc) {
            //Handle your network errors.
            std::cerr << "sendBytes : Timeout error: " << exc.displayText() << endl;
            //    isOpen = false;
            //failed = true;
        } catch (...) {
            
            std::cerr << "sendBytes : unspecified error " << endl;
        }
        
        
        
        // LET'S ASSUME FOR NOW...
        // that every packet is a complete message from a single dac.
        
        float sendTime = ofGetElapsedTimef();
        
        int numBytesReceived = 0;
        do {
            memset(udpMessage,0,sizeof(udpMessage));
            Poco::Net::SocketAddress socketAddress;
            //numBytesReceived = commandUdpSocket.receiveFrom(udpMessage,packetSize, socketAddress); //returns number of bytes received
            numBytesReceived = commandUdpSocket.receiveFrom(udpMessage,packetSize, socketAddress);
            if(numBytesReceived >=1)  {
                
               // ofLogNotice("Received "+ ofToString(numBytesReceived) + " bytes from UDP connection ") << ;
                
                
                
                string address = socketAddress.toString();
                int port = socketAddress.port();
                
                
                std::cout << "----------------------------------------------------------------------"<< std::endl;
                            std::cout << "ip: " << address << " " << port << std::endl;
                            std::cout << "Packet Size: " << numBytesReceived << std::endl;
                            std::cout << "UDP Packet: " << std::endl;
                
                
                if(numBytesReceived>=64) {
                    DacLaserDockNetStatus status;
                    status.deserialize((unsigned char*)(&udpMessage));
                    cout << status.toString() << endl;
                    
                    string id = status.serial_number;
                    
    //                // if we haven't already got this LaserDockNet, then add it
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
                        }
                    }
                    
                }
            }
            sleep(10); 
        } while ((ofGetElapsedTimef()-sendTime)<1);
        
//        if(ofGetElapsedTimef()-lastCheckTime > 2) {
//
//        }
//
        
        // delete devices we haven't seen for a while
        if(lock()) {

            for (auto it = dacStatusById.cbegin(); it != dacStatusById.cend() /* not hoisted */; /* no increment */)  {
                const DacLaserDockNetStatus& status = it->second;
                if ((ofGetElapsedTimef() - status.lastUpdateTime)>2){
                    dacStatusById.erase(it++);    // or "it = m.erase(it)" since C++11
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
    
vector<DacData> DacManagerLaserDockNet :: updateDacList(){
    
    vector<DacData> daclist;

    for(auto LaserDockNetpair : dacStatusById) {
        DacLaserDockNetStatus& status = LaserDockNetpair.second;
       // ofLogNotice(ed.macAddress);

        string id = status.serial_number;
        daclist.emplace_back(getType(), id, status.ip_address);

    }
    
    daclist.emplace_back(getType(), "LaserDockNetTest", "192.168.0.162");

    return daclist;
}


DacBase* DacManagerLaserDockNet :: getAndConnectToDac(const string& id){
    
    // returns a dac - if failed returns nullptr.

    DacLaserDockNet* dac = (DacLaserDockNet*) getDacById(id);
    if(dac!=nullptr) {
        ofLogNotice("DacManagerLaserDockNet :: getAndConnectToDac(...) - Already a dac made with id "+ofToString(id));
        return dac;
    }
    // check it exists!
    if(dacStatusById.find(id)!=dacStatusById.end()) {
        DacLaserDockNetStatus& status = dacStatusById.at(id);
        // MAKE DAC
        dac = new DacLaserDockNet();
        dac->setup(id, status.ip_address, status);
        dacsById[id] = dac;
        return dac;
    } else {
        return nullptr;
    }
}

bool DacManagerLaserDockNet :: disconnectAndDeleteDac(const string& id){
    
    DacLaserDockNet* dac = (DacLaserDockNet*)getDacById(id);
    if(dac==nullptr) {
        ofLogError("DacManagerLaserDockNet::disconnectAndDeleteDac("+id+") - dac not found");
        return false;
    }
   
    dac->close();
    auto it=dacsById.find(id);
    dacsById.erase(it);
    delete dac;
    return true;
    
}



void DacManagerLaserDockNet :: exit() {
    
}

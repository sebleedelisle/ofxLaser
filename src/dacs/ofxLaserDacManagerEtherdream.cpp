//
//  ofxLaserDacManagerBase.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 29/03/2021.
//

#include "ofxLaserDacManagerEtherdream.h"

using namespace ofxLaser;

DacManagerEtherdream :: DacManagerEtherdream()  {
    ofxUDPSettings settings;
    settings.bindPort = 7654;
    settings.blocking = false;
    
    udpConnection.Setup(settings);
    startThread();
    
    labelById = {
        {"5EE67D9E3666","EtherDream A2"}

    };

    
}
DacManagerEtherdream :: ~DacManagerEtherdream()  {
    
    stopThread();
    waitForThread();
    // TODO wait for all DACs threads to stop
    udpConnection.Close();

}

void DacManagerEtherdream :: threadedFunction() {
    
    const int packetSize = 50;
    char udpMessage[packetSize];

    while(isThreadRunning()) {
        
        // LET'S ASSUME FOR NOW...
        // that every packet is a complete message from a single dac.
        
        memset(udpMessage,0,sizeof(udpMessage));
        int numBytesReceived = udpConnection.Receive(udpMessage,packetSize); //returns number of bytes received
        
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
            unsigned long macAddress=0;
            int i = 0;
            for(i = 0; i < 6 ; i++) {
                macAddress<<=8;
                macAddress|=(unsigned char)udpMessage[i];
            }
            //cout << endl;
           // printf("Mac Address : %lx\n", macAddress);
            uint16_t hardwareRevision, softwareRevision, bufferCapacity;
            uint32_t maxPointRate;
            unsigned char* byteaddress = (unsigned char*)&udpMessage[i];
            hardwareRevision = DacEtherdream::bytesToUInt16(byteaddress);
            byteaddress+=2;
            softwareRevision = DacEtherdream::bytesToUInt16(byteaddress);
            byteaddress+=2;
            bufferCapacity = DacEtherdream::bytesToUInt16(byteaddress);
            byteaddress+=2;
            maxPointRate = DacEtherdream::bytesToUInt32(byteaddress);
            byteaddress+=4;
            
          
            unsigned char* buffer = byteaddress-2;
            dac_status status;
            status.protocol = buffer[2];
            status.light_engine_state = buffer[3];
            status.playback_state = buffer [4];
            status.source = buffer[5];
            status.light_engine_flags =  DacEtherdream::bytesToUInt16(&buffer[6]);
            status.playback_flags =   DacEtherdream::bytesToUInt16(&buffer[8]);
            status.source_flags =   DacEtherdream::bytesToUInt16(&buffer[10]);
            status.buffer_fullness =  DacEtherdream::bytesToUInt16(&buffer[12]);
            status.point_rate =  DacEtherdream::bytesToUInt32(&buffer[14]);
            status.point_count =  DacEtherdream::bytesToUInt32(&buffer[18]);
            
//            cout << "Hardware version :" << hardwareRevision << endl;
//            cout << "Software version :" << softwareRevision << endl;
//            cout << "Buffer capacity  :" << bufferCapacity << endl;
//            cout << "Max point rate   :" << maxPointRate << endl;
//            cout << "Buffer           :" << status.buffer_fullness << endl;
//            cout << "Point count      :" << status.point_count << endl;
           
            char idchar[100];
            sprintf(idchar, "%lX", macAddress);
            string id(idchar);
            
            // if we haven't already got this etherdream, then add it
            if(etherdreamDataByMacAddress.find(id) == etherdreamDataByMacAddress.end()) {
                EtherdreamData ed = {hardwareRevision, softwareRevision,bufferCapacity, (int) maxPointRate, id, address, ofGetElapsedTimef()};
                ofLogNotice("Adding etherdream "+ id);
                etherdreamDataByMacAddress[id] = ed;
            } else {
                
                etherdreamDataByMacAddress[id].lastUpdateTime = ofGetElapsedTimef(); 
            } 
            
        }
        sleep(10);
    }
            
     
}
    
vector<DacData> DacManagerEtherdream :: updateDacList(){
    
    vector<DacData> daclist;
    
    for(auto etherdreampair : etherdreamDataByMacAddress) {
        EtherdreamData& ed = etherdreampair.second;
        ofLogNotice(ed.macAddress);
        
        string id = ed.macAddress;
        
        // if we last got an update from the etherdream less
        // than two seconds ago, add it to the list.
        //ofLogNotice("lastUpdateTime : " ) << (ofGetElapsedTimef() - ed.lastUpdateTime);
        if((ofGetElapsedTimef() - ed.lastUpdateTime)<2){
            daclist.emplace_back(getType(), id, ed.ipAddress);
            // here is where to look up the label!
            if(labelById.find(id)!=labelById.end()) {
                daclist.back().label = labelById[id];
                
            }
        }
        
    }

    return daclist;
    
}


DacBase* DacManagerEtherdream :: getAndConnectToDac(const string& id){
    
    // returns a dac - if failed returns nullptr.
    
    DacEtherdream* dac = (DacEtherdream*) getDacById(id);
    if(dac!=nullptr) {
        ofLogNotice("DacManagerEtherdream :: getAndConnectToDac(...) - Already a dac made with id "+ofToString(id));
        return dac;
    }
    // todo check it exists!
    EtherdreamData& ed = etherdreamDataByMacAddress.at(id);
    // MAKE DAC
    dac = new DacEtherdream();
    dac->setup(id, ed.ipAddress);
    dacsById[id] = dac;
    return dac;
}

bool DacManagerEtherdream :: disconnectAndDeleteDac(const string& id){
    
    DacEtherdream* dac = (DacEtherdream*)getDacById(id);
    if(dac==nullptr) {
        ofLogError("DacManagerEtherdream::disconnectAndDeleteDac("+id+") - dac not found");
        return false;
    }
   
    dac->close();
    auto it=dacsById.find(id);
    dacsById.erase(it);
    delete dac;
    return true;
    
}



void DacManagerEtherdream :: exit() {
    
}

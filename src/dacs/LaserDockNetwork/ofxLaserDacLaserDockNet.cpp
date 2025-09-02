//
//  ofxLaserDacLaserDockNet.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//
//

#include "ofxLaserDacLaserDockNet.h"

using namespace ofxLaser;

DacLaserDockNet :: DacLaserDockNet(){
    // the maximum points in the dac's buffer
    // this is passed in when
    // the LaserDockNet is initialised
    pointBufferCapacity = 1000;
   // minPacketDataSize = 1;
    //lastCommandSendTime = 0;
    lastAckTimeMicros = 0;
    lastDataSentTimeMicros = 0;
    hasTemperature = true;
    hasBatteryLevel = true;
    colourShiftImplemented = true;
    verbose = false;
}



DacLaserDockNet :: ~DacLaserDockNet(){
    
    // sends stop command, stops the thread, closes the socket
    close();
    
    cleanUpFramesAndPoints();
    
}


float DacLaserDockNet :: getBatteryLevel() {
    getLaserDockStatus();
    if(laserDockNetData.battery_percent>100) {
        return 1.0f;
    } else {
        return (float)laserDockNetData.battery_percent/100.0f;
    }
    
}

bool DacLaserDockNet :: getIsPluggedIn() {
    getLaserDockStatus();
    if(laserDockNetData.battery_percent==255) {
        return true;
    } else {
        return false;
    }
    
}
void DacLaserDockNet :: setup(string _id, string _ip, DacLaserDockNetStatus status) {
    
    pps = status.point_rate;
    newPPS = 30000; // this is always sent on begin if different
    maxPointRate = status.point_rate_max;
    pointBufferCapacity = status.buffer_max;//*0.75;
    
    networkConnected = false;
    ipAddress = _ip;
    id = _id;
    laserDockNetData = status;
    lastAckTimeMicros = ofGetElapsedTimeMicros();
    
    versionString = status.firmware_version;
    
    
    int commandPort = DacLaserDockNetConsts::CMD_PORT;
    
    // update buffer from status
    
    
    try {
        
        Poco::Net::SocketAddress sa(ipAddress, DacLaserDockNetConsts::DATA_PORT);
        dataUdpSocket.connect(sa);
        
        networkConnected = true;
 
    }catch (Poco::Net::HostNotFoundException& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "DacLaserDockNet setup failed - host not found: " + exc.displayText());
        networkConnected = false;
        
    }catch (Poco::TimeoutException& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "DacLaserDockNet setup failed - Timeout error: " + exc.displayText());
        networkConnected = false;
    } catch (Poco::Exception& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "DacLaserDockNet setup failed - Network error: " +ipAddress+" "+ exc.displayText());
        networkConnected = false;
        
    } catch(...){
        ofLog(OF_LOG_ERROR, "DacLaserDockNet setup failed - unknown error");
        //std::rethrow_exception(current_exception);
        networkConnected = false;
    }
    
    try {
        
        Poco::Net::SocketAddress sa(ipAddress, DacLaserDockNetConsts::CMD_PORT);
        commandUdpSocket.connect(sa);
        
        networkConnected &= true;
        
        
    } catch (Poco::Exception& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "DacLaserDockNet setup failed - Network error: " +ipAddress+" "+ exc.displayText());
        networkConnected = false;
        
    } catch(...){
        ofLog(OF_LOG_ERROR, "DacLaserDockNet setup failed - unknown error");
        //std::rethrow_exception(current_exception);
        networkConnected = false;
    }
    
    if(networkConnected) {
        dataUdpSocket.setBlocking(false);
        startThread(); // blocking is true by default I think?
    }
}

void DacLaserDockNet :: threadedFunction(){
    
    auto & thread = getNativeThread();
    
#ifndef _MSC_VER
    // only linux and osx
    //http://www.yonch.com/tech/82-linux-thread-priority
    struct sched_param param;
    param.sched_priority = 90; // (highest) sched_get_priority_max(SCHED_FIFO);//89; // - higher is faster
    pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &param );
#else
    // windows implementation
    SetThreadPriority( thread.native_handle(), THREAD_PRIORITY_HIGHEST);
#endif
    
    
    while(isThreadRunning() ) {
        
        
        // if we have new point rate, send it
        // pps = points per second
        if(networkConnected && (newPPS!=pps)) {
            sendPointRate(newPPS); // assume it was sent i guess? Or periodically send it?
            pps = (uint32_t)newPPS;
        }
        
        
        if(networkConnected && isThreadRunning()) {
            
            //waitUntilReadyToSend();
            
            //check buffer and send the next points
            
            bool dataSent = sendPointsToDac();
            
            if(!dataSent) {
                //checkDataPortIncoming();
                //connected = false;
               // ofLogNotice("DacLaserDockNet :: threadedFunction : Data send failed");
                
            } else {
                // probably because there are no points in the buffer.
                //cout << "DATA NOT SENT " << endl;
                if(messageNumber%25==0) {
                    sleep(10);
                }
            }
            
            checkAcks();
            
        } else {
            // get rid of frames!
            std::shared_ptr<DacFrame>frame;
            while(frameThreadChannel.tryReceive(frame) && isThreadRunning()) {
                //bufferedFrames.push_back(frame);
                //newFrame = true;
                //delete frame;
            }
            
        }
        
        // TODO - check connected state
        
        sleep(1);
        //yield();
    }
    
    
}



void DacLaserDockNet :: reset() {
    if(lock()) {
        resetFlag = true;
        unlock();
    }
    
}

void DacLaserDockNet :: closeWhileRunning() {
    //if(!networkConnected) return;
    
    waitForThread();
    
    while(!lock())
        ;
    dataUdpSocket.close();
    commandUdpSocket.close();
    unlock();
    
    
}




inline bool DacLaserDockNet :: sendPointsToDac(){
    
    
    // add points from the frame queue to the bufferedPoints up to that minimum level
    updateFrameQueue();
    
    frameNumber++; // bit meaningless as we're sending a stream, but maybe it will keep the DAC happy ?
    
    
    int minEstimatedBufferFullness = calculateBufferFullnessByTimeSent();
    
    int latencyPointAdjustment = 300; // TODO calculate based on point rate / latency
    int maxPointsToAdd = MAX(0, getDacTotalPointBufferCapacity() - minEstimatedBufferFullness - latencyPointAdjustment);
    
    int numpointstoadd = MIN(bufferedPoints.size(), maxPointsToAdd);
    
    int maxPointsInPacket = 140; // why 140??? 
    
    if(numpointstoadd < minPacketDataSize) {
        //        if(verbose) {
        //            ofLogNotice("Not enough points to add yet ") << numpointstoadd << " " << minPacketDataSize;
        //        }
        return true;
    }
    
    if(verbose) {
        ofLogNotice("------------------------------- sendPointsToDac");
        ofLogNotice("numpointstoadd : ") <<  numpointstoadd;
        //ofLogNotice("maxEstimatedBufferFullness : ") <<  maxEstimatedBufferFullness;
        //ofLogNotice("bufferedPoints.size() : ") <<  bufferedPoints.size();
    }
    
    if(numpointstoadd>maxPointsInPacket) numpointstoadd = maxPointsInPacket;
    
    bool success  = true;
    
    //uint8_t lastMessageNum =-1;
    
    // it's just a point object to use, nothing special
    LaserDockNetDacPoint& dacPoint = sendPoint;
    
    // the colour shift delay in point count
    int colourShiftInPoints =  (float)pps/10000.0f*colourShift ;
    
    if(verbose)  {
        ofLogNotice("Sending ") << numpointstoadd << " points... ";
    }
    string verboseMsg;
    if(verbose) verboseMsg = "Packets : ";
    
    dacCommand.setDataCommand(messageNumber, frameNumber); // packetNumber is a uint8_t so hopefully should overflow itself
    for(int i = 0; (i<numpointstoadd); i++) {
        
        if(bufferedPoints.size()>0) {
            // pop the point off the front
            // TODO figure out how to add extra points in the buffer to accommodate the
            // colour shift
            int pointindex = colourShiftInPoints;
            if(pointindex >= bufferedPoints.size()) pointindex = bufferedPoints.size()-1;
            
            ofxLaser::Point& laserPoint = *bufferedPoints[pointindex];
            ofxLaser::Point& colourPoint = *bufferedPoints[0];
            
            dacPoint.x = ofMap(armed ? laserPoint.x : 400, 800, 0, LaserDockNet_MIN, LaserDockNet_MAX);  // seems flipped!
            dacPoint.y = ofMap(armed ? laserPoint.y : 400, 800, 0, LaserDockNet_MIN, LaserDockNet_MAX); // Y is UP
            
            
            if(! armed || blankPointsToSend>0) {
                dacPoint.r =  dacPoint.g = dacPoint.b = 0;
                if(blankPointsToSend>0) blankPointsToSend--;
                
            } else {
                dacPoint.r = colourPoint.r/255.0f*0xFFF;
                dacPoint.g = colourPoint.g/255.0f*0xFFF;
                dacPoint.b = colourPoint.b/255.0f*0xFFF;
            }
            
            
            PointFactory :: releasePoint(bufferedPoints[0]); // recycling system
            bufferedPoints.pop_front(); // no longer destroys point
            lastPointSent = dacPoint; //
        } else  {
            
            // THIS SHOULD NEVER HAPPEN!!!
            // EXCEPT MAYBE IN STREAM MODE
            // just send some blank points in the same position as the
            // last point
            
            dacPoint = lastPointSent;
            
            dacPoint.r = 0;
            dacPoint.g = 0;
            dacPoint.b = 0;
            
        }
        
        dacCommand.addPoint(dacPoint);
       
        
        
    }
    
    if(!sendData(dacCommand)) {
        success = false;
        if(verbose) ofLogNotice("Send data failed");
    } else {
       // if(verbose) ofLogNotice("     Points sent : ") << (int)messageNumber << " " << pointsInPacket ;
        messageTimes[messageNumber] = ofGetElapsedTimeMicros();
        
    }
    
    messageNumber++;
  //  packetcount++;
//    if(verbose) verboseMsg +=ofToString(packetcount) +  "..";
//    // I'm told this is necessary but it never seems to send more than 25 in one go
//    if(packetcount%25==0) {
//        // sleep(10);
//        verboseMsg+=" | ";
//        //packetcount = 0;
//    }
//        
    
    // THIS SEEMS ODD, only called on the last packet...
    if(success) {
        //messageTimes[messageNumber] = ofGetElapsedTimeMicros();
        
        lastDataSentTimeMicros = ofGetElapsedTimeMicros();
        lastDataSentBufferSize = minEstimatedBufferFullness + numpointstoadd;
    }  else {
       // logNotice("sendCommand failed!");
    }
    
    
    if(verbose) ofLogNotice() << verboseMsg;
    
    
    return success;
    
}
string DacLaserDockNet :: getRawId(){
    return id;
}

// TODO could this be a conflict?
int DacLaserDockNet :: getStatus(){
    
    
    uint64_t timeMicros  =  ofGetElapsedTimeMicros();
    uint64_t timeSinceLastAck = timeMicros-lastAckTimeMicros;
    //ofLogNotice() << ofGetElapsedTimeMillis() - lastDroppedPacketTimeMs;
    if(timeSinceLastAck>2000000) {
        return OFXLASER_DACSTATUS_ERROR;
    } else if(ofGetElapsedTimeMillis() - lastDroppedPacketTimeMs < 5000) {
        return OFXLASER_DACSTATUS_WARNING;
    } else {
        return OFXLASER_DACSTATUS_GOOD;
    }
    
    //return 0;
    //	if(!connected) return OFXLASER_DACSTATUS_ERROR;
    //    int status = 0;
    //    if(lock()) {
    //        status = response.status.playback_state;
    //        unlock();
    //    }
    //
    //	if(status <=1) return OFXLASER_DACSTATUS_WARNING;
    //	else if(status ==2) return OFXLASER_DACSTATUS_GOOD;
    //	else return OFXLASER_DACSTATUS_ERROR;
}

inline bool DacLaserDockNet :: sendPointRate(uint32_t rate){
    if(verbose) ofLogNotice("Sending point rate : ") << rate;
    dacCommand.setPointRateCommand(rate);
    return sendCommand(dacCommand);
}

void DacLaserDockNet :: logData() {
    dacCommand.logData();
    
}

bool DacLaserDockNet :: pushStatus(DacLaserDockNetStatus& newstatus) {
    if(newstatus!=laserDockNetData) { // do i need comparison function ?
        statusChannel.send(newstatus);
        return true;
    } else {
        return false;
    }
}

//// information about the device, IP address, MAC address, version number etc
//DacLaserDockNetStatus laserDockNetData;


bool DacLaserDockNet :: sendData(DacLaserDockNetCommand& command) { // sendBytes(const uint8_t* buffer, int length) {
    
    const uint8_t* buffer = command.getBuffer();
    int length = command.size();
    
    int numBytesSent = 0;
    bool failed = false;
    bool networkerror = false;
    //lastCommandSendTime = ofGetElapsedTimeMicros();//  count = 0;
    
    try {
        numBytesSent = dataUdpSocket.sendBytes(buffer, length);
        //		if(verbose && (length>1)) {
        //			cout << "command sent : " << buffer[0] << " numBytesSent : " << numBytesSent <<  endl;
        //			for(int i = 1; i<length; i++) {
        //
        //				printf("%X2 ", (uint8_t)buffer[i]);
        //				if(i%8==0) cout << endl;
        //			}
        //			cout << endl;
        //		}
    }
    catch (Poco::TimeoutException& exc) {
        //Handle your network errors.
        ofLogNotice() << "sendBytes : Timeout error: " << exc.displayText() << endl;
        //    isOpen = false;
        failed = true;
    }
    catch (Poco::Exception& exc) {
        //Handle your network errors.
        ofLogNotice() << "sendBytes : Network error: " << exc.displayText() << endl;
        networkerror = true;
        failed = true;
     
    } catch (...) {
        
        ofLogNotice() << "sendBytes : unspecified error " << endl;
    }
    if(numBytesSent!=length) {
        //do something!
        ofLogNotice() << "send fail, fewer bytes sent than expected : "<< numBytesSent << endl;
        failed = true;
    } else if (numBytesSent<0) {
        //do something!
        ofLogNotice() << "send fail, sendBytes returned : "<< numBytesSent << endl;
        failed = true;
    }
    
    if(failed) {
        if(networkerror) {
            closeWhileRunning();
            networkConnected = false;
            
           // setup(id, ipAddress, laserDockNetData);
        }
        
        
        return false;
    }
    return true;
}

bool DacLaserDockNet :: checkAcks() {
    
    // so the logic here is...
    // we have a list of messages we're waiting for acks on
    // while that is not empty
    // keep checking for acks for a set period of time or until we get the one we're waiting for
    // if we get one, store the latency/frame  for that ack
        // if it's really late, then register a network issue
    // if we got all the acks, return true
    
    int numBytesReceived = 0;
    bool failed = false;

    try {
        numBytesReceived = dataUdpSocket.receiveBytes(inBuffer, 4);
    } catch (Poco::Exception& exc) {
        ofLog(OF_LOG_ERROR,  "Network error: " + exc.displayText());
        failed = true;
    } catch (Poco::TimeoutException& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "Timeout error: " + exc.displayText());
        failed = true;
    }
    
    if(numBytesReceived==4) {
        if(inBuffer[0] == DacLaserDockNetConsts::CMD_GET_RINGBUFFER_EMPTY_SAMPLE_COUNT) {
            
            // get message number
            int receivedMessageNumber = (inBuffer[1]);
            // is it in the list we're waiting for?
            
            if(messageTimes.find(receivedMessageNumber) !=messageTimes.end()) {
                
                uint64_t timeMicros  =  ofGetElapsedTimeMicros();
                lastAckTimeMicros = timeMicros;
                uint64_t commandSentTime = messageTimes.at(receivedMessageNumber);
                
                int roundTripTimeMicros  = lastAckTimeMicros - commandSentTime;
                int bufferSpace = ByteStreamUtils::bytesToUInt16(&inBuffer[2]);
               
                
                // todo should probably check that this message responses is greater than the last one
                lastReportedBufferFullness = getDacTotalPointBufferCapacity() - bufferSpace;
                lastAckTimeMicros = timeMicros;
                
                if(commandSentTime>=lastDataSentTimeMicros) {
                    if(verbose) ofLogNotice("correcting last data sent buffer size : ") << lastDataSentBufferSize << " >> " <<  lastReportedBufferFullness;
                    lastDataSentTimeMicros = commandSentTime;
                    lastDataSentBufferSize = lastReportedBufferFullness;
                }
                //ofLogNotice("buffer reported : ") << receivedMessageNumber << " reported : " << lastReportedBufferFullness << " by time sent : " << calculateBufferFullnessByTimeSent();
                
                stateRecorder.recordStateThreadSafe(lastDataSentTimeMicros, 1, lastReportedBufferFullness, roundTripTimeMicros, dacCommand.numPoints, pps, 0);
                
                // cout << lastReportedBufferFullness << endl;
                if(verbose) {
                    ofLogNotice("BUFFER REPORTED : ") << receivedMessageNumber << " "<< lastReportedBufferFullness << " " ;
                }
                if(bufferSpace ==0) {
                    ofLogNotice("BUFFER OVERRUN ------------------");
                }
                
                messageTimes.erase(receivedMessageNumber);
                
            }
            
           
            
        } else {
            logNotice("DIFFERENT RESPONSE! "+ofToString(inBuffer[0]));
        }
        
    } else if(numBytesReceived>0){
        logNotice("ofxLaserDacLaserDockNet :: message received is greater than 4 bytes : " +ofToString(numBytesReceived));
        
    }
    //usleep(1);
    
  
 

    if(messageTimes.size()>0) {
        uint64_t currenttime = ofGetElapsedTimeMicros();
        map<uint8_t, uint64_t>::iterator it;

        for(it = messageTimes.begin(); it != messageTimes.end(); ) {
            
            uint64_t time = it->second;
            uint64_t delay = currenttime - time;
            
            if(delay>10000000) { // 1s
                logNotice(ofToString((int)it->first) + " " + ofToString(delay/1000)  + " << DELETED ");
                it = messageTimes.erase(it);
                lastDroppedPacketTimeMs = ofGetElapsedTimeMillis();
            } else {
                it++;
            }
        }
    }
   

    return failed;// || timedOut;
    
}
bool DacLaserDockNet :: sendCommand(DacLaserDockNetCommand& command) { // sendBytes(const uint8_t* buffer, int length) {
    
    const uint8_t* buffer = command.getBuffer();
    int length = command.size();
    
    int numBytesSent = 0;
    bool failed = false;
    bool networkerror = false;
   // lastCommandSendTime = ofGetElapsedTimeMicros();//  count = 0;
    
    try {
        numBytesSent = commandUdpSocket.sendBytes(buffer, length);
        //        if(verbose && (length>1)) {
        //            cout << "command sent : " << buffer[0] << " numBytesSent : " << numBytesSent <<  endl;
        //            for(int i = 1; i<length; i++) {
        //
        //                printf("%X2 ", (uint8_t)buffer[i]);
        //                if(i%8==0) cout << endl;
        //            }
        //            cout << endl;
        //        }
    }
    catch (Poco::Exception& exc) {
        //Handle your network errors.
        std::cerr << "sendBytes : Network error: " << exc.displayText() << endl;
        networkerror = true;
        failed = true;
        
    }
    catch (Poco::TimeoutException& exc) {
        //Handle your network errors.
        std::cerr << "sendBytes : Timeout error: " << exc.displayText() << endl;
        //    isOpen = false;
        failed = true;
    } catch (...) {
        
        std::cerr << "sendBytes : unspecified error " << endl;
    }
    if(numBytesSent!=length) {
        //do something!
        std::cerr << "send fail, fewer bytes sent than expected : "<< numBytesSent << endl;
        failed = true;
    } else if (numBytesSent<0) {
        //do something!
        std::cerr << "send fail, sendBytes returned : "<< numBytesSent << endl;
        failed = true;
    }
    
    if(failed) {
        if(networkerror) {
            networkConnected = false;
            closeWhileRunning();
            setup(id, ipAddress, laserDockNetData);
        }
        
        
        return false;
    }
    return true;
}
void DacLaserDockNet :: close() {
    //if(!networkConnected) return;
    //    if(isThreadRunning()) {
    //        if(connected) {
    //            if(lock()) {
    //
    //            }
    //        }
    //        // also stops the thread :
    //        waitForThread(true, 1000); // 1 second time out
    //    }
    ofLogNotice("DacLaserDockNet :: close()"); 
    waitForThread(true, 1000);
    while(!lock());
    dataUdpSocket.close();
    commandUdpSocket.close();
    unlock();
    
}

int DacLaserDockNet::getDacTotalPointBufferCapacity() {
//    int returnvalue = 0;
//    if(lock()) {
//        returnvalue = pointBufferCapacity;
//        unlock();
//    }
    return pointBufferCapacity;
}

bool DacLaserDockNet::setPointsPerSecond(uint32_t newpps){
    //logNotice"setPointsPerSecond " + ofToString(newpps));
    if(newpps>maxPointRate) newpps = maxPointRate;
    if(!isThreadRunning()){
        pps = newPPS = newpps;
        return true;
    } else {
        while(!lock());
        newPPS = newpps;
        unlock();
        return true;
    }
}

const vector<ofAbstractParameter*>& DacLaserDockNet :: getDisplayData() {
    
    return displayData;
}


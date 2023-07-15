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
    // I've set it to the number of points in the
    // original LaserDockNet, but this is passed in when
    // the LaserDockNet is initialised
    pointBufferCapacity = 1799;

    lastCommandSendTime = 0;
    lastAckTime = 0;
    lastDataSentTime = 0;
    
    colourShiftImplemented = true;
    verbose = false;
}



DacLaserDockNet :: ~DacLaserDockNet(){
	
    // sends stop command, stops the thread, closes the socket
    close();
    
    
    cleanUpFramesAndPoints();
    
}


void DacLaserDockNet :: setup(string _id, string _ip, DacLaserDockNetStatus status) {
	
	pps = status.point_rate;
	newPPS = 30000; // this is always sent on begin if different
    maxPointRate = status.point_rate_max;
	
	connected = false;
	ipAddress = _ip;
    id = _id;
    laserDockNetData = status;
    lastAckTime = ofGetElapsedTimeMicros();

    versionString = status.firmware_version;
    

    int commandPort = DacLaserDockNetConsts::CMD_PORT;
   
    // update buffer from status
    pointBufferCapacity = status.buffer_max;
	
	try {

		Poco::Net::SocketAddress sa(ipAddress, DacLaserDockNetConsts::DATA_PORT);
        dataUdpSocket.connect(sa);
		
		connected = true;
	} catch (Poco::Exception& exc) {
		//Handle your network errors.
		ofLog(OF_LOG_ERROR,  "DacLaserDockNet setup failed - Network error: " +ipAddress+" "+ exc.displayText());
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
    
    try {

        Poco::Net::SocketAddress sa(ipAddress, DacLaserDockNetConsts::CMD_PORT);
        commandUdpSocket.connect(sa);
        
        connected &= true;
        
        
    } catch (Poco::Exception& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "DacLaserDockNet setup failed - Network error: " +ipAddress+" "+ exc.displayText());
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
        
	if(connected) {
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
        if(connected && (newPPS!=pps)) {
            sendPointRate(newPPS); // assume it was sent i guess? Or periodically send it?
            pps = newPPS;
        }
        
        // maxPointsToFillBuffer is the minimum number of points we want
        // in the buffer before we send more points.
        // If it's set to zero we would wait a long time before sending
        // points, and potentially risk an underrun.
        //
        // if it's set to the pointBufferCapacity, we'll keep sending
        // points as much as possible, as soon as the point count drops
        // even just one below that level we'll fill up the buffer again
        //
        // But we also use the latency value as we don't want to
        // fill the buffer right up if the time it would take to
        // process those points would be greater than the latency value
        int maxPointsToFillBuffer = MIN(pointBufferCapacity-minPacketDataSize, maxLatencyMS * pps /1000);
        
        
        if(isThreadRunning()) {
    
            waitUntilReadyToSend(maxPointsToFillBuffer);
            
            //check buffer and send the next points
            // while(!lock()) {}
            bool dataSent = sendPointsToDac();
            //unlock();
            if(dataSent) {
                checkDataPortIncoming();
               //connected = false;
        
                
            } else {
                // probably because there are no points in the buffer.
                //cout << "DATA NOT SENT " << endl;
            }
        }
        checkDataPortIncoming();
        yield();
    }
    
    
}



void DacLaserDockNet :: reset() {
	if(lock()) {
		resetFlag = true;
		unlock();
	}
	
}

void DacLaserDockNet :: closeWhileRunning() {
	if(!connected) return;
	
	waitForThread();
	
	while(!lock())
        ;
    dataUdpSocket.close();
	unlock();
	
	
}




inline bool DacLaserDockNet :: sendPointsToDac(){
    
    
    // get current buffer - note safer to be size by time acked
    // buffer fullness by time acked will be higher than actual buffer
    int maxEstimatedBufferFullness =  calculateBufferFullnessByTimeSent();//
    
    // total buffered points
    int numPointsWaitingToBeSent =  bufferedPoints.size();
    
    // get min buffer size dependent on the latency
    int minBufferToFill = maxLatencyMS * pps / 1000;

    // if the minimum is more than the maximum buffer size, then reduce it to fit
    if(minBufferToFill>getMaxPointBufferSize()) {
        minBufferToFill = getMaxPointBufferSize();
    }
    
    // the minimum number of points to queue
     
    int totalNumPointsToSend = 0;
    
    int minExpectedSpaceInBuffer = MAX(0, pointBufferCapacity - maxEstimatedBufferFullness );
   
    
    if(frameMode) {
        
        // calculate the minimum number of extra points to add to the
        // point queue in order to fill the buffer to the necessary level
        int minPointsToQueue = MAX(0, minBufferToFill - maxEstimatedBufferFullness - numPointsWaitingToBeSent);
        
       
        
       // add points from the frame queue to the bufferedPoints up to that minimum level
        updateFrameQueue(minPointsToQueue);

        totalNumPointsToSend = MIN(bufferedPoints.size(), minExpectedSpaceInBuffer);
       //if(totalNumPointsToSend<2000) totalNumPointsToSend = 0;
        
        if(totalNumPointsToSend==0) {
            if(verbose) logNotice("sendData : no points to send");
            return false;
        }
       // if(verbose) cout << maxEstimatedBufferFullness << " " << currentDacBufferFullnessMin << " " << numpointstosend << endl;
    } else {
        // for non-frame mode, just send the buffer
       totalNumPointsToSend = MIN(bufferedPoints.size(), minExpectedSpaceInBuffer);
    }

    bool success  = true;
    
    
    dacCommand.setDataCommand(packetNumber++); // packetNumber is a uint8_t so hopefully should overflow itself
    
    // it's just a point object to use, nothing special
	LaserDockNetDacPoint& dacPoint = sendPoint;
    
    // the colour shift delay in point count
    int colourShiftInPoints =  (float)pps/10000.0f*colourShift ;
    
    // need to send in groups of 140!
    // Note that at top speed this needs to be 1 packet every 4ms
    int numPointsLeftToSend = totalNumPointsToSend;
   
    if(verbose)  {
        ofLogNotice("maxEstimatedBufferFullness : ") <<maxEstimatedBufferFullness << " " << maxEstimatedBufferFullness+totalNumPointsToSend;
        ofLogNotice("Sending ") << totalNumPointsToSend << " points... ";
    }
    if(verbose) cout << "Packets : ";
    
    int packetcount = 0;
    
    while(numPointsLeftToSend>0) {
        //cout << numPointsLeftToSend << endl;
        int pointsInPacket = 0;

            
        for(int i = 0; (i<numPointsLeftToSend) && (i<140); i++) {
            
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
            pointsInPacket ++;
          
                
            
        }
        
        if(!sendData(dacCommand)) {
            success = false;
        }
        
        // check for ack
        checkDataPortIncoming();
       
        numPointsLeftToSend-=pointsInPacket;
        if(numPointsLeftToSend>0) {
            // reset command for next time around
            dacCommand.setDataCommand(packetNumber++);
        }
        
        packetcount++;
        if(verbose) cout << packetcount << "..";
        // I'm told this is necessary but it never seems to send more than 25 in one go
        if(packetcount ==25) {
            sleep(10);
           
            packetcount = 0;
        }

    }
    
    if(verbose) cout << endl;
    
    if(success) {
        lastDataSentTime = ofGetElapsedTimeMicros();
        lastDataSentBufferSize = maxEstimatedBufferFullness + totalNumPointsToSend;
    }  else {
        logNotice("sendCommand failed!");
        
    }
    
    
    return success;
	
}
string DacLaserDockNet :: getId(){
    //return "Ether Dream "+versionString+ " " +id;
	return "LaserDockNet "+id;
  
}

// TODO could this be a conflict?
int DacLaserDockNet :: getStatus(){
    return 0;
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
    dacCommand.setPointRateCommand(rate);
	return sendCommand(dacCommand);
}

void DacLaserDockNet :: logData() {
    dacCommand.logData();
    
}
bool DacLaserDockNet :: sendData(DacLaserDockNetCommand& command) { // sendBytes(const uint8_t* buffer, int length) {
	
    const uint8_t* buffer = command.getBuffer();
    int length = command.size();
    
	int numBytesSent = 0;
	bool failed = false;
	bool networkerror = false;
    lastCommandSendTime = ofGetElapsedTimeMicros();//  count = 0;

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
	catch (Poco::Exception& exc) {
		//Handle your network errors.
		cerr << "sendBytes : Network error: " << exc.displayText() << endl;
		networkerror = true;
		failed = true;
    
    }
	catch (Poco::TimeoutException& exc) {
		//Handle your network errors.
		cerr << "sendBytes : Timeout error: " << exc.displayText() << endl;
		//	isOpen = false;
		failed = true;
    } catch (...) {
        
        cerr << "sendBytes : unspecified error " << endl;
    }
	if(numBytesSent!=length) {
		//do something!
		cerr << "send fail, fewer bytes sent than expected : "<< numBytesSent << endl;
		failed = true;
	} else if (numBytesSent<0) {
		//do something!
		cerr << "send fail, sendBytes returned : "<< numBytesSent << endl;
		failed = true;
	}
	
	if(failed) {
		if(networkerror) {
            connected = false; 
			closeWhileRunning();
			setup(id, ipAddress, laserDockNetData);
		}

        
		return false;
	}
	return true;
}

bool DacLaserDockNet :: checkDataPortIncoming() {
   // return true;
    int n = 0;
    bool failed = false;
    try {
        n = dataUdpSocket.receiveBytes(inBuffer, 4);
        
        
    } catch (Poco::Exception& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "Network error: " + exc.displayText());
        //    isOpen = false;
        failed = true;
    } catch (Poco::TimeoutException& exc) {
        //Handle your network errors.
        ofLog(OF_LOG_ERROR,  "Timeout error: " + exc.displayText());
        //    isOpen = false;
        failed = true;
        
    }
    if(n==4) {
        if(inBuffer[0] == DacLaserDockNetConsts::CMD_GET_RINGBUFFER_EMPTY_SAMPLE_COUNT) {
            
            //cout << calculateBufferFullnessByTimeAcked() << " " ;
            lastAckTime = ofGetElapsedTimeMicros();
            lastReportedBufferFullness = getMaxPointBufferSize() - ByteStreamUtils::bytesToUInt16(&inBuffer[2]);
            
           // cout << lastReportedBufferFullness << endl;
        }
        
    }
    return failed;
    
}
bool DacLaserDockNet :: sendCommand(DacLaserDockNetCommand& command) { // sendBytes(const uint8_t* buffer, int length) {
    
    const uint8_t* buffer = command.getBuffer();
    int length = command.size();
    
    int numBytesSent = 0;
    bool failed = false;
    bool networkerror = false;
    lastCommandSendTime = ofGetElapsedTimeMicros();//  count = 0;

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
        cerr << "sendBytes : Network error: " << exc.displayText() << endl;
        networkerror = true;
        failed = true;
    
    }
    catch (Poco::TimeoutException& exc) {
        //Handle your network errors.
        cerr << "sendBytes : Timeout error: " << exc.displayText() << endl;
        //    isOpen = false;
        failed = true;
    } catch (...) {
        
        cerr << "sendBytes : unspecified error " << endl;
    }
    if(numBytesSent!=length) {
        //do something!
        cerr << "send fail, fewer bytes sent than expected : "<< numBytesSent << endl;
        failed = true;
    } else if (numBytesSent<0) {
        //do something!
        cerr << "send fail, sendBytes returned : "<< numBytesSent << endl;
        failed = true;
    }
    
    if(failed) {
        if(networkerror) {
            connected = false;
            closeWhileRunning();
            setup(id, ipAddress, laserDockNetData);
        }

        
        return false;
    }
    return true;
}
void DacLaserDockNet :: close() {
    
    if(isThreadRunning()) {
        if(connected) {
            if(lock()) {
              
            }
        }
        // also stops the thread :
        waitForThread(true, 1000); // 1 second time out
    }
        
    dataUdpSocket.close();

}

int DacLaserDockNet::getMaxPointBufferSize() {
    int returnvalue = 0;
    if(lock()) {
        returnvalue = pointBufferCapacity;
        unlock();
    }
    return returnvalue;
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


//
//  ofxLaserDacLaserDockNetCommand.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//

#pragma once
#include "ByteBuffer.h"
#include "ofxLaserDacLaserDockNetDacPoint.h"
#include "ofxLaserDacLaserDockNetConsts.h"

namespace ofxLaser {
class DacLaserDockNetCommand : public ByteBuffer {
 
    public :
    
    void clear() override ;
    void setCommand(char command);
    void setDataCommand (uint8_t messagenum) ;
    void setPointRateCommand (uint32_t newrate); 
    void addPoint(LaserDockNetDacPoint& p);
 
    void logData();
    
    int numPoints = 0;
    
  //  const uint8_t CMD_SAMPLE_DATA = 0xa9;
    
    
} ;

}

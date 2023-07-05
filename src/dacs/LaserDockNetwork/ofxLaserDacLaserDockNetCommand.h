//
//  ofxLaserDacLaserDockNetCommand.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/07/2023.
//

#pragma once
#include "ByteBuffer.h"
#include "ofxLaserDacLaserDockNetDacPoint.h"

namespace ofxLaser {
class DacLaserDockNetCommand : public ByteBuffer {
 
    public :
    
    void clear() override ;
    void setCommand(char command);
    void setDataCommand (uint16_t numpoints) ;
    void addPoint(LaserDockNetDacPoint& p);
    void setBeginCommand(uint32_t pointRate);
    void setPointRateCommand(uint32_t pointRate);
    void logData();
    
    int numPointsExpected = 0;
    int numPoints = 0;
    
} ;

}

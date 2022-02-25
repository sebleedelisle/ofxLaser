//
//  ofxLaserDacEtherDreamCommand.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/02/2022.
//

#pragma once
#include "ByteBuffer.h"
#include "ofxLaserDacEtherDreamDacPoint.h"

namespace ofxLaser {
class DacEtherDreamCommand : public ByteBuffer {
 
    public :
    
    void clear() override ;
    void setCommand(char command);
    void setDataCommand (uint16_t numpoints) ;
    void addPoint(EtherDreamDacPoint& p);
    void setBeginCommand(uint32_t pointRate);
    void setPointRateCommand(uint32_t pointRate);
    void logData();
    
    int numPointsExpected = 0;
    int numPoints = 0;
    
} ;

}

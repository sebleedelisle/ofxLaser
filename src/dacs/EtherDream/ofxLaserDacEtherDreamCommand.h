//
//  ofxLaserDacEtherDreamCommand.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/02/2022.
//

#pragma once
#include "ByteBuffer.h"
#include "ofxLaserDacEtherDreamDacPoint.h"
#include <format>
#include <iostream>
#include <string>
#include <string_view>

namespace ofxLaser {
class DacEtherDreamCommand : public ByteBuffer {
 
    public :
    
    void clear() override ;
    void setCommand(char command);
    void setAsDataCommand (uint16_t numpoints) ;
    void addPoint(EtherDreamDacPoint& p);
    void setAsBeginCommand(uint32_t pointRate);
    void setAsPointRateCommand(uint32_t pointRate);
    //void logData();
    void getDataLog(std::stringstream& str);
    int numPointsExpected = 0;
    int numPoints = 0;
    
} ;

}

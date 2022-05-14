//
//  ofxLaserDacEtherDreamCommand.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 05/02/2022.
//

#pragma once
#include "ByteBuffer.h"
#include "LaserdockDevice.h"

namespace ofxLaser {
class DacLaserdockByteStream : public ByteBuffer {
 
    public :
    
    void clear() override ;
    void addPoint(LaserdockSample& p);
     void logData();
    
    int numPointsExpected = 0;
    int numPoints = 0;
    
} ;

}

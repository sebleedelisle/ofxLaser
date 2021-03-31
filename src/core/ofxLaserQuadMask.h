//
//  ofxLaserQuadMask.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 01/02/2018.
//

#include "ofxLaserQuadGui.h"



namespace ofxLaser {
class QuadMask : public QuadGui{
    
    public :
    QuadMask() : QuadGui() {};


    float maskLevel = 1; 

    
};
}

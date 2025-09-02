//
//  ofxLaserIconSVGs.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 11/02/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxSvgExtra.h"

namespace ofxLaser {
class IconSVGs {
    
    public :
    IconSVGs();
    void initSVGs(); 
    
    ofxSVGExtra iconGrabOpen;
    ofxSVGExtra iconGrabClosed;
    ofxSVGExtra iconMagPlus;
    ofxSVGExtra iconMagMinus;
    vector<ofxSVGExtra> numberSVGs;

    
};
}




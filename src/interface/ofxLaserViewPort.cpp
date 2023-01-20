//
//  ofxLaserViewPort.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 13/01/2023.
//
//

#include "ofxLaserViewPort.h"

using namespace ofxLaser;

void ViewPort::zoom(glm::vec2 anchor, float zoomMultiplier){
    
    glm::vec2 clickoffset = anchor-offset;
    clickoffset-=(clickoffset*zoomMultiplier);
    offset+=clickoffset;
    offset*=zoomMultiplier;

}

void ViewPort::setOffsetAndScale(glm::vec2 newoffset, float newscale){
    offset = newoffset;
    scale = newscale;
}

//
//  Particle.h
//  example_Empty
//
//  Created by Seb Lee-Delisle on 03/12/2015.
//
//
#pragma once 
#include "ofMain.h"

class Particle {
    
    public :
    Particle(){};
    
    ofVec2f pos;
    ofVec2f vel;
    float size;
};
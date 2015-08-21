//
//  ofxIldaPoint.h
//  ofxIlda
//
//  Created by Memo Akten on 09/05/2013.
//
//

#pragma once

namespace ofxIlda {
    
#define kIldaMinPoint -32768
#define kIldaMaxPoint 32767
#define kIldaDimension (kIldaMaxPoint - kIldaMinPoint)
#define kIldaMaxIntensity 65535
    
    class Point {
    public:
        Point() {}
        Point(int16_t x, int16_t y, int16_t r=0, int16_t g=0, int16_t b=0, int16_t a=0): x(x), y(y), r(r), g(g), b(b), a(a) {}
        Point(ofPoint p, ofFloatColor c, ofPoint pmin = ofPoint::zero(), ofPoint pmax = ofPoint::one()) { set(p, c, pmin, pmax); }
        Point(ofPoint p, ofPoint pmin = ofPoint::zero(), ofPoint pmax = ofPoint::one()) { setPosition(p, pmin, pmax); }
        
        int16_t x = 0;
        int16_t y = 0;
        uint16_t r = 0;
        uint16_t g = 0;
        uint16_t b = 0;
        uint16_t a = 0;
        uint16_t u1 = 0;  // what are these for? standard ILDA or just etherdream?
        uint16_t u2 = 0;
        
        //--------------------------------------------------------------
        void set(int16_t x, int16_t y) {
            this->x = x;
            this->y = y;
        }
        
        //--------------------------------------------------------------
        void set(int16_t x, int16_t y, int16_t r, int16_t g, int16_t b, int16_t a) {
            this->x = x;
            this->y = y;
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }
        
        
        //--------------------------------------------------------------
        // set color and position mapped from custom range (defaults to normalized)
        void set(ofPoint p, ofFloatColor c, ofPoint pmin = ofPoint::zero(), ofPoint pmax = ofPoint::one()) {
            set(
                ofMap(p.x, pmin.x, pmax.x, kIldaMinPoint, kIldaMaxPoint, true),
                ofMap(p.y, pmin.y, pmax.y, kIldaMinPoint, kIldaMaxPoint, true),
                c.r * kIldaMaxIntensity,
                c.g * kIldaMaxIntensity,
                c.b * kIldaMaxIntensity,
                c.a * kIldaMaxIntensity
                );
        }
        
        //--------------------------------------------------------------
        // set position mapped from custom range (defaults to normalized)
        void setPosition(ofPoint p, ofPoint pmin = ofPoint::zero(), ofPoint pmax = ofPoint::one()) {
            set(
                ofMap(p.x, pmin.x, pmax.x, kIldaMinPoint, kIldaMaxPoint, true),
                ofMap(p.y, pmin.y, pmax.y, kIldaMinPoint, kIldaMaxPoint, true)
                );
        }
        
        
        //--------------------------------------------------------------
        // gets position of point mapped to desired range (defaults to normalized)
        ofPoint getPosition(ofPoint pmin = ofPoint::zero(), ofPoint pmax = ofPoint::one()) {
            return ofPoint(
                           ofMap(x, kIldaMinPoint, kIldaMaxPoint, pmin.x, pmax.x),
                           ofMap(y, kIldaMinPoint, kIldaMaxPoint, pmin.y, pmax.y)
                           );
        }
        
    };
}
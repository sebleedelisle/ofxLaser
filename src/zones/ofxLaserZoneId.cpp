//
//  ofxLaserZoneId.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 25/02/2023.
//
//

#include "ofxLaserZoneId.h"

using namespace ofxLaser; 

//bool operator== (const ZoneId& c1, const ZoneId& c2)
//{
//    return c1.getUid()==c2.getUid();
//}
//
//bool operator!= (const ZoneId& c1, const ZoneId& c2)
//{
//    return c1.getUid()!=c2.getUid();
//}
//

bool ZoneId::operator==(const ZoneId & other) const{
    return other.getUid()==getUid();
}

//--------------------------------------------------------------
bool ZoneId::operator!=(const ZoneId & other) const{
    return other.getUid()!=getUid();
}

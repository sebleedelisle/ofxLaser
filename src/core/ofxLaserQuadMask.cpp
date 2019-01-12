//
//  ofxLaserQuadMask.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 01/02/2018.
//

#include "ofxLaserQuadMask.h"

using namespace ofxLaser;

bool QuadMask::loadSettings() {
    
    string filename = saveLabel+".xml";
    ofxXmlSettings xml;
    if(!xml.loadFile(filename)) {
        ofLog(OF_LOG_ERROR, "QuadMask::loadSettings - file not found : "+filename);
        return false;
    }
    
    //cout << "Warp Pre load: " << filename << " " << dstPoints[0].x << ", " << dstPoints[0].y << endl;
    
    handles[0].x    = xml.getValue("quad:upperLeft:x", 0.0);
    handles[0].y    = xml.getValue("quad:upperLeft:y", 0.0);
    
    handles[1].x    = xml.getValue("quad:upperRight:x", 1.0);
    handles[1].y    = xml.getValue("quad:upperRight:y", 0.0);
    
    handles[2].x    = xml.getValue("quad:lowerLeft:x", 0.0);
    handles[2].y    = xml.getValue("quad:lowerLeft:y", 1.0);
    
    handles[3].x    = xml.getValue("quad:lowerRight:x", 1.0);
    handles[3].y    = xml.getValue("quad:lowerRight:y", 1.0);
    
    updateCentreHandle();
    maskLevel = xml.getValue("quad:maskLevel:level", 1.0);
    return true;
}

void QuadMask::saveSettings() {
    
    ofxXmlSettings xml;
    
    string filename = saveLabel+".xml";
    
    xml.addTag("quad");
    xml.pushTag("quad");
    
    xml.addTag("upperLeft");
    xml.pushTag("upperLeft");
    xml.addValue("x", handles[0].x);
    xml.addValue("y", handles[0].y);
    xml.popTag();
    
    xml.addTag("upperRight");
    xml.pushTag("upperRight");
    xml.addValue("x", handles[1].x);
    xml.addValue("y", handles[1].y);
    xml.popTag();
    
    xml.addTag("lowerLeft");
    xml.pushTag("lowerLeft");
    xml.addValue("x", handles[2].x);
    xml.addValue("y", handles[2].y);
    xml.popTag();
    
    xml.addTag("lowerRight");
    xml.pushTag("lowerRight");
    xml.addValue("x", handles[3].x);
    xml.addValue("y", handles[3].y);
    xml.popTag();
    
    xml.addTag("maskLevel");
    xml.pushTag("maskLevel");
    xml.addValue("level", maskLevel);
    xml.popTag();

    xml.saveFile(filename);
    
    
}

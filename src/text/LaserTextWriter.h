//
//  LaserTextWriter.h
//
//  Created by Seb Lee-Delisle on 16/04/2022.
//

#pragma once
#include "TextWriter.h"
#include "ofxLaserFactory.h"



class LaserTextWriter : public TextWriter {
    
    public :
    vector<ofPolyline*> getGlyphPolylines(Letter &letter, ofRectangle box );
    vector<vector<ofPolyline*>> getPolylines(vector<string>& lines, ofVec3f pos, float size, bool centred);
    vector<vector<ofPolyline*>> getPolylines(string line, ofVec3f pos, float size, bool centred);

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
};

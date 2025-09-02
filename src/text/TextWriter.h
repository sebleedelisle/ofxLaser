//
//  TextWriter.h
//  PixelPyros
//
//  Created by Paul King on 27/08/2012.
//

#pragma once 

#include <map>
#include "ofMain.h"
#include "VectorFont.h"

class TextWriter {
    
public:
    
    TextWriter();
    void draw(ofRectangle, string, bool centered);
    void drawFixedSize(ofRectangle, string, float, bool);
    ofMesh getMesh(vector<string>& lines, ofVec3f pos, float size, bool centred);
    ofMesh getMesh(string line, ofVec3f pos, float size, bool centred);
    
//    vector<ofPolyline> getPolylines(vector<string>& lines, ofVec3f pos, float size, bool centred);
//    vector<ofPolyline> getPolylines(string line, ofVec3f pos, float size, bool centred);
//    
    float glyphSpacing, glyphLineSpacing, glyphLineWeight;
	ofColor colour;
	
	float lineRandomness;
	float colourFlickerMin;
	bool smooth; 
    
	void addGlyphToMesh(Letter&, ofRectangle, ofMesh& );
	
	
	float glyphWidth, glyphHeight;
	
protected:
    
    int findBestBreak(string, int, int);
    float calculateBlockWidth(string, float, float);
    void drawGlyph(Letter&, ofRectangle);
    void renderGlyph(Letter&, float, float);

    float glyphRatio;
    VectorFont font;
};

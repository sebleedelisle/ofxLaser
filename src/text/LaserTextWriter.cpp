//
//  LaserTextWriter.cpp
//
//  Created by Seb Lee-Delisle on 16/04/2022.
//

#include "LaserTextWriter.h"




vector<ofPolyline*> LaserTextWriter::getGlyphPolylines(Letter &letter, ofRectangle box ) {
    
    
    vector<ofPolyline*> polylines;
    ofPolyline* poly = ofxLaser::Factory::getPolyline();
    polylines.push_back(poly);
    
    ofVec3f lastpoint(-10000,100000); // make sure it's no where near
    
    for( int i = 0; i < letter.points.size(); i+=2 ) {
        
        ofVec3f v1, v2;
        v1 = letter.points[i];
        v2 = letter.points[i+1];
        
        if(v1.squareDistance(lastpoint)>0.1) {
            // if were not connected, make a new polyline
            poly = ofxLaser::Factory::getPolyline();
            polylines.push_back(poly);
            poly->addVertex(box.x + (v1.x/glyphWidth*box.width), box.y + (v1.y/glyphHeight*box.height));
            
        }
        poly->addVertex(box.x + (v2.x/glyphWidth*box.width), box.y + (v2.y/glyphHeight*box.height));
        lastpoint = v2;

    }
    return polylines;
    
}

vector<vector<ofPolyline*>> LaserTextWriter :: getPolylines(string line, ofVec3f pos, float size, bool centred) {
    vector<string> lines;
    lines.push_back(line);
    return getPolylines(lines, pos, size, centred);
}

vector<vector<ofPolyline*>> LaserTextWriter::getPolylines(vector<string>& lines, ofVec3f pos, float size, bool centred) {
    

    float glyphRenderWidth = glyphWidth * size;
    float glyphRenderHeight = glyphHeight * size;
    float glyphRenderSpacing = glyphSpacing * size;

    map <string, Letter>& letters = font.letters;

    vector<vector<ofPolyline*>> polylines;
    
    float marginTop = 0;

    float ofsX = 0, ofsY = marginTop;
    for( int j = 0; j < lines.size(); j++ ) {
        string line = lines[j];
        float glyphMarginLeft = centred ? (( - calculateBlockWidth(line, glyphRenderWidth, glyphRenderSpacing)) / 2.0) : 0;
        for( int i = 0; i < line.length(); i++ ) {
            string nextChar = line;
            // this hacky way of trimming the string retains
            // special characters
            nextChar.erase(0,i);
            unsigned char c = nextChar[0];
            if ((c & 0xE0) == 0xC0) {
                //specialCharacter = true;
                if(i<line.size()-2) nextChar.erase(2);
                i++;
                
            } else {
                //specialCharacter = false;
                if(i<line.size()-1) nextChar.erase(1);
            }
            //char letterchar = nextChar[0];
            //Letter& letter= letters[letterchar];
            if ( letters.find(nextChar) != letters.end() ) {
                polylines.push_back(getGlyphPolylines(letters[nextChar], ofRectangle(pos.x + ofsX + glyphMarginLeft, pos.y + ofsY, glyphRenderWidth, glyphRenderHeight)));
            } else {
                if((nextChar!=" ") && (nextChar!="\n")&& (nextChar!="\r")) {
                    ofLog() << "CHARACTER NOT FOUND : '" << nextChar << "' " << (int)nextChar[0] << " " << line;
                }
            }
//            addGlyphToMesh(letters[line[i]], ofRectangle(pos.x + ofsX + glyphMarginLeft, pos.y + ofsY, glyphRenderWidth, glyphRenderHeight), writingMesh);
            
            
            
            ofsX += glyphRenderWidth + glyphRenderSpacing;
        }
        ofsX = 0;
        ofsY += glyphRenderHeight + glyphRenderSpacing;
    }
    return polylines;
//    return writingMesh;
}

//
//  TextWriter.cpp
//  PixelPyros
//
//  Created by Paul King on 27/08/2012.
//

#include "TextWriter.h"

// From http://www.cplusplus.com/faq/sequences/strings/trim/
inline std::string trim(const std::string &s) {
    return s.substr(0, s.find_last_not_of(" ") + 1);
}

TextWriter::TextWriter() {
    glyphWidth = 4;
    glyphHeight = 10;
    glyphSpacing = 1.5;
    glyphLineWeight = 2;
	colour.set(20,200,20);
	lineRandomness = 0;
	colourFlickerMin = 1;
	smooth = true; 
    
    glyphRatio = glyphHeight / (float)glyphWidth;
}

int TextWriter::findBestBreak(string s, int start, int radius) {
    int bestIndex = start;
    float bestWeight = 0, examWeight;
    
    radius = MIN(MIN(start + radius, s.length() - 1) - start, start - MAX(start - radius, 0));
    
    for( int i = start - radius; i <= start + radius; i++ ) {
        if( s[i] != ' ' ) {
            continue;
        }
        
        examWeight = 10 / (float)abs(start - i);
        if( examWeight > bestWeight ) {
            bestIndex = i;
            bestWeight = examWeight;
        }
    }
    
    return MIN(bestIndex + 1, s.length());
}

float TextWriter::calculateBlockWidth(string s, float glyphRenderWidth, float glyphRenderSpacing) {
    float len = s.length();
    
    return (len * glyphRenderWidth) + (glyphRenderSpacing * (len - 1));
}

void TextWriter::addGlyphToMesh(Letter &letter, ofRectangle box, ofMesh& mesh) {
	
	//float randomness = 0.05;
	
    for( int i = 0; i < letter.points.size(); i++ ) {
		
		ofVec3f v; 
		v.x = box.x + ofMap(letter.points[i].x + ofRandom(-lineRandomness, lineRandomness), 0, glyphWidth, 0, box.width);
		v.y = box.y + ofMap(letter.points[i].y + ofRandom(-lineRandomness, lineRandomness), 0, glyphHeight, 0, box.height);
       
        mesh.addVertex(v);
		
		ofColor vcolor(colour);
		vcolor.setBrightness(vcolor.getBrightness() * ofRandom(colourFlickerMin, 1));
		mesh.addColor(vcolor); 
       
    }

}




void TextWriter::drawFixedSize(ofRectangle box, string text, float glyphScaleFactor, bool centred) {
    text = ofToUpper(text);
    
	if(box.height<=0) box.height = 1;
	if(box.width<=0) box.width = 1;
    
    ofPushStyle();

    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetLineWidth(glyphLineWeight);
    
    
    vector<string> lines;
    
    if( text.find('\n') == string::npos)  {
        lines.push_back(text);
    } else {
        lines = ofSplitString(text, "\n");
    }
    
	ofMesh writingMesh = getMesh(lines, box.getTopLeft(), glyphScaleFactor, centred);
	
	writingMesh.setMode(OF_PRIMITIVE_LINES);
    writingMesh.draw(); 
    
    ofPopStyle();
}


void TextWriter::draw(ofRectangle box, string text, bool centred) {
    text = ofToUpper(text);
    
	// added these lines otherwise you get horribleness! 
	if(box.height<=0) box.height = 1;
	if(box.width<=0) box.width = 1;
	
	
    ofPushStyle();

    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetLineWidth(glyphLineWeight);
    
    ofPushStyle();
    ofSetColor(10, 10, 255, 128);
    ofNoFill();
	
    ofPopStyle();
    
    float boxRatio = box.height / (float)box.width;
    int textLength = text.length();
    string longestLine = "";
    vector<string> lines;
    
    int numRows = (int)sqrt(boxRatio * textLength / glyphRatio);
    if( (text.find(' ') == string::npos) || (numRows == 0) ) {
        longestLine = trim(text);
        lines.push_back(longestLine);
    } else {
        int numCols = textLength / numRows;
        int start = numCols;
        int index = 0;
        int last;
        
        for( int row = 0; row < numRows - 1; row++ ) {
            last = index;
            index = findBestBreak(text, start, numCols * 2);
            
            string linePart = trim(text.substr(last, index - last));
            
            if( linePart.length() > longestLine.length() ) {
                longestLine = linePart;
            }
            
            lines.push_back(linePart);
            
            start = index + numCols;
        }
        
          string linePart = trim(text.substr(index));
        lines.push_back(linePart);
        if( linePart.length() > longestLine.length() ) {
            longestLine = linePart;
        }
        
          }
    
    float glyphScaleFactor = (box.width / (float)longestLine.length()) / (float)(glyphWidth + glyphSpacing);
    float glyphRenderWidth = glyphWidth * glyphScaleFactor;
    float glyphRenderHeight = glyphHeight * glyphScaleFactor;
    float glyphRenderSpacing = glyphSpacing * glyphScaleFactor;
    
    float marginTop = ((box.height - ((glyphRenderHeight + glyphRenderSpacing) * lines.size())) / 2.0);
    float marginLeft = 0; //(int)((box.width - calculateBlockWidth(longestLine, glyphRenderWidth, glyphRenderSpacing)) / 2.0); 
    
    map <string, Letter>& letters = font.letters;
    
	ofMesh writingMesh; 
	
    float ofsX = marginLeft, ofsY = marginTop;
    for( int j = 0; j < lines.size(); j++ ) {
        string line = lines[j];
        float glyphMarginLeft = centred ? ((box.width - calculateBlockWidth(line, glyphRenderWidth, glyphRenderSpacing)) / 2.0) : 0;
        for( int i = 0; i < line.length(); i++ ) {
			addGlyphToMesh(letters[line.substr(i,1)], ofRectangle(box.x + ofsX + glyphMarginLeft, box.y + ofsY, glyphRenderWidth, glyphRenderHeight), writingMesh);
            ofsX += glyphRenderWidth + glyphRenderSpacing;
        }
        ofsX = marginLeft;
        ofsY += glyphRenderHeight + glyphRenderSpacing;
    }
	
	writingMesh.setMode(OF_PRIMITIVE_LINES);
    writingMesh.draw();
    writingMesh.setMode(OF_PRIMITIVE_POINTS);
	writingMesh.draw();
    

	
    ofPopStyle();
	
	//return writingMesh;
}

ofMesh TextWriter::getMesh(string line, ofVec3f pos, float size, bool centred) {
	vector<string> lines;
	lines.push_back(line);
	return getMesh(lines, pos, size, centred); 
	
}

ofMesh TextWriter::getMesh(vector<string>& lines, ofVec3f pos, float size, bool centred) {
	
	
	float glyphRenderWidth = glyphWidth * size;
    float glyphRenderHeight = glyphHeight * size;
    float glyphRenderSpacing = glyphSpacing * size;

	
	map <string, Letter>& letters = font.letters;
    
	ofMesh writingMesh;

	
    float marginTop = 0;
	
    float ofsX = 0, ofsY = marginTop;
    for( int j = 0; j < lines.size(); j++ ) {
        string line = lines[j];
        float glyphMarginLeft = centred ? (( - calculateBlockWidth(line, glyphRenderWidth, glyphRenderSpacing)) / 2.0) : 0;
        for( int i = 0; i < line.length(); i++ ) {
			addGlyphToMesh(letters[line.substr(i,1)], ofRectangle(pos.x + ofsX + glyphMarginLeft, pos.y + ofsY, glyphRenderWidth, glyphRenderHeight), writingMesh);
            ofsX += glyphRenderWidth + glyphRenderSpacing;
        }
        ofsX = 0;
        ofsY += glyphRenderHeight + glyphRenderSpacing;
    }
	
	return writingMesh;
}

//
//vector<ofPolyline>  TextWriter::getPolylines(string line, ofVec3f pos, float size, bool centred) {
//    vector<string> lines;
//    lines.push_back(line);
//    return getPolylines(lines, pos, size, centred);
//    
//}
//
//vector<ofPolyline> TextWriter::getPolylines(vector<string>& lines, ofVec3f pos, float size, bool centred) {
//    
//    vector<ofPolyline> polylines;
//    
//    ofMesh mesh = getMesh(lines, pos, size, centred);
//    auto& vertices = mesh.getVertices();
//   
//    for(int i = 0; i<vertices.size(); i+=2) {
//        if(i+1>=vertices.size()) break;
//        polylines.resize(polylines.size()+1);
//        ofPolyline& poly = polylines.back();
//        
//        poly.addVertex(vertices[i]);
//        poly.addVertex(vertices[i+1]) ;
//        
//    }
//    return polylines;
//}

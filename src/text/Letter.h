//
//  Letter.h
//  PixelPyros
//
//  Created by Seb Lee-Delisle on 21/08/2012.
//
//

#pragma once

class Letter {
    
public:
	
	void addLine(float x1, float y1, float x2, float y2) {
		points.push_back(ofVec2f(x1,y1));
		points.push_back(ofVec2f(x2,y2));
	};
    
	vector <ofVec2f> points;
};

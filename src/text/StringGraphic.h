//
//  LaserStringGraphic.h
//  LaserTrainTimes
//
//  Created by Seb Lee-Delisle on 07/06/2020.
//
//

#pragma once
#include "ofMain.h"
#include "ofxLaserGraphic.h"
#include "ofxHersheyFont.h"
#include "LaserTextWriter.h"

namespace ofxLaser {
class StringGraphic : public ofxLaser::Graphic{
	
	public:
	
	
	enum Alignment{
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_CENTRE
	};
	
	bool setString(string newString, const ofColor& colour);
	bool setString(string newString);
    void setAlignment(Alignment newalignment);
	
	void updateGraphic();
	
	float getWidth();
	
	ofPath textPath;
	string textString;
	ofColor colour;
	
    vector<vector<ofPolyline*>> polylineCharacters; 

	Alignment alignment = ALIGN_LEFT;
	
	
	ofxHersheyFont hersheyFont;
    LaserTextWriter textWriter; 
	
	protected:

	private:

};
}

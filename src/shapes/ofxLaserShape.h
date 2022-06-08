//
//  LaserShape.h
//  PixelPyros
//
//  Created by Seb Lee-Delisle on 25/09/2013.
//
//

#pragma once
#include "ofxLaserPoint.h"
#include "ofxLaserRenderProfile.h"

namespace ofxLaser {
class Shape {

	public :

	Shape(){};
	virtual ~Shape(){
		//cout << "LASER SHAPE DESTROY!!!" << endl;
	};
    virtual Shape* clone() const =0;
    
	virtual void addPreviewToMesh(ofMesh& mesh) =0;
	
    vector<float>& getPointsAlongDistance(float distance, float acceleration, float speed, float speedMultiplier);

	vector<float> unitDistances;

    virtual ofPoint& getStartPos();
    virtual ofPoint& getEndPos();
	
    virtual ofFloatColor& getColour();
    virtual void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier) ;
    virtual bool intersectsRect(ofRectangle & rect) = 0;
	
    void setTargetZone(int zonenumber);
	
    int getTargetZone();
    
    
    bool tested = false;
    bool reversed = false;
    bool reversable = false;
    
    
	string profileLabel;
	
	protected :
	ofPoint startPos;
	ofPoint endPos;
	ofFloatColor colour;
	int targetZoneNumber = 0;
	

};
}

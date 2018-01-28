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
	virtual void addPreviewToMesh(ofMesh& mesh){};
	
	vector<float>& getPointsAlongDistance(float distance, float acceleration, float speed){
		
		unitDistances.clear();
		
		float acceleratedistance = (speed*speed) / (2*acceleration);
		float timetogettospeed = speed / acceleration;
		
		float totaldistance = distance;
		
		float constantspeeddistance = totaldistance - (acceleratedistance*2);
		float constantspeedtime = constantspeeddistance/speed;
		
		if(totaldistance<(acceleratedistance*2)) {
			
			constantspeeddistance = 0 ;
			constantspeedtime = 0;
			acceleratedistance = totaldistance/2;
			speed = sqrt( acceleratedistance * 2 * acceleration);
			timetogettospeed = speed / acceleration;
			
		}
		
		float totaltime = (timetogettospeed*2) + constantspeedtime;
		
		float timeincrement = totaltime / (floor(totaltime));
		
		float currentdistance;
		
		float t = 0;
		
		while (t <= totaltime + 0.001) {
			
			if(t>totaltime) t = totaltime;
			
			if(t <=timetogettospeed) {
				currentdistance = 0.5 * acceleration * (t*t);
				
			} else if((t>timetogettospeed) && (t<=timetogettospeed+constantspeedtime)){
				currentdistance = acceleratedistance + ((t-timetogettospeed) * speed);
				
			} else  {
				float t3 = t - (timetogettospeed + constantspeedtime);
				
				currentdistance = (acceleratedistance + constantspeeddistance) + (speed*t3)+(0.5 *(-acceleration) * (t3*t3));
				
				
			}
			
			unitDistances.push_back(currentdistance/totaldistance);
			
			t+=timeincrement;
			
		}
		
		return unitDistances; 
		
	}

	vector<float> unitDistances;
	
	bool tested = false;
	bool reversed = false;
	bool reversable = false; 
	
	virtual ofPoint& getStartPos(){
		if(reversed && reversable) return endPos;
		else return startPos; 
	}
	virtual ofPoint& getEndPos(){
		if(reversed && reversable) return startPos;
		else return endPos;
	};
	
	virtual ofFloatColor& getColour() {
		return colour;
	}
	virtual void appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile) {
		
	};
	virtual bool intersectsRect(ofRectangle & rect) {
		
		
	};
	
	string profileLabel;
	
	protected :
	ofPoint startPos;
	ofPoint endPos;
	ofFloatColor colour;
	

};
}

//
//  ofxLaser.h
//
//  Created by Seb Lee-Delisle on 13/08/2015.
//

#pragma once

#include "ofxEtherdream.h"
#include "ofxGui.h"
#include "ofxLaserShape.h"
#include "ofxLaserPoint.h"
#include "ofxLaserDot.h"
#include "ofxLaserLine.h"
#include "ofxLaserCircle.h"
#include "ofxLaserPolyline.h"
#include "ofxLaserQuadWarp.h"



namespace ofxLaser {
	
	class Manager {
		
		public:
		
		Manager();
		
		void setup(int width, int height);
		void update();
		void draw();
		void drawShapes();
		void renderPreview();
		void renderLaserPath();
		
		void setupParameters();
		void roundPPS(int& v);
		//void roundPPS(int& v);
		
		void connectToEtherdream();
		void disconnectFromEtherdream();
		void connectButtonPressed(bool&v);
		
		void drawTestPattern();
		
		
		
		void addLaserDot(const ofPoint& ofpoint, ofFloatColor colour, float intensity = 1, int maxpoints = -1);
		void addLaserLine(const ofPoint&startpoint, const ofPoint&endpoint, ofFloatColor colour, float speed = -1, float acceleration = -1);
		void addLaserCircle(const ofPoint& ofpoint, float radius, ofFloatColor colour, float speed = -1, float acceleration = -1, float overlap = -1);
		void addLaserPolyline(const ofPolyline& line, ofColor col, float speed = -1, float acceleration = -1, float cornerthreshold = -1);

		

		void addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint);


		
		void addPoint(ofPoint p, ofFloatColor c, float pointIntensity = 1, bool useCalibration = true);
		void addPoint(ofxLaser::Point p);
		void addPoints(vector<ofxLaser::Point>&points);

		void processIldaPoints();
		
		float calculateCalibratedBrightness(float value, float intensity, float level100, float level75, float level50, float level25, float level0);
		
		
		
		
		ofxEtherdream etherdream;
		ofParameter<string> etherdreamStatus;
		//bool shouldBeConnected;
		
		vector<ofxIlda::Point> ildaPoints;
		vector<ofxLaser::Point> laserPoints;
		
		QuadWarp warp;
		
		ofRectangle maskRectangle;
		
		
		ofParameterGroup parameters;
		ofParameter<bool> connectButton;
		ofParameter<float> intensity;
		ofParameter<int> pps;
		
		ofParameter<int> testPattern;
		int numTestPatterns;
		
		ofParameter<bool> flipX;
		ofParameter<bool> flipY;
		ofParameter<bool> showWarpUI; 
		ofParameter<int> colourChangeDelay;
		
		ofParameter<int> shapePreBlank;
		ofParameter<int> shapePostBlank;
		
		// the number of points for a dot
		ofParameter<int> dotMaxPoints;
		
		
		
		// preview stuff
		
		ofParameter<bool> laserOnWhileMoving;
		
		// Preview parameters refer to the on-screen
		// simulation of what the laser renders.
		ofParameter<bool> renderLaserPathPreview;
		ofParameter<bool> renderLaserPreview;
		ofParameter<bool> showPostTransformPreview;
		
		
		// Mesh used for previewing the laser path
		ofMesh previewPathMesh;
		
		// the speed for movement and acceleration
		ofParameter<float> moveSpeed;
		ofParameter<int>   movePointsPadding;

		ofParameter<float> defaultLineSpeed;
		ofParameter<float> defaultLineAcceleration;
		
		ofParameter<float> defaultCircleSpeed;
		ofParameter<float> defaultCircleAcceleration;
		ofParameter<float> defaultCircleOverlap;

		ofParameter<float> defaultPolylineAcceleration;
		ofParameter<float> defaultPolylineSpeed;
		ofParameter<float> defaultPolylineCornerThreshold;

		ofParameter<float>red100;
		ofParameter<float>red75;
		ofParameter<float>red50;
		ofParameter<float>red25;
		ofParameter<float>red0;
		
		ofParameter<float>green100;
		ofParameter<float>green75;
		ofParameter<float>green50;
		ofParameter<float>green25;
		ofParameter<float>green0;
		
		ofParameter<float>blue100;
		ofParameter<float>blue75;
		ofParameter<float>blue50;
		ofParameter<float>blue25;
		ofParameter<float>blue0;

		ofParameterGroup redParams, greenParams, blueParams;

		
		deque <ofxLaser::Shape*> shapes;
		
		float appWidth;
		float appHeight;

		ofPoint pmin;
		ofPoint pmax;
		int minPoints;
		
		ofPoint laserHomePosition; 
		
		bool offScreen;
		ofVec2f offScreenPoint;
		ofVec2f lastClampedOffScreenPoint;

	};
}


// Modified from Robert Penner's easing equations

class Quint {
	
public:
	
	static float easeIn (float t,float b , float c, float d) {
		t/=d;
		return c*t*t*t*t*t + b;
	}
	static float easeOut(float t,float b , float c, float d) {
		t=t/d-1;
		return c*(t*t*t*t*t + 1) + b;
	}
	
	static float easeInOut(float t,float b , float c, float d) {
		t/=d/2;
		if (t < 1) return c/2*t*t*t*t*t + b;
		;t-=2;
		return c/2*(t*t*t*t*t + 2) + b;
	}
	
};


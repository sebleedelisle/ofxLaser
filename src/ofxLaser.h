//
//  ofxLaser.h
//
//  Created by Seb Lee-Delisle on 13/08/2015.
//

#pragma once

#include "ofxEtherdream.h"
#include "ofxGui.h"
#include "LaserShape.h"
#include "ofxLaserPoint.h"
#include "LaserDot.h"
#include "LaserLine.h"
#include "LaserCircle.h"
#include "LaserPolyline.h"
#include "QuadWarp.h"

#include "Quint.h"

//namespace ofxLaser {
	
	class ofxLaser {
		
		public:
		
		ofxLaser();
		
		void setup(int width, int height);
		void update();
		void draw();
		void drawShapes();
		void renderPreview();
		void renderLaserPath();
		
		void setupParameters();
		void roundPPS(int& v);
		
		void connectToEtherdream();
		void disconnectFromEtherdream();
		void connectButtonPressed();
		
		void drawTestPattern();
		
		void addLaserDot(const ofPoint& ofpoint, ofFloatColor colour, float intensity = 1, int maxpoints = -1);
		void addLaserLine(const ofPoint&startpoint, const ofPoint&endpoint, ofFloatColor colour, float speed = -1, float acceleration = -1);
		void addLaserCircle(const ofPoint& ofpoint, float radius, ofFloatColor colour, float speed = -1, float acceleration = -1, float overlap = -1);
		void addLaserPolyline(const ofPolyline& line, ofColor col, float speed = -1, float acceleration = -1, float cornerthreshold = -1);


		void addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint);


		
		void addPoint(ofPoint p, ofFloatColor c, float pointIntensity = 1, bool useCalibration = true);
		void addPoint(ofxLaserPoint p);
		void addPoints(vector<ofxLaserPoint>&points);

		void processIldaPoints();
		
		ofxEtherdream etherdream;
		ofParameter<string> etherdreamStatus;
		
		vector<ofxIlda::Point> ildaPoints;
		vector<ofxLaserPoint> laserPoints;
		
		QuadWarp warp;
		
		ofRectangle maskRectangle;
		
		
		ofParameterGroup parameters;
		ofxButton connectButton;
		ofParameter<float> intensity;
		ofParameter<int> pps;
		
		ofParameter<int> testPattern;
		int numTestPatterns;
		
		ofParameter<bool> flipX;
		ofParameter<bool> flipY;
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

		
		deque <LaserShape*> shapes;
		
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
//}


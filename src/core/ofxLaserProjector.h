//
//  ofxLaserProjector.hpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//
#pragma once

#include "ofxLaserDacBase.h"
#include "ofxLaserZone.h"
#include "ofxLaserZoneTransform.h"
#include "ofxLaserRenderProfile.h"
#include "ofxLaserManualShape.h"
#include "ofxGui.h"

namespace ofxLaser {

	class SegmentPoints : public vector<Point> {
		
		public:
		bool tested = false;
		bool reversed = false;
		bool reversable = true;
		Point& getStart() {
			return reversed?this->back() : this->front();
		}
		Point& getEnd() {
			return reversed?this->front() : this->back();
		}
	};

	class Projector {
        
		public :
		
		Projector(string projectorlabel, DacBase& dac);
		~Projector();
		
        void initGui(bool showAdvanced = false);
        void initListeners();
        bool mousePressed(ofMouseEventArgs &e);
		
		void update(bool updateZones);
		void send(ofPixels* pixels = NULL);
        void sendRawPoints(const vector<Point>& points, int zonenum = 0);
        int getPointRate() {
            return pps; 
        };
        float getFrameRate() {
            if(numPoints>0) return (float)pps/(float)numPoints;
            else return pps;
        }
		
		void renderStatusBox(float x=0, float y=0, float w=300, float h=100);
		
		void addZone(Zone* zone, float srcwidth, float srcheight);
		void drawLaserPath(float x=0, float y=0, float w=800, float h=800);
		void drawWarpUI(float x=0, float y=0, float w=800, float h=800);
		void hideGui();
		void showGui();

		void addPoint(ofxLaser::Point p);
		void addPoint(ofPoint p, ofFloatColor c, float pointIntensity = 1, bool useCalibration = true);
		void addPoints(vector<ofxLaser::Point>&points);

		void addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint, vector<Point>& points);
		void addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint);
		void processPoints();
		void saveSettings();
		
		RenderProfile& getRenderProfile(string profilelabel);
		
		void updateZoneMasks();
		void zoneMaskChanged(ofAbstractParameter& e);
		void ppsChanged(int& e);
		
		void minimiseGui();
		
		deque<Shape*> getTestPatternShapesForZone(int zoneindex);
		float calculateCalibratedBrightness(float value, float intensity, float level100, float level75, float level50, float level25, float level0);


		vector<Zone*> zones;
		vector<ZoneTransform*> zoneWarps;
		vector<ofRectangle> zoneMasks;
		
		vector<ofParameter<float>>leftEdges;
		vector<ofParameter<float>>rightEdges;
		vector<ofParameter<float>>topEdges;
		vector<ofParameter<float>>bottomEdges;
		
		//ofRectangle maskRectangle;
		
        vector<ofParameter<bool>> zonesMuted;
        vector<ofParameter<bool>> zonesSoloed;
        vector<bool> zonesEnabled;

		DacBase& dac;
		
		const int min = -32768;
		const int max = 32767;
		
		ofPoint laserHomePosition;
		string label;
		float smoothedFrameRate = 0; 
		vector<Point> laserPoints;
        int numPoints;
		ofMesh previewPathMesh;
		
		bool offScreen; // keeps track of whether we are going outside the mask
		Point offScreenPoint; // marks the position where we left the mask
		
		ofParameter<bool> armed;
		ofParameter<int> pps;
        ofParameter<float> speedMultiplier; 
		ofParameter<float>intensity;
		ofParameter<bool> resetDac;
		
		ofParameter<float> colourChangeOffset;
		ofParameter<int> testPattern;
		int numTestPatterns; 
		ofParameter<bool> flipX;
		ofParameter<bool> flipY;
		
		ofParameter<float> moveSpeed = 5;
		ofParameter<int> shapePreBlank = 0;
		ofParameter<int> shapePostBlank = 0;
		ofParameter<int> shapePreOn = 0;
		ofParameter<int> shapePostOn = 0;

		ofParameter<bool> laserOnWhileMoving = false;
		
		ofParameter<glm::vec2> outputOffset;
		
		map<string, RenderProfile> renderProfiles;

		// would probably be sensible to move these settings out into a colour
		// calibration object.
        
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
		
		ofxPanel* gui;
        //bool guiIsVisible;
		bool guiInitialised = false; 

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


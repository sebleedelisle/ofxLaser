//
//  ofxLaserProjector.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//
#pragma once

//#include "ofxLaserManager.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserDacEmpty.h"
#include "ofxLaserZone.h"
#include "ofxLaserProjectorZone.h"
#include "ofxLaserRenderProfile.h"
#include "ofxLaserManualShape.h"
#include "PennerEasing.h"
#include "ofxLaserMaskManager.h"

namespace ofxLaser {

	// a container than holds all the points for a shape
    // it extends a vector of ofxLaser::Point objects
	class PointsForShape : public vector<Point> {
		
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
		
		Projector(int _index);
		~Projector();
		
        void init();
        
        string getLabel() {
            return "Projector " + ofToString(projectorIndex+1);
        }
        void setDac(DacBase* dac);
        DacBase* getDac();
        bool removeDac();
        
        bool loadSettings(vector<Zone*>& zones);
        bool saveSettings();
        
		void setDefaultHandleSize(float size);
     	
		void update(bool updateZones);
		void send(ofPixels* pixels = NULL, float masterIntensity = 1);
        
        void setArmed(bool& armed); 
        
        // adds all the shape points to the vector passed in
		void getAllShapePoints(vector<PointsForShape>* allzoneshapepoints, ofPixels*pixels, float speedmultiplier);

		
        void sendRawPoints(const vector<Point>& points, Zone* zone, float masterIntensity =1);
        int getPointRate() {
            return pps; 
        };
        float getFrameRate() {
            if(numPoints>0) return (float)pps/(float)numPoints;
            else return pps;
        }
		
        // DAC
        
        string getDacLabel() ;
        bool getDacConnectedState();
		
        // Zones
        
		void addZone(Zone* zone, float srcwidth, float srcheight);
        bool hasZone(Zone* zone);
        bool removeZone(Zone* zone); 
		void drawLaserPath(ofRectangle rect);
		void drawLaserPath(float x=0, float y=0, float w=800, float h=800);
		void drawWarpUI(float x=0, float y=0, float w=800, float h=800);
		void hideWarpGui();
		void showWarpGui();

        
        vector<ProjectorZone*> projectorZones;
   
        ProjectorZone* getProjectorZoneForZone(Zone* zone) {
            for(ProjectorZone* projectorZone : projectorZones) {
                if(&projectorZone->zone == zone) return projectorZone;
            }
            return nullptr;

        }
        
        // Managing points
        void addPoint(ofxLaser::Point p);
		void addPoint(ofPoint p, ofFloatColor c, float pointIntensity = 1, bool useCalibration = true);
		void addPoints(vector<ofxLaser::Point>&points, bool reversed = false);

		void addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint);
		void processPoints(float masterIntensity, bool offsetColours = true);
		
		RenderProfile& getRenderProfile(string profilelabel);
		
		void updateZoneMasks();
		
		void ppsChanged(int& e);
		
		deque<Shape*> getTestPatternShapesForZone(ProjectorZone& zone);
		
        float calculateCalibratedBrightness(float value, float intensity, float level100, float level75, float level50, float level25, float level0);

        //-----------------------------------------------
        
        
        int projectorIndex; 
        
        DacEmpty emptyDac; 

        MaskManager maskManager; 
		
		float defaultHandleSize = 8;

		DacBase* dac;
		
		const int min = -32768;
		const int max = 32767;
		
		ofPoint laserHomePosition;
		float smoothedFrameRate = 0;
		vector<Point> laserPoints;
		vector<Point> sparePoints;
		vector<Point> sparePoints2;
		unsigned long frameCounter = 0; 
		
        int numPoints;
		ofMesh previewPathMesh;
		
		ofParameter<bool> armed;
		ofParameter<int> pps;
        ofParameter<float> speedMultiplier; 
		ofParameter<float>intensity;
        // used to keep track of the dac that we're connected to
        // (particularly when loading / saving)
        ofParameter<string> dacId;
		
		ofParameter<float> colourChangeShift;
		ofParameter<int> testPattern;
		int numTestPatterns; 
		ofParameter<bool> flipX;
		ofParameter<bool> flipY;
		ofParameter<float> rotation;
        ofParameter<bool>hideContentDuringTestPattern;
        ofParameter<glm::vec2> outputOffset;
        
		ofParameter<float>targetFramerate;
		ofParameter<bool>syncToTargetFramerate;
		ofParameter<int>syncShift;
		ofParameter<bool>sortShapes;
		
		ofParameter<float> moveSpeed = 5;
		ofParameter<int> shapePreBlank = 0;
		ofParameter<int> shapePostBlank = 0;
		ofParameter<int> shapePreOn = 0;
		ofParameter<int> shapePostOn = 0;
		
		ofParameter<bool> smoothHomePosition; 

		ofParameter<bool> laserOnWhileMoving = false;
		
		map<string, RenderProfile> renderProfiles;

		// would probably be sensible to move these settings out into a colour
		// calibration object.
        ofParameterGroup colourParams;
        
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
		
		//ofxPanel* gui;
        //bool guiIsVisible;
        ofParameterGroup params;
        ofParameterGroup advancedParams; 
		bool guiInitialised = false; 

	};

}


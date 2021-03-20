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
// #include "ofxGui.h"
#include "ofxImGui.h"
#include "PennerEasing.h"

namespace ofxLaser {

	// a container than holds all the points for a shape
	class ShapePoints : public vector<Point> {
		
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
        void drawGui();
        
        static void HelpMarker(const char* desc)
        {
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(desc);
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }
        
		void setDefaultHandleSize(float size);
        void initListeners();
        bool mousePressed(ofMouseEventArgs &e);
		
		void update(bool updateZones);
		void send(ofPixels* pixels = NULL, float masterIntensity = 1);
        
        void setArmed(bool& armed); 
        
		void getAllShapePoints(vector<ShapePoints>* allzoneshapepoints, ofPixels*pixels, float speedmultiplier);

		
        void sendRawPoints(const vector<Point>& points, int zonenum = 0, float masterIntensity =1);
        int getPointRate() {
            return pps; 
        };
        float getFrameRate() {
            if(numPoints>0) return (float)pps/(float)numPoints;
            else return pps;
        }
		
		void renderStatusBox(float x=0, float y=0, float w=300, float h=100);
		
		void addZone(Zone* zone, float srcwidth, float srcheight);
		void drawLaserPath(ofRectangle rect);
		void drawLaserPath(float x=0, float y=0, float w=800, float h=800);
		void drawWarpUI(float x=0, float y=0, float w=800, float h=800);
		void hideWarpGui();
		void showWarpGui();

		void addPoint(ofxLaser::Point p);
		void addPoint(ofPoint p, ofFloatColor c, float pointIntensity = 1, bool useCalibration = true);
		void addPoints(vector<ofxLaser::Point>&points, bool reversed = false);

		void addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint, vector<Point>& points);
		void addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint);
		void processPoints(float masterIntensity, bool offsetColours = true);
		void saveSettings();
		
		RenderProfile& getRenderProfile(string profilelabel);
		
		void updateZoneMasks();
		void zoneMaskChanged(ofAbstractParameter& e);
		void ppsChanged(int& e);
		
		void minimiseGui();
		
		deque<Shape*> getTestPatternShapesForZone(int zoneindex);
		float calculateCalibratedBrightness(float value, float intensity, float level100, float level75, float level50, float level25, float level0);


		vector<Zone*> zones;
		vector<ZoneTransform*> zoneTransforms;
		vector<ofRectangle> zoneMasks;
		
		vector<ofParameter<float>>leftEdges;
		vector<ofParameter<float>>rightEdges;
		vector<ofParameter<float>>topEdges;
		vector<ofParameter<float>>bottomEdges;
		
		//ofRectangle maskRectangle;
		
        vector<ofParameter<bool>> zonesMuted;
        vector<ofParameter<bool>> zonesSoloed;
        vector<bool> zonesEnabled;
		
		float defaultHandleSize = 8;

		DacBase* dac;
		
		const int min = -32768;
		const int max = 32767;
		
		ofPoint laserHomePosition;
		string label;
		float smoothedFrameRate = 0; 
		vector<Point> laserPoints;
		vector<Point> sparePoints;
		vector<Point> sparePoints2;
		unsigned long frameCounter = 0; 
		
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
		ofParameter<float> rotation;
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
		
		//ofxPanel* gui;
        //bool guiIsVisible;
        ofParameterGroup params; 
		bool guiInitialised = false; 

	};

}


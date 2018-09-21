//
//  ofxLaserManager.hpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#pragma once
#include "ofMain.h"
#include "ofxLaserZone.h"
#include "ofxLaserShape.h"
#include "ofxLaserLine.h"
#include "ofxLaserPolyline.h"
#include "ofxLaserCircle.h"
#include "ofxLaserProjector.h"
#include "ofxLaserDacBase.h"
#include "ofxGui.h"
#include "ofxLaserMaskManager.h"

#define OFXLASER_PROFILE_FAST "FAST"
#define OFXLASER_PROFILE_DEFAULT "DEFAULT"
#define OFXLASER_PROFILE_DETAIL "DETAIL"


enum ofxLaserZoneMode {
	OFXLASER_ZONE_MANUAL, // all zones are separate, you manually specify which zone you want
	OFXLASER_ZONE_AUTOMATIC // non-overlapping zones assumed - shapes go in all zones that
							// contain it
	//OFXLASER_ZONE_OPTIMISE, // automatically puts it in the best zone
	
	//OFXLASER_ZONE_OVERLAY // doubles up multiple lasers for improved brightness - actually not required cos
							// AUTOMATIC does the same thing
	
	
};

namespace ofxLaser {

	class Manager {

		public :

		// it's a Singleton so shouldn't ever have more than one.
		static Manager * instance();
		static Manager * laserManager;

		Manager();
		~Manager();
        
		void addProjector(DacBase& dac);
		
		void addZone(float x, float y, float w, float h);
		void addZone(const ofRectangle& zoneRect);
		
		void addZoneToProjector(int zonenum, int projnum);
		
		void nextProjector();

		void setup(int width, int height);
		void update();
		void drawUI(bool fullscreen = false);
        void renderPreview();
        
        void updateScreenSize(ofResizeEventArgs &e);
		void updateScreenSize(); 
        void send();
        void sendRawPoints(const std::vector<ofxLaser::Point>& points, int projectornum = 0, int zonenum = 0);
        
        int getProjectorPointRate(int projectornum = 0);
        float getProjectorFrameRate(int projectornum); 

        
		void drawPoly(const ofPolyline &poly, const ofColor& col,  string profileName = OFXLASER_PROFILE_DEFAULT);
		void drawPoly(const ofPolyline & poly, vector<ofColor>& colours, string profileName = OFXLASER_PROFILE_DEFAULT);
		void drawLine(const ofPoint& start, const ofPoint& end, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT);
		void drawDot(const ofPoint& p, const ofColor& col, float intensity =1, string profileName = OFXLASER_PROFILE_DEFAULT);
		void drawCircle(const ofPoint & centre, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);

        Projector& getProjector(int index = 0){
            return *projectors.at(index);
        };
        
		void initGui(bool showAdvanced = false);
        bool togglePreview(){
            showPreview = !showPreview;
            return showPreview;
        };
        
		void saveSettings();
		
		Zone& getZone(int zonenum) {
            // TODO bounds check? 
            return *zones[zonenum-1];
            
        }
		
		bool setTargetZone(int zone){  // only for OFX_ZONE_MANUAL
			if(zone>=zones.size()) return false;
			else if(zone<0) return false;
			else {
				targetZone = zone;
				return true;
			}
		}
		
		bool setZoneMode(ofxLaserZoneMode newmode) {
			zoneMode = newmode;
		}
		
		
		int width, height;
		ofxPanel gui;
        bool guiIsVisible;
        
		// converts openGL coords to screen coords //
		
		ofPoint gLProject(ofPoint p);
		ofPoint gLProject( float ax, float ay, float az ) ;
		
		int currentProjector;
		
        ofParameter<bool> showZones;
        ofParameter<bool> showPreview;
        ofParameter<bool> useBitmapMask;
        ofParameter<bool> showBitmapMask; 
        
        MaskManager laserMask;
        
		private:
		int createDefaultZone();
		
		ofxLaserZoneMode zoneMode = OFXLASER_ZONE_AUTOMATIC;
		int targetZone = 0; // for OFXLASER_ZONE_MANUAL mode
		
		std::vector<Zone*> zones;
		std::vector<Projector*> projectors;
		
		std::deque <ofxLaser::Shape*> shapes;
		ofParameter<int> testPattern;
		
		 
		
		ofPolyline tmpPoly; // to avoid generating polyline objects
		int screenHeight;
		
		
	};
}


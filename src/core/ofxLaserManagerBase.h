//
//  ofxLaserManager.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#pragma once

#include "ofMain.h"
#include "constants.h"
#include "ofxLaserDacAssigner.h"
#include "ofxLaserZone.h"
#include "ofxLaserShape.h"
#include "ofxLaserLine.h"
#include "ofxLaserPolyline.h"
#include "ofxLaserCircle.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserBitmapMaskManager.h"
#include "ofxLaserGraphic.h"
#include "ofxLaserLaser.h"
#include "ofxLaserPresetManager.h"



enum ofxLaserZoneMode {
    OFXLASER_ZONE_MANUAL, // all zones are separate, you manually specify which zone you want
    OFXLASER_ZONE_AUTOMATIC, // non-overlapping zones assumed - shapes go in all zones that
    // contain it
    OFXLASER_ZONE_OPTIMISE // automatically puts it in the best zone -- NOT CURRENTLY IMPLEMENTED
};

namespace ofxLaser {
class UI;

class ManagerBase {
    
    public :
    
    // it's a Singleton so shouldn't ever have more than one.
    static ManagerBase * instance();
    static ManagerBase * laserManager;
    
    ManagerBase();
    ~ManagerBase();
    
    
    virtual void update();
   
    virtual bool deleteLaser(Laser* laser);
    
    void addZone(float x = 0 , float y = 0, float w = -1, float h= -1);
    void addZone(const ofRectangle& zoneRect);
    bool deleteZone(Zone* zone); 

    void renumberZones(); 
    
    void addZoneToLaser(unsigned int zonenum, unsigned int lasernum);
    
    virtual void setCanvasSize(int width, int height);
    
    void beginDraw();
    void endDraw();
    
    bool loadSettings();
    bool saveSettings();
    
    void send();
    void sendRawPoints(const std::vector<ofxLaser::Point>& points, int lasernum = 0, int zonenum = 0);
    
    int getLaserPointRate(unsigned int lasernum = 0);
    float getLaserFrameRate(unsigned int lasernum);
    
    void armAllLasersListener();
    void disarmAllLasersListener();
    void armAllLasers();
    void disarmAllLasers();
    void testPatternAllLasers(int& pattern);
    
    void drawPoly(const ofPolyline &poly, const ofColor& col,  string profileName = OFXLASER_PROFILE_DEFAULT, float brightness = 1);
    void drawPoly(const ofPolyline & poly, vector<ofColor>& colours, string profileName = OFXLASER_PROFILE_DEFAULT, float brightness =1);
    void drawPolyFromPoints(const vector<glm::vec3>& points, const vector<ofColor>& colours, string profileName = OFXLASER_PROFILE_DEFAULT, float brightness =1);
   
    void drawLine(const glm::vec3& start, const glm::vec3& end, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawLine(const glm::vec2& start, const glm::vec2& end, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawLine(float x1, float y1, float x2, float y2, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT);

    void drawDot(const glm::vec3& p, const ofColor& col, float intensity =1, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawDot(const glm::vec2& p, const ofColor& col, float intensity =1, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawDot(float x, float y, const ofColor& col, float intensity =1, string profileName = OFXLASER_PROFILE_DEFAULT);
    
    void drawCircle(const float& x, const float& y, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
    void drawCircle(const glm::vec3& centre, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
    void drawCircle(const glm::vec2& centre, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
   
    void drawLaserGraphic(Graphic& graphic, float brightness = 1, string renderProfile = OFXLASER_PROFILE_DEFAULT);
    
    vector<Laser*>& getLasers();
    Laser& getLaser(int index = 0);
    int getNumLasers() { return (int)lasers.size(); };
    
    OF_DEPRECATED_MSG("ofxLaser::Manager::initGui(bool showAdvanced) - show advanced parameter no longer a feature", void initGui(bool showAdvanced));
    OF_DEPRECATED_MSG("You no longer need to call ofxLaser::Manager::setup(width, height). If you want to set the size, use setCanvasSize(w,h)", void setup(int w, int h));
  
    OF_DEPRECATED_MSG("Lasers are no longer set up in code, use the UI within the app.", void addProjector(DacBase&));
    OF_DEPRECATED_MSG("Lasers are no longer set up in code, use the UI within the app.", void addProjector());
  
    
     
   // Zone& getZone(int zonenum);
    Zone* getZone(int zonenum);
    int getNumZones();
    Zone* getSelectedZone(); 
    bool setTargetZone(unsigned int zone);
    int getTargetZone();
    bool setZoneMode(ofxLaserZoneMode newmode);
    
    bool isLaserArmed(unsigned int i);
	bool areAllLasersArmed();
    
    ofPoint gLProject(ofPoint p);
    ofPoint gLProject( float ax, float ay, float az ) ;
  
    //--------------------------------------------------------
    
    DacAssigner& dacAssigner;
    
    int width, height;

    ofParameter<int> testPattern;
    
  
   
    ofParameter<bool> useBitmapMask;
    ofParameter<bool> showBitmapMask;
    ofParameter<bool> laserMasks;
    ofParameter<int> numLasers; // << not used except for load / save
    

    
    ofParameter<float>globalBrightness;

    
    BitmapMaskManager laserMask;
    
    
    bool zonesChanged;
    std::vector<Zone*> zones;
    
    ofParameterGroup params;
 
    ofJson loadJson;
    
    protected :
    ofFbo canvasPreviewFbo;
    
    
    
    bool doArmAll = false;
    bool doDisarmAll = false;
    
    void createAndAddLaser();
    
    int createDefaultZone();
    
    ofxLaserZoneMode zoneMode = OFXLASER_ZONE_AUTOMATIC;
    int targetZone = 0; // for OFXLASER_ZONE_MANUAL mode
    
    std::vector<Laser*> lasers;
    
    std::deque <ofxLaser::Shape*> shapes;
    //ofParameter<int> testPattern;
    
    ofPolyline tmpPoly; // to avoid generating polyline objects
    vector<glm::vec3> tmpPoints;
  //  vector<ofColor> tmpColours;
    
    bool dacsInitialised = false; 
    
    private:
};
}


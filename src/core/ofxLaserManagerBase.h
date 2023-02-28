//
//  ofxLaserManager.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserConstants.h"
#include "ofxLaserDacAssigner.h"
#include "ofxLaserInputZone.h"
#include "ofxLaserShape.h"
#include "ofxLaserLine.h"
#include "ofxLaserPolyline.h"
#include "ofxLaserCircle.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserBitmapMaskManager.h"
#include "ofxLaserGraphic.h"
#include "ofxLaserLaser.h"
#include "ofxLaserZoneContent.h"

#include "ofxLaserShapeTargetCanvas.h"
#include "ofxLaserShapeTargetBeamZone.h"
#include "ofxLaserBeamZoneContainer.h"

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
    
    ZoneId createNewBeamZone();
    bool addBeamZoneToLaser(int beamZoneIndex, int laserIndex) ;
    
    void addCanvasZone(float x = 0 , float y = 0, float w = -1, float h= -1);
    void addCanvasZone(const ofRectangle& zoneRect);
    bool deleteCanvasZone(InputZone* zone);
    
    bool hasAnyAltZones();
    void setAllAltZones();
    void unSetAllAltZones(); 

    void addZoneToLaser(ZoneId& zoneId, unsigned int lasernum);
    
    virtual void setCanvasSize(int width, int height);
    
    bool loadSettings();
    bool saveSettings();
    
    void send();
    void sendRawPoints(const std::vector<ofxLaser::Point>& points, int lasernum = 0, ZoneId* zoneId = nullptr);
    
    int getLaserPointRate(unsigned int lasernum = 0);
    float getLaserFrameRate(unsigned int lasernum);
    
    void armAllLasers();
    void disarmAllLasers();
    void updateGlobalTestPattern();
    //void canvasSizeChanged(int&size);
    void useAltZonesChanged(bool& state); 
    bool areAllLasersUsingAlternateZones();

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
  

    bool setTargetBeamZone(int index);
    bool setTargetCanvas(int canvasIndex = 0);
    
    //bool setTargetZone(unsigned int zone);
    //int getTargetZone();
    //bool setZoneMode(ofxLaserZoneMode newmode);
    
    bool isLaserArmed(unsigned int i);
	bool areAllLasersArmed();
    
    void beginDraw() {
        // to do : check target
        ofViewport((ofGetWidth()-canvasTarget.getWidth())/-2, (ofGetHeight()-canvasTarget.getHeight())/-2, ofGetWidth(), ofGetHeight()) ;
        ofPushMatrix();
        ofTranslate((ofGetWidth()-canvasTarget.getWidth())/2, (ofGetHeight()-canvasTarget.getHeight())/2);
        
    }
    void endDraw() {
        ofPopMatrix();
        ofViewport(0,0,ofGetWidth(), ofGetHeight());
    }
    
    
  
    //--------------------------------------------------------
    
    DacAssigner& dacAssigner;
    
    //ofParameter<int> canvasWidth, canvasHeight;

    int testPatternGlobal;
    bool testPatternGlobalActive;
    
    ofParameter<bool> useAltZones;
    //ofParameter<bool> useBitmapMask;
    //ofParameter<bool> showBitmapMask;
    //ofParameter<bool> laserCanvasMaskOutlines;
    ofParameter<int> numLasers; // << not used except for load / save
    
    ofParameter<bool> dontCalculateDisconnected;
    
    ofParameter<float>globalBrightness;

    BitmapMaskManager laserMask;
                
  //  bool zonesChanged;
    //std::vector<InputZone*> zones;
    
    ofParameterGroup params;
 
    ofJson loadedJson;
    
    protected :
    
    ofPoint convert3DTo2D(ofPoint p);
    ofPoint convert3DTo2D( float ax, float ay, float az );
    
    virtual void createAndAddLaser();
    
    void createDefaultCanvasZone();
    
    //ofxLaserZoneMode zoneMode = OFXLASER_ZONE_AUTOMATIC;
    //int targetZone = 0; // for OFXLASER_ZONE_MANUAL mode
    
    std::vector<Laser*> lasers;
    
    ShapeTarget* currentShapeTarget; 
    ShapeTargetCanvas canvasTarget;
    BeamZoneContainer beamZoneContainer;
    //vector<ShapeTargetBeamZone> zones;
    //std::deque <ofxLaser::Shape*> shapes;

    // used in "drawPoly" functions
    // to avoid generating polyline objects
    ofPolyline tmpPoly;
    vector<glm::vec3> tmpPoints;
  
    ofSoundPlayer beepSound; 
   
    private:
};
}


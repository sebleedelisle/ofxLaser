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
#include "ofxLaserUI.h"
#include "ofxLaserProjector.h"
#include "ofxLaserPresetManager.h"



enum ofxLaserZoneMode {
    OFXLASER_ZONE_MANUAL, // all zones are separate, you manually specify which zone you want
    OFXLASER_ZONE_AUTOMATIC, // non-overlapping zones assumed - shapes go in all zones that
    // contain it
    OFXLASER_ZONE_OPTIMISE // automatically puts it in the best zone -- NOT CURRENTLY IMPLEMENTED
};

namespace ofxLaser {
class UI;

class Manager {
    
    public :
    
    // it's a Singleton so shouldn't ever have more than one.
    static Manager * instance();
    static Manager * laserManager;
    
    Manager();
    ~Manager();
    
    OF_DEPRECATED_MSG("You no longer need to call ofxLaser::Manager::setup(widht, height). If you want to set the size, use setCanvasSize(w,h)", void setup(int w, int h));
  
    OF_DEPRECATED_MSG("Projectors are no longer set up in code, use the UI within the app.", void addProjector(DacBase&));
    OF_DEPRECATED_MSG("Projectors are no longer set up in code, use the UI within the app.", void addProjector());
   
    bool deleteProjector(Projector* projector);
    
    
    void addZone(float x = 0 , float y = 0, float w = -1, float h= -1);
    void addZone(const ofRectangle& zoneRect);
    bool deleteZone(Zone* zone); 

    void renumberZones(); 
    
    void addZoneToProjector(unsigned int zonenum, unsigned int projnum);
    
    void nextProjector();
    void previousProjector();
    
    void setCanvasSize(int width, int height);
    void update();
    void drawUI(bool fullscreen = false);
    void drawPreviews(bool fullscreen = false);
    
    
    void renderPreview();
    
    void beginDraw();
    void endDraw();
    
    bool loadSettings();
    bool saveSettings();
    
    void send();
    void sendRawPoints(const std::vector<ofxLaser::Point>& points, int projectornum = 0, int zonenum = 0);
    
    int getProjectorPointRate(unsigned int projectornum = 0);
    float getProjectorFrameRate(unsigned int projectornum);
    
    void armAllProjectorsListener();
    void disarmAllProjectorsListener();
    void armAllProjectors();
    void disarmAllProjectors();
    void testPatternAllProjectors(int& pattern);
    
    void drawPoly(const ofPolyline &poly, const ofColor& col,  string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawPoly(const ofPolyline & poly, vector<ofColor>& colours, string profileName = OFXLASER_PROFILE_DEFAULT);
    
    void drawLine(const glm::vec3& start, const glm::vec3& end, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawLine(const glm::vec2& start, const glm::vec2& end, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawLine(float x1, float y1, float x2, float y2, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT);

    void drawDot(const glm::vec3& p, const ofColor& col, float intensity =1, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawDot(const glm::vec2& p, const ofColor& col, float intensity =1, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawDot(float x, float y, const ofColor& col, float intensity =1, string profileName = OFXLASER_PROFILE_DEFAULT);
    
    void drawCircle(const float& x, const float& y, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
    void drawCircle(const glm::vec3& centre, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
    void drawCircle(const glm::vec2& centre, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
   
    vector<Projector*>& getProjectors();
    Projector& getProjector(int index = 0);
    int getNumProjectors() { return (int)projectors.size(); };
    
    OF_DEPRECATED_MSG("ofxLaser::Manager::initGui(bool showAdvanced) - show advanced parameter no longer a feature", void initGui(bool showAdvanced));

    void initAndLoadSettings();
    void addCustomParameter(ofAbstractParameter& param);
    
    void setDefaultHandleSize(float size);
    
    void drawProjectorPanel(ofxLaser::Projector* projector, float projectorpanelwidth, float spacing, float x);
    void drawLaserGui();
    bool togglePreview();
    bool toggleGui();
    void setGuiVisible(bool visible);
    bool isGuiVisible();
    
    Zone& getZone(int zonenum);
    int getNumZones();
    Zone* getSelectedZone(); 
    bool setTargetZone(unsigned int zone);
    bool setZoneMode(ofxLaserZoneMode newmode);
    
    // should be called before initGui
    bool setGuideImage(string filename);
    bool isProjectorArmed(unsigned int i);
	bool areAllLasersArmed();

    //--------------------------------------------------------
    
    DacAssigner& dacAssigner;
    
    int width, height;
    int guiProjectorPanelWidth;
    int dacStatusBoxSmallWidth;
    int dacStatusBoxHeight;
    int guiSpacing;
    // converts openGL coords to screen coords //
    
    ofPoint gLProject(ofPoint p);
    ofPoint gLProject( float ax, float ay, float az ) ;
    
    int currentProjector;
    
    ofParameter<int> testPattern;
    
    ofParameter<bool> showProjectorSettings;
    
    ofParameter<bool> lockInputZones;
    ofParameter<bool> showInputPreview;
    ofParameter<bool> showOutputPreviews;
    ofParameter<bool> useBitmapMask;
    ofParameter<bool> showBitmapMask;
    ofParameter<bool> laserMasks;
    ofParameter<int> numLasers; // << not used except for load / save
    
    float defaultHandleSize = 10;
    
    ofParameter<float>globalBrightness;
    ofImage guideImage;
    
    BitmapMaskManager laserMask;
    
    ofFbo previewFbo;
    glm::vec2 previewOffset;
    float previewScale;
    
    bool zonesChanged;
    std::vector<Zone*> zones;
    
    ofParameterGroup params;
    ofParameterGroup interfaceParams;
    ofParameterGroup customParams;
    
    protected :
	  
    bool guiIsVisible;
    bool initialised = false;
    
    bool showEditScannerPreset = false;
    
    bool doArmAll = false;
    bool doDisarmAll = false;
    
private:
    
    void createAndAddProjector();
    
    int createDefaultZone();
    
    ofxLaserZoneMode zoneMode = OFXLASER_ZONE_AUTOMATIC;
    int targetZone = 0; // for OFXLASER_ZONE_MANUAL mode
    
    std::vector<Projector*> projectors;
    
    std::deque <ofxLaser::Shape*> shapes;
    //ofParameter<int> testPattern;
    
    ofPolyline tmpPoly; // to avoid generating polyline objects
    
};
}


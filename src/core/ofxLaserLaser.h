//
//  ofxLaserLaser.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
// The Laser class manages everything to do with an individual laser projector.
// It keeps a reference to its own DAC (controller), holds all the various settings
// objects, optimises the laser path and calculates all the points.
//
// It also looks after the LaserZone objects which own all the data about how
// each zone is transformed within this laser output area. They also manage the UI
// elements for those transformation objects.
//

#pragma once

#include "ofxLaserConstants.h"
#include "ofxLaserPointsForShape.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserDacEmpty.h"
#include "ofxLaserInputZone.h"
#include "ofxLaserOutputZone.h"
#include "ofxLaserManualShape.h"
#include "PennerEasing.h"
#include "ofxLaserMaskManager.h"
#include "ofxLaserScannerSettings.h"
#include "ofxLaserLine.h"
#include "ofxLaserColourSettings.h"
#include "ofxLaserCircle.h"

namespace ofxLaser {

class Laser {
    
    public :
    
    Laser(int _index);
    ~Laser();
    
    void init();
    
    string getLabel();
    void setDac(DacBase* dac);
    DacBase* getDac();
    bool removeDac();
    
    bool hasDac(); 
    
    void paramsChanged(ofAbstractParameter& e);
    bool loadSettings(vector<InputZone*>& zones);
    bool saveSettings();
    bool getSaveStatus();
    
    void setGrid(bool gridstate, int gridsize); 
    
    
    void update(bool updateZones);
    void send( float masterIntensity = 1, ofPixels* pixelmask = NULL);
    
    bool toggleArmed(); 
   
    // adds all the shape points to the vector passed in
    void getAllShapePoints(vector<PointsForShape>* allzoneshapepoints, ofPixels*pixels, float speedmultiplier);

    
    void sendRawPoints(const vector<Point>& points, InputZone* zone, float masterIntensity =1);
    int getPointRate();
    float getFrameRate();
    
    // DAC
    
    string getDacLabel() ;
    //string getDacAlias() ;
    int getDacConnectedState();
    
    // Zones
    
    void addZone(InputZone* zone, float srcwidth, float srcheight, bool isAlternate = false);
    bool hasZone(InputZone* zone);
    bool removeZone(InputZone* zone);
    bool muteZone(int zonenum);
    bool unMuteZone(int zonenum);
    bool soloZone(int zonenum);
    bool unSoloZone(int zonenum);
  
    // Alternative zones
    void addAltZone(InputZone* zone, float srcwidth, float srcheight);
    bool hasAltZone(InputZone* zone);
    bool removeAltZone(InputZone* zone);
    bool hasAnyAltZones();
    
    OutputZone* getLaserZoneForZone(InputZone* zone);
    OutputZone* getLaserAltZoneForZone(InputZone* zone);

    vector<OutputZone*>getActiveZones();
    bool areAnyZonesSoloed();
    
    void drawLaserPath(ofRectangle rect, bool drawDots = true, bool showMovement = true, float radius = 4);
    //void drawLaserPath(float x=0, float y=0, float w=800, float h=800, bool drawDots = true, float radius = 4);
    void drawLaserPath(bool drawDots = true, bool showMovement = true);
   //void drawTransformUI(float x=0, float y=0, float w=800, float h=800);
    void drawTransformUI();
    
    void zoomAroundPoint(glm::vec2 anchor, float zoomMultiplier);
    void startDrag(glm::vec2 p);
    void stopDrag();
    void setOffsetAndScale(glm::vec2 newoffset =glm::vec2(0,0), float newscale = 1); 
    
    void disableTransformGui();
    void enableTransformGui();
    
    void drawTransformAndPath(ofRectangle rect); 

    vector<Point>& getLaserPoints() { return laserPoints;}; 
   
   
    // Managing points
    void addPoint(ofxLaser::Point p);
    void addPoint(ofPoint p, ofFloatColor c, bool useCalibration = true);
    void addPoints(vector<ofxLaser::Point>&points, bool reversed = false);

    void addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint);
    void processPoints(float masterIntensity, bool offsetColours = true);
    
    RenderProfile& getRenderProfile(string profilelabel);
    
    void updateZoneMasks();
    
    void ppsChanged(int& e);
    void colourShiftChanged(float& e);
    deque<Shape*> getTestPatternShapesForZone(OutputZone& zone);
    float getMoveDistanceForShapes(vector<PointsForShape>& shapes);
    float getMoveDistanceForShapes(vector<PointsForShape*>& shapes);


    //-----------------------------------------------
    
    int laserIndex;
    
    // LaserZone stores a reference to the source zone,
    // and it
    vector<OutputZone*> outputZones;
    
    string savePath ="ofxLaser/lasers/";
    
    ofParameterGroup params;
    ofParameter<bool> armed;
    ofParameter<int> pps;
    ofParameter<float> speedMultiplier;
    ofParameter<float>intensity;
    
    ofParameter<bool> paused; // pause frame
    bool pauseStateRecorded;
    map<InputZone*, deque<Shape*>> pauseShapesByZone;
    
    // used to keep track of the dac that we're connected to
    // (particularly when loading / saving)
    ofParameter<string> dacLabel;
    // Keep track of the dac alias, even when it's not connected.
    // The ID is unique so should be used to reference the DAC.
    // The alias can be anything. 
    //ofParameter<string> dacAlias;
 
    ofParameter<float> colourChangeShift;
    int maxLatencyMS; 
    
    ofParameter<int> testPattern;
    ofParameter<bool>hideContentDuringTestPattern;
 
    int numTestPatterns;
    ofParameter<bool> useAlternate;
    ofParameter<bool> muteOnAlternate;
    
    ofParameter<bool> flipX;
    ofParameter<bool> flipY;
    ofParameter<int> mountOrientation;
    ofParameter<float> rotation;
    ofParameter<glm::vec2> outputOffset;
    
    ScannerSettings scannerSettings; 
    ColourSettings colourSettings; 
    // advanced settings
    ofParameterGroup advancedParams;
    ofParameter<float> targetFramerate;
    ofParameter<bool> syncToTargetFramerate;
    ofParameter<int> syncShift;
    ofParameter<bool> sortShapes;
    ofParameter<bool> newShapeSortMethod;
    ofParameter<bool> alwaysClockwise;
    ofParameter<bool> smoothHomePosition;
    ofParameter<bool> laserOnWhileMoving = false;
 
    MaskManager maskManager;

    
    bool guiInitialised = false;
    
    float lastSaveTime = 0; 
    float smoothedFrameRate = 0;
    int frameTimeHistorySize = 200;
    float frameTimeHistory[200];
    int frameTimeHistoryOffset = 0;
    bool ignoreParamChange = false;
    
    // for the ui representation
    glm::vec2 previewOffset;
    float previewScale;
    bool previewDragging;
    glm::vec2 dragStartPoint;
  
    // parent laser manager is in control of this
    bool snapToGrid;
    int gridSize;
    ofMesh gridMesh; 
  
    ofMesh previewPathMesh;
    vector<ofColor> previewPathColours;
   
    
    
    //-----------------------------------
    protected :
  
    
    void setDacArmed(bool& armed);
    
    DacEmpty emptyDac;
 

    DacBase* dac;
    
    //const int min = -32768;
    //const int max = 32767;
    
    ofPoint laserHomePosition;
    
     
    vector<Point> laserPoints;
    vector<Point> sparePoints;
    vector<Point> sparePoints2;
    unsigned long frameCounter = 0;
    
    int numPoints;
    ofEventListener paramsChangedListener;

    
};

}


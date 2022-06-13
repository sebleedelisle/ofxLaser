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

#include "constants.h"
#include "ofxLaserPointsForShape.h"
#include "ofxLaserDacBase.h"
#include "ofxLaserDacEmpty.h"
#include "ofxLaserZone.h"
#include "ofxLaserLaserZone.h"
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
    bool loadSettings(vector<Zone*>& zones);
    bool saveSettings();
    bool getSaveStatus();
    
    
    void update(bool updateZones);
    void send( float masterIntensity = 1, ofPixels* pixelmask = NULL);
    
    bool toggleArmed(); 
   
    // adds all the shape points to the vector passed in
    void getAllShapePoints(vector<PointsForShape>* allzoneshapepoints, ofPixels*pixels, float speedmultiplier);

    
    void sendRawPoints(const vector<Point>& points, Zone* zone, float masterIntensity =1);
    int getPointRate();
    float getFrameRate();
    
    // DAC
    
    string getDacLabel() ;
    int getDacConnectedState();
    
    // Zones
    
    void addZone(Zone* zone, float srcwidth, float srcheight);
    bool hasZone(Zone* zone);
    bool removeZone(Zone* zone);
    bool muteZone(int zonenum);
    bool unMuteZone(int zonenum); 
    
    
    vector<LaserZone*>getActiveZones();
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
   
    LaserZone* getLaserZoneForZone(Zone* zone);
    
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
    deque<Shape*> getTestPatternShapesForZone(LaserZone& zone);
    float getMoveDistanceForShapes(vector<PointsForShape>& shapes);
    float getMoveDistanceForShapes(vector<PointsForShape*>& shapes);


    //-----------------------------------------------
    
    int laserIndex;
    vector<LaserZone*> laserZones;
    
    string savePath ="ofxLaser/lasers/";
    
    ofParameterGroup params;
    ofParameter<bool> armed;
    ofParameter<int> pps;
    ofParameter<float> speedMultiplier;
    ofParameter<float>intensity;
    
    ofParameter<bool> paused; // pause frame
    bool pauseStateRecorded;
    map<Zone*, deque<Shape*>> pauseShapesByZone;
    
    // used to keep track of the dac that we're connected to
    // (particularly when loading / saving)
    ofParameter<string> dacId;
    
    ofParameter<float> colourChangeShift;
    ofParameter<int> maxLatencyMS; 
    
    ofParameter<int> testPattern;
    ofParameter<bool>hideContentDuringTestPattern;
 
    int numTestPatterns;
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


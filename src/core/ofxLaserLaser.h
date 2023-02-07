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
    
    void update(bool updateZones);
    void send( float masterIntensity = 1, ofPixels* pixelmask = NULL);
    
    bool toggleArmed(); 
   
    // adds all the shape points to the vector passed in
    void getAllShapePoints(vector<PointsForShape>* allzoneshapepoints, ofPixels*pixels, float speedmultiplier);

    void sendRawPoints(const vector<Point>& points, InputZone* zone, float masterIntensity =1);
    int getPointRate();
    float getFrameRate();
    
    void clearPoints();
    
    // DAC
    string getDacLabel() ;
    int getDacConnectedState();
    
    // Zones
    void addZone(InputZone* zone,  bool isAlternate = false);
    bool hasZone(InputZone* zone);
    bool removeZone(InputZone* zone);
    bool removeZone(OutputZone* zone);
    
    bool muteZone(int zonenum);
    bool unMuteZone(int zonenum);
    bool soloZone(int zonenum);
    bool unSoloZone(int zonenum);
    bool isLaserZoneActive(OutputZone* outputZone);

    // gets output zones (but not alt zones) in order
    vector<OutputZone*> getSortedOutputZones();
    vector<OutputZone*> getSortedOutputAltZones();
   
    // Alternative zones
    void addAltZone(InputZone* zone);
    void addAltZone(int zoneIndex);
    bool hasAltZone(InputZone* zone);
    bool hasAltZone(int zoneIndex);

    bool removeAltZone(InputZone* zone);
    bool removeAltZone(OutputZone* zone);
    bool removeAltZone(int zoneIndex);
    
    bool hasAnyAltZones();
    
    OutputZone* getLaserZoneForZone(InputZone* zone);
    InputZone* getLaserInputZoneForZoneIndex(int zoneIndex); // bit nasty
    OutputZone* getLaserAltZoneForZone(InputZone* zone);
    OutputZone* getLaserAltZoneForZone(int zoneIndex);

    vector<OutputZone*>getActiveZones();
    bool areAnyZonesSoloed();
    void clearOutputZones();

    vector<Point>& getLaserPoints() { return laserPoints;}; 
   
   
    // Managing points
    void addPoint(ofxLaser::Point p);
    void addPoint(ofPoint p, ofFloatColor c, bool useCalibration = true);
    void addPoints(vector<ofxLaser::Point>&points, bool reversed = false);

    void addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint);
    void processPoints(float masterIntensity, bool offsetColours = true);
    
    RenderProfile& getRenderProfile(string profilelabel);
    
    //void updateZoneMasks();
    
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
  
    ofMesh previewPathMesh;
    ofMesh previewPathColoured;

    DacEmpty emptyDac;
 
    //-----------------------------------
    protected :
    
    map<string, string> laserZonesLastSavedMap;
  
    void setDacArmed(bool& armed);

    DacBase* dac;
    
    ofPoint laserHomePosition;
     
    vector<Point> laserPoints;
    vector<Point> sparePoints;
    vector<Point> sparePoints2;
    unsigned long frameCounter = 0;
    
    int numPoints;
    ofEventListener paramsChangedListener;

    
};

}


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
#include "ofxLaserDacNetworkBaseThreaded.h"
#include "ofxLaserDacEmpty.h"
#include "ofxLaserOutputZone.h"
#include "ofxLaserManualShape.h"
#include "PennerEasing.h"
#include "ofxLaserMaskManager.h"
#include "ofxLaserScannerSettings.h"

#include "ofxLaserColourSettings.h"

#include "ofxLaserZoneContent.h"
#include "ofxLaserTestPatternGenerator.h"
#include "ofxLaserObjectWithZoneId.h"
#include "SebUtils.h"

namespace ofxLaser {

class Laser {
    
    public :
    
    Laser(int _index);
    ~Laser();
    
    void init();
    void reset();
    
    string getLabel();
    void setDac(std::shared_ptr<DacBase> dac);
    std::shared_ptr<DacBase> getDac();
    bool removeDac();
    
    bool hasDac(); 
    void setDacDiagnostics(bool state);
    
    void paramsChanged(ofAbstractParameter& e);
    bool loadSettings();
    bool saveSettings();
    bool getSaveStatus();
    
    string getFilenameForZone(std::shared_ptr<OutputZone>& outputZone);
    bool deleteSettingsFileForZone(std::shared_ptr<OutputZone>& outputZone);
    void deleteAllSettingsFiles();
    void deleteOldZoneFiles();
    
    void serialize(ofJson& json);
    bool deserialize(ofJson& json);
    
    void update();
    void send(const vector<ZoneContent>& zonesContent, float masterIntensity = 1, ofPixels* pixelmask = NULL);
    
    bool toggleArmed(); 
   
    // adds all the shape points to the vector passed in
    void getAllShapePoints(const vector<ZoneContent>& zonesContent, vector<PointsForShape>* allzoneshapepoints, ofPixels*pixels, float speedmultiplier);

    void sendRawPoints(const vector<Point>& points, ZoneId* zoneId = nullptr, float masterIntensity =1);
    int getPointRate();
    float getFrameRate();
    
    void clearPoints();
    
    // DAC
    string getDacLabel() ;
    int getDacConnectedState();
    
    void setGlobalTestPattern(bool active, int pattern); 
    
    // Zones
    void addZone(ZoneId zoneId,  bool isAlternate = false);
    bool removeZone(ZoneId zoneId);
    bool removeZone(std::shared_ptr<OutputZone>& zone);
    bool hasZone(ZoneId zoneId);
    
    void addAltZone(ZoneId zoneId);
    bool removeAltZone(std::shared_ptr<OutputZone>& zone);
    bool removeAltZone(ZoneId zoneId);
    bool hasAltZone(ZoneId zoneId);
    bool hasAnyAltZones();
    
    bool muteZone(ZoneId zoneId);
    bool unMuteZone(ZoneId zoneId);
    bool soloZone(ZoneId zoneId);
    bool unSoloZone(ZoneId zoneId);
    bool isLaserZoneActive(std::shared_ptr<OutputZone>& outputZone);
    
    bool updateZones(map<ZoneId, ZoneId>& changedZones);
    bool updateZoneLabels(vector<std::shared_ptr<ObjectWithZoneId>>& zoneids);

    // gets output zones (but not alt zones) in order
    vector<std::shared_ptr<OutputZone>> getSortedOutputZones();
    vector<std::shared_ptr<OutputZone>> getSortedOutputAltZones();
   
    
    std::shared_ptr<OutputZone> getLaserZoneForZoneId(ZoneId zoneId);
    //InputZone* getLaserInputZoneForZoneIndex(int zoneIndex); // bit nasty
    //OutputZone* getLaserAltZoneForZone(InputZone* zone);
    std::shared_ptr<OutputZone> getLaserAltZoneForZoneId(ZoneId zoneId);
    const int findZoneContentIndexForId(ZoneId zoneId,const vector<ZoneContent>& zonesContent );
    bool hasZoneContentForId(ZoneId zoneId,const vector<ZoneContent>& zonesContent );
 
    vector<std::shared_ptr<OutputZone>>getActiveZones();
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

    float getMoveDistanceForShapes(vector<PointsForShape>& shapes);
    float getMoveDistanceForShapes(vector<PointsForShape*>& shapes);


    //-----------------------------------------------
    
    int laserIndex;
    
    // LaserZone stores a reference to the source zone,
    // and it
    vector<std::shared_ptr<OutputZone>> outputZones;
    
    string savePath ="ofxLaser/lasers/";
    
    ofParameterGroup params;
    ofParameter<bool> armed;
    ofParameter<int> pps;
    ofParameter<float> speedMultiplier;
    ofParameter<float>intensity;
    
    ofParameter<bool> paused; // pause frame
    bool pauseStateRecorded;
    map<string, vector<std::shared_ptr<Shape>>> pauseShapesByZoneUid;
    
    // used to keep track of the dac that we're connected to
    // (particularly when loading / saving)
    ofParameter<string> dacLabel;
    // Keep track of the dac alias, even when it's not connected.
    // The ID is unique so should be used to reference the DAC.
    // The alias can be anything. 
    //ofParameter<string> dacAlias;
 
    ofParameter<float> colourChangeShift;
    int maxLatencyMS; 
    
    int testPattern;
    bool testPatternActive;
    
    int testPatternGlobal;
    int testPatternGlobalActive;

    ofParameter<bool>hideContentDuringTestPattern;
 
    //int numTestPatterns;
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
    float frameTimeHistory[201] ={0};
    int frameTimeHistoryOffset = 0;
    bool ignoreParamChange = false;
  
    ofMesh previewPathMesh;
    ofMesh previewPathColoured;

    std::shared_ptr<DacEmpty> emptyDac;
 
    //-----------------------------------
    protected :
    
    map<string, string> laserZonesLastSavedMap;
  
    void setDacArmed(bool& armed);

    std::shared_ptr<DacBase> dac;
    
    ofPoint laserHomePosition;
     
    vector<Point> laserPoints;
    vector<Point> sparePoints;
    vector<Point> sparePoints2;
    unsigned long frameCounter = 0;
    
    int numPoints;
    ofEventListener paramsChangedListener;

    
};

}


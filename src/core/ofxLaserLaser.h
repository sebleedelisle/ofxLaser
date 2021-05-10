//
//  ofxLaserLaser.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//
#pragma once

#include "constants.h"
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
    
    void setDefaultHandleSize(float size);
    
    void update(bool updateZones);
    void send(ofPixels* pixels = NULL, float masterIntensity = 1);
    
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
    void drawLaserPath(ofRectangle rect, bool drawDots = true, float radius = 4);
    void drawLaserPath(float x=0, float y=0, float w=800, float h=800, bool drawDots = true, float radius = 4);
    void drawTransformUI(float x=0, float y=0, float w=800, float h=800);
    void disableTransformGui();
    void enableTransformGui();
    
    void drawTransformAndPath(ofRectangle rect); 

    
   
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
    
    deque<Shape*> getTestPatternShapesForZone(LaserZone& zone);
    


    //-----------------------------------------------
    
    int laserIndex;
    vector<LaserZone*> laserZones;
    
    string savePath ="ofxLaser/lasers/";
    
    ofParameterGroup params;
    ofParameter<bool> armed;
    ofParameter<int> pps;
    ofParameter<float> speedMultiplier;
    ofParameter<float>intensity;
    
    // used to keep track of the dac that we're connected to
    // (particularly when loading / saving)
    ofParameter<string> dacId;
    
    ofParameter<float> colourChangeShift;
    
    ofParameter<int> testPattern;
    ofParameter<bool>hideContentDuringTestPattern;
 
    int numTestPatterns;
    ofParameter<bool> flipX;
    ofParameter<bool> flipY;
    ofParameter<float> rotation;
    ofParameter<glm::vec2> outputOffset;
    
    ScannerSettings scannerSettings; 
    ColourSettings colourSettings; 
    // advanced settings
    ofParameterGroup advancedParams;
    ofParameter<float>targetFramerate;
    ofParameter<bool>syncToTargetFramerate;
    ofParameter<int>syncShift;
    ofParameter<bool>sortShapes;
    ofParameter<bool> smoothHomePosition;
    ofParameter<bool> laserOnWhileMoving = false;
 
    MaskManager maskManager;

    
    bool guiInitialised = false;
    
    float lastSaveTime = 0; 
    float smoothedFrameRate = 0;
    int frameTimeHistorySize = 200;
    float frameTimeHistory[200];
    int frameTimeHistoryOffset = 0;

    //-----------------------------------
    protected :
  
    void setDacArmed(bool& armed);
    
    DacEmpty emptyDac;

    
    float defaultHandleSize = 8;

    DacBase* dac;
    
    //const int min = -32768;
    //const int max = 32767;
    
    ofPoint laserHomePosition;
    
     
    vector<Point> laserPoints;
    vector<Point> sparePoints;
    vector<Point> sparePoints2;
    unsigned long frameCounter = 0;
    
    int numPoints;
    ofMesh previewPathMesh;
    ofEventListener paramsChangedListener;

    
};

}


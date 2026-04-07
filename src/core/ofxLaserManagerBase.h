//
//  ofxLaserManager.hpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserTransformationManager.h"
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
#include "ClipperUtils.h"
#include "SebUtils.h"
#include "ofxLaserState.h"
#include "ofxLaserMsg.h"
#include "ofxLaserBaseView.h"

namespace ofxLaser {


class ManagerBase : public TransformationManager {
    
    public :
    
    // it's a Singleton so shouldn't ever have more than one.
    static ManagerBase * instance();
    static ManagerBase * laserManager;
    
    ManagerBase();
    ~ManagerBase();
    
    virtual void resetAllLasersToDefault();
    
    virtual void update();
   
    virtual bool deleteLaser(std::shared_ptr<Laser>& laser);
    
    ZoneId addZone(float x = 0 , float y = 0, float w = -1, float h= -1);
    ZoneId addZone(const ofRectangle& zoneRect);
    virtual bool deleteZone(std::shared_ptr<InputZone> zone);

    void addZoneToLaser(ZoneId& zoneId, unsigned int lasernum);
    
    virtual void setCanvasSize(int width, int height);
    int getCanvasWidth() {
        return canvasTarget->getWidth();
    }
    int getCanvasHeight() {
        return canvasTarget->getHeight();
    } 
    
    bool loadSettings();
    bool saveSettings();
    bool scheduleSaveSettings();
    
    virtual void serialize(ofJson& json);
    virtual bool deserialize(ofJson& json);

protected:
    // Subclasses can persist extra manager-owned state alongside the shared
    // laser settings file without replacing the core save/load implementation.
    virtual void loadAdditionalSettings(const ofJson& json) {}
    virtual void saveAdditionalSettings(ofJson& json) const {}

public:
    
    virtual void send();
    void sendRawPoints(const std::vector<ofxLaser::Point>& points, int lasernum = 0, ZoneId* zoneId = nullptr);
    
    int getLaserPointRate(unsigned int lasernum = 0);
    float getLaserFrameRate(unsigned int lasernum);
    
    void armAllLasers();
    void disarmAllLasers();
    void updateGlobalTestPattern();
    void hideContentDuringTestPatternChanged(bool& state);
    
    void setStroke(bool strokestate) {
        strokeOn = strokestate;
    }
    void setFill(bool fillstate) {
        fillOn = fillstate; 
    }
        

    void drawDot(const glm::vec3& p, const ofColor& col, float intensity =1, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawDot(const glm::vec2& p, const ofColor& col, float intensity =1, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawDot(float x, float y, const ofColor& col, float intensity =1, string profileName = OFXLASER_PROFILE_DEFAULT);
 
    void drawLine(const glm::vec3& start, const glm::vec3& end, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawLine(const glm::vec2& start, const glm::vec2& end, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT);
    void drawLine(float x1, float y1, float x2, float y2, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT);
   
    void drawPoly(const ofPolyline &poly, const ofColor& col, string profileName = OFXLASER_PROFILE_DEFAULT, float brightness = 1);
    void drawPoly(const ofPolyline & poly, vector<ofColor>& colours, string profileName = OFXLASER_PROFILE_DEFAULT, float brightness =1);
    void drawPolyFromPoints(const vector<glm::vec3>& points, const vector<ofColor>& colours, bool closed = false, string profileName = OFXLASER_PROFILE_DEFAULT, float brightness =1);
  
    std::shared_ptr<ofxLaser::Polyline> getPolyFromPoints(const vector<glm::vec3>& points, const vector<ofColor>& colours, bool closed = false, string profileName = OFXLASER_PROFILE_DEFAULT, float brightness =1);
    
    void drawPolys(const vector<ofPolyline>& polys, vector<vector<ofColor>>&colours, string profileName = OFXLASER_PROFILE_DEFAULT, float brightness =1);
    void drawPolysFromPoints(const vector<vector<glm::vec3>>& allPoints, vector<vector<ofColor>>&allColours, string profileName, float brightness);
    // Pointer-based variant used by high-volume callers to avoid deep-copying
    // nested point/colour vectors before they enter ofxLaser.
    void drawPolysFromPointRefs(const vector<const vector<glm::vec3>*>& allPointRefs,
                                const vector<const vector<ofColor>*>& allColourRefs,
                                string profileName,
                                float brightness);
    
    void drawCircle(const float& x, const float& y, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
    void drawCircle(const glm::vec3& centre, const float& radius, const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
    void drawCircle(const glm::vec2& centre, const float& radius, const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
   
    void drawLaserGraphic(Graphic& graphic, float brightness = 1, string renderProfile = OFXLASER_PROFILE_DEFAULT);
    
    
    vector<std::shared_ptr<Laser>>& getLasers();
    std::shared_ptr<Laser>& getLaser(int index = 0);
    int getNumLasers() { return (int)lasers.size(); };
    int getNumZones() { return canvasTarget->getNumZoneIds(); }
    
    
    bool isLaserArmed(unsigned int i);
	bool areAllLasersArmed();
    
    void beginDraw() {
        // to do : check target
        ofViewport((ofGetWidth()-canvasTarget->getWidth())/-2, (ofGetHeight()-canvasTarget->getHeight())/-2, ofGetWidth(), ofGetHeight()) ;
        ofPushMatrix();
        ofTranslate((ofGetWidth()-canvasTarget->getWidth())/2, (ofGetHeight()-canvasTarget->getHeight())/2);
        
    }
    void endDraw() {
        ofPopMatrix();
        ofViewport(0,0,ofGetWidth(), ofGetHeight());
    }
    
    
    int getNextId();

    // ============================================================
    // View registration (Step 4 — Liberation-style signals)
    // ============================================================
    void addLaserView(LaserBaseView* view);
    bool removeLaserView(LaserBaseView* view);

    // ============================================================
    // Message bus receiver (Step 2 — Liberation-style messages)
    // ============================================================
    void receiveLaserMessage(LaserMsgEnvelope& env);

    // ============================================================
    // Read-only state snapshot (Step 1 — Liberation-style AppState)
    // ============================================================
    const LaserState& getLaserState() const { return laserState; }

    //--------------------------------------------------------

    DacAssigner& dacAssigner;
    
    int testPatternGlobal;
    bool testPatternGlobalActive;
    
    ofParameter<int> numLasers; // << not used except for load / save
    
    ofParameter<bool> dontCalculateDisconnected;
    ofParameter<bool> hideContentDuringTestPattern;
    
    ofParameter<float>globalBrightness;

    BitmapMaskManager laserMask;
    
                
    ofParameterGroup params;
 
    ofJson loadedJson;
    
    bool useClipRectangle = false;
    ofRectangle clipRectangle;
  
    
    protected :

    // ============================================================
    // LaserState — read-only snapshot for views
    // ============================================================
    LaserState laserState;
    void syncLaserState();

    // ============================================================
    // Registered views
    // ============================================================
    std::vector<LaserBaseView*> laserViews;

    // ============================================================
    // Deferred signal queue — fire*() sets a bit, flushPendingSignals()
    // delivers them at the start of the next update().
    // This eliminates re-entrancy and intermediate-state risks.
    // ============================================================
    enum SignalFlag : uint32_t {
        SIG_LASERS          = 1 << 0,
        SIG_ZONES           = 1 << 1,
        SIG_CANVAS          = 1 << 2,
        SIG_DAC_STATUS      = 1 << 3,
        SIG_GLOBAL_SETTINGS = 1 << 4,
        SIG_TEST_PATTERN    = 1 << 5,
        SIG_MASKS           = 1 << 6,
    };
    uint32_t pendingSignals = 0;

    void fireLasersChanged();
    void fireZonesChanged();
    void fireCanvasChanged();
    void fireDacStatusChanged();
    void fireGlobalSettingsChanged();
    void fireTestPatternChanged();
    void fireMasksChanged();

    void flushPendingSignals();

    int currentId = 0;

    virtual void createAndAddLaser();
    
    void createDefaultZone();
    
    std::vector<std::shared_ptr<Laser>> lasers;
    
    std::shared_ptr<ShapeTargetCanvas> canvasTarget;
    
      
    // used in "drawPoly" functions
    // to avoid generating polyline objects
    ofPolyline tmpPoly;
    vector<glm::vec3> tmpPoints;
  
    ofSoundPlayer beepSound;
    bool settingsNeedSave = false;
    float lastSaveTime = 0;
    
    bool strokeOn = true;
    bool fillOn = false;
    
    private:
};
}

//
//  ofxLaserManager.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserManagerBase.h"

using namespace ofxLaser;

ManagerBase * ManagerBase :: laserManager = nullptr;

ManagerBase * ManagerBase::instance() {
    if(laserManager == nullptr) {
        laserManager = new ManagerBase();
    }
    return laserManager;
}


ManagerBase :: ManagerBase()
    : dacAssigner(*DacAssigner::instance())
    , laserState{ lasers, canvasTarget, dacAssigner, laserMask }
{


    canvasTarget = std::make_shared<ShapeTargetCanvas>();
    
    if(laserManager == nullptr) {
        laserManager = this;
    } else {
        ofLog(OF_LOG_ERROR, "Multiple ofxLaser::Manager instances created");
    }
    
    setCanvasSize(800,800);
    
    params.setName("Laser");
    params.add(globalBrightness.set("Global brightness", 0.2,0,1));

    params.add(numLasers.set("numLasers", 0));
    params.add(dontCalculateDisconnected.set("Don't calculate disconnected", false));
    params.add(hideContentDuringTestPattern.set("Hide content during test pattern", true));
    
    testPatternGlobal = 1;
    testPatternGlobalActive = false; 

    useClipRectangle = false;
    
    ClipperUtils::initialise();
    
}

ManagerBase :: ~ManagerBase() {

    saveSettings();
    for(auto& laser : lasers) {
        laser->getDac()->close();
        //laser->removeDac();
    }
    lasers.clear();
    DacAssigner::destroy(); 
    
}

//
//
void ManagerBase ::resetAllLasersToDefault() {
    vector<string> dacLabels;
    for(std::shared_ptr<Laser>& laser : lasers) {
        dacLabels.push_back(laser->dacLabel.get());
    }
    
    int numlasers = getNumLasers();
    while(lasers.size()>0) {
        deleteLaser(lasers[0]);
    }
    canvasTarget->clearZones();
    
    for(int lasernum = 0; lasernum<numlasers; lasernum++) {
        createAndAddLaser();

        dacAssigner.assignToLaser(dacLabels[lasernum], getLaser(lasernum));
    }
    
    saveSettings();
    
}


void ManagerBase :: setCanvasSize(int w, int h){
    canvasTarget->setBounds(0,0,w,h);
}

void ManagerBase::createAndAddLaser() {
    
    // create and add new laser object
    int newlaserindex = lasers.size();
//    Laser* laser = new Laser(lasers.size());
    lasers.emplace_back(std::make_shared<Laser>(newlaserindex)); // should make a new shared pointer maybe?
    
    
    // TODO should this be here?
    lasers.back()->init();

    
}



bool ManagerBase :: deleteLaser(std::shared_ptr<Laser>& laser) {
    
    bool deleteZones = true;
    
    // check if laser exists and isn't null
    if(laser == nullptr) return false;
    
    if(!SebUtils::elementInVector(lasers, laser)) return false;

    // disconnect dac
    dacAssigner.disconnectDacFromLaser(laser);
    
    laser->deleteAllSettingsFiles();
    // Remove all output zones from this laser
    if(deleteZones) {
        laser->clearOutputZones();
    }
    
    // remove laser from laser array
    SebUtils::removeElementFromVector(lasers, laser);
    //lasers.erase(it);

    // delete laser object
    //delete laser;
    
    //  delete laser settings files
    ofDirectory::removeDirectory("lasers/", true);
    
    // re-save remaining laser
    // TODO - Do we need to do that ?
    for(int i = 0; i<(int)lasers.size(); i++) {
        
        lasers[i]->laserIndex = i;
        lasers[i]->saveSettings();
        scheduleSaveSettings(); 
    }
    
    
    
    return true;
}

ZoneId ManagerBase::addZone(const ofRectangle& rect) {
    return addZone(rect.x, rect.y, rect.width, rect.height);

}

ZoneId  ManagerBase :: addZone(float x, float y, float w, float h) {
    if(w<=0) w = canvasTarget->getWidth();
    if(h<=0) h = canvasTarget->getHeight();
    return canvasTarget->addInputZone(x, y, w, h);
}


bool ManagerBase :: deleteZone(std::shared_ptr<InputZone> inputZone) {
    if(inputZone==nullptr) return false;
    
    map<ZoneId, ZoneId>  changedzones = canvasTarget->removeZoneById(inputZone->getZoneId());
    for(std::shared_ptr<Laser>& laser : lasers) {
        laser->updateZones(changedzones);
        laser->removeZone(inputZone->getZoneId());
    }
    return true;
    
    
}

void ManagerBase::addZoneToLaser(ZoneId& zoneId, unsigned int lasernum) {
    if(lasers.size()<=lasernum) {
        ofLog(OF_LOG_ERROR, "Invalid laser number passed to addZoneToLaser(...)");
        return;
    }
    // Todo - check zone exists
    lasers[lasernum]->addZone(zoneId);
}

void ManagerBase::createDefaultZone() {
    // check there aren't any zones yet?
    // create a zone equal to the width and height of the total output space
    addZone(0,0,canvasTarget->getWidth(),canvasTarget->getHeight());
    
}


void ManagerBase::drawDot( float x,  float y, const ofColor& col, float intensity, string profileLabel) {
    drawDot(glm::vec3(x, y, 0), col, intensity, profileLabel);
}
void ManagerBase::drawDot( const glm::vec2& p, const ofColor& col, float intensity, string profileLabel) {
    drawDot(glm::vec3(p.x, p.y, 0), col, intensity, profileLabel);
}


void ManagerBase::drawDot(const glm::vec3& p, const ofColor& col, float intensity, string profileLabel) {
    
    std::shared_ptr<Dot> d = std::make_shared<Dot>(getTransformed(p), col, intensity, profileLabel);
    canvasTarget->addShape(d, useClipRectangle, clipRectangle);
}



void ManagerBase::drawLine(float x1, float y1, float x2, float y2, const ofColor& col, string profileName){
    drawLine( glm::vec3(x1,y1,0), glm::vec3(x2,y2,0), col, profileName);
}

void ManagerBase::drawLine(const glm::vec2& start, const glm::vec2& end, const ofColor& col, string profileName){
    drawLine( glm::vec3(start.x, start.y, 0), glm::vec3(end.x, end.y, 0), col, profileName);
}

void ManagerBase::drawLine(const glm::vec3& start, const glm::vec3& end, const ofColor& col, string profileLabel) {
    
    std::shared_ptr<Line> l = std::make_shared<Line>(getTransformed(start), getTransformed(end), col, profileLabel);
    
    canvasTarget->addShape(l, useClipRectangle, clipRectangle);

}





void ManagerBase::drawCircle(const float& x, const float& y, const float& radius, const ofColor& col, string profileName){
    drawCircle(glm::vec3(x, y, 0), radius, col, profileName);
}
void ManagerBase::drawCircle(const glm::vec2& pos, const float& radius, const ofColor& col, string profileName){
    drawCircle(glm::vec3(pos.x, pos.y, 0), radius, col, profileName);
}
void ManagerBase::drawCircle(const glm::vec3 & centre, const float& radius, const ofColor& col,  string profileName){
     std::shared_ptr<Circle> c = std::make_shared<Circle>(centre, radius, col, profileName);
    
    c->setFilled(fillOn);
    c->setStroked(strokeOn);
    vector<glm::vec3>& points = c->getPoints();
    for(glm::vec3& v : points) {
        v = getTransformed(v);
    }
    c->setDirty();
    
    canvasTarget->addShape(c, useClipRectangle, clipRectangle);
    
}


void ManagerBase::drawPoly(const ofPolyline & poly, std::vector<ofColor>& colours, string profileName, float brightness){
    
    drawPolyFromPoints(poly.getVertices(), colours, poly.isClosed(), profileName, brightness);
    
}



void ManagerBase::drawPoly(const ofPolyline & poly, const ofColor& col, string profileName, float brightness){
    std::vector<ofColor> colours = {col};
    drawPoly(poly, colours, profileName, brightness);
    
    
}

void ManagerBase::drawPolyFromPoints(const vector<glm::vec3>& points, const vector<ofColor>& colours, bool closed, string profileName, float brightness){
    
    if(points.size()==0) return;
   
    
    std::shared_ptr<Polyline> p  = getPolyFromPoints(points, colours, closed, profileName, brightness);
        
    if(p->getLength()>0.1) {
        canvasTarget->addShape(p, useClipRectangle, clipRectangle);
    } else {
        // shape should get destroyed
    }
}

std::shared_ptr<ofxLaser::Polyline> ManagerBase::getPolyFromPoints(const vector<glm::vec3>& points, const vector<ofColor>& colours, bool closed, string profileName, float brightness){
    
    tmpPoints = points;
    bool allOnSameHorizontal = true;
    bool increasingLeftToRight = true;

    float yRef = 0.0f;
    const float eps = 1e-3f; // tolerance for float comparisons
    bool first = true;
    float prevX = 0.0f;

    for (glm::vec3& v : tmpPoints) {
        v = getTransformed(v);

        // some real nasty stuff here to get around what I think is a bug in the
        // clipping code which seems to break with horizontal lines from left to right
        // If we detect this, we just reverse the line.
        if (first) {
            yRef = v.y;
            prevX = v.x;
            first = false;
            continue;
        }

        // a) check horizontal alignment
        if (fabs(v.y - yRef) > eps) {
            allOnSameHorizontal = false;
        }

        // b) check left-to-right ordering
        if (v.x >= prevX + eps) {
            increasingLeftToRight = false;
        }

        prevX = v.x;
    }
    
    std::shared_ptr<Polyline> p;
    
    if(allOnSameHorizontal && increasingLeftToRight) {
        std::reverse(tmpPoints.begin(), tmpPoints.end());
        auto reversed = colours;
        std::reverse(reversed.begin(), reversed.end());
        p = std::make_shared<Polyline>(tmpPoints, reversed, profileName, brightness);
    } else {
        p = std::make_shared<Polyline>(tmpPoints, colours, profileName, brightness);
    }
    
    
    //ofxLaser::Polyline* p =new ofxLaser::Polyline(tmpPoints, colours, profileName, brightness);
    
    p->setFilled(fillOn);
    p->setStroked(strokeOn);

    p->setClosed(closed);
    return p;
    
}



void ManagerBase::drawPolys(const vector<ofPolyline>& polys, vector<vector<ofColor>>&colours, string profileName, float brightness) {
    
    if(polys.size()!=colours.size()) {
        ofLogError("ManagerBase::drawPolys - mismatched polys / colours lengths");
        return ;
    }
    
    int id = getNextId();
    for(int i = 0; i<polys.size() ; i++ ) {
        const ofPolyline& ofpolyline = polys[i];
        std::shared_ptr<Polyline> poly = getPolyFromPoints(ofpolyline.getVertices(), colours[i], ofpolyline.isClosed(), profileName, brightness);
        poly->id = id;
        if(poly->getLength()>0.1) {
            canvasTarget->addShape(poly, useClipRectangle, clipRectangle);
        } else {
           // shape should get destroyed
        }
    }
    
}

void ManagerBase::drawPolysFromPoints(const vector<vector<glm::vec3>>& allPoints, vector<vector<ofColor>>&allColours, string profileName, float brightness) {
    
    if(allPoints.size()!=allColours.size()) {
        ofLogError("ManagerBase::drawPolysFromPoints - mismatched polys / colours lengths");
        return ;
    }
    
    // TODO - vector of closed!
    bool closed = false;
    
    int id = getNextId();
    for(int i = 0; i<allPoints.size() ; i++ ) {
        
        std::shared_ptr<Polyline> poly = getPolyFromPoints(allPoints[i], allColours[i], closed, profileName, brightness);
        poly->id = id;
        if(poly->getLength()>0.1) {
            canvasTarget->addShape(poly, useClipRectangle, clipRectangle);
        } else {
            // should get destroyed
        }
    }
    
    
    //ofxLaser::Polylines* polylines = new ofxLaser::Polylines();
    //TODO ******
    
}

void ManagerBase::drawPolysFromPointRefs(const vector<const vector<glm::vec3>*>& allPointRefs,
                                         const vector<const vector<ofColor>*>& allColourRefs,
                                         string profileName,
                                         float brightness) {
    if(allPointRefs.size() != allColourRefs.size()) {
        ofLogError("ManagerBase::drawPolysFromPointRefs - mismatched polys / colours lengths");
        return;
    }

    // TODO - vector of closed!
    const bool closed = false;
    const int id = getNextId();

    for(size_t i = 0; i < allPointRefs.size(); i++) {
        const vector<glm::vec3>* points = allPointRefs[i];
        const vector<ofColor>* colours = allColourRefs[i];
        if(points == nullptr || colours == nullptr || points->empty() || colours->empty()) {
            continue;
        }

        // This uses the existing poly conversion/render path, but avoids creating
        // temporary deep-copied vector<vector<...>> wrappers at the call site.
        std::shared_ptr<Polyline> poly = getPolyFromPoints(*points, *colours, closed, profileName, brightness);
        poly->id = id;
        if(poly->getLength() > 0.1f) {
            canvasTarget->addShape(poly, useClipRectangle, clipRectangle);
        }
    }
}



void ManagerBase::drawLaserGraphic(Graphic& graphic, float brightness, string renderProfile) {
    
    auto & polylines = graphic.polylines;
    auto & colours = graphic.colours;
    
    for(size_t i= 0; i<polylines.size(); i++) {
        ofColor col = colours[i];
        col*=brightness;
        drawPoly(*polylines[i],col, renderProfile);
        
    }
    
}

void ManagerBase:: update(){

    // Flush any deferred signals from the previous frame's message handlers.
    flushPendingSignals();

    // bit of a hack to check dacs that may take a little while to appear
    if(ofGetFrameNum()==1000) {
        dacAssigner.updateDacList();
    }

    // resets transformations
    resetTransformations();
    
    dacAssigner.update();
    
    // delete all the shapes - all shape objects need a destructor!
    canvasTarget->deleteShapes();
    
    // updates all the zones. If zone->update returns true, then
    // it means that the zone has changed.
    bool updateZoneRects = false;

    bool dacDisconnected = false;
    // update all the lasers which clears the points,
    // and updates all the zone settings
    for(std::shared_ptr<Laser>& laser : lasers){
        laser->update(); // clears the points
        if(laser->hasDac()) {
            int laserstatus = laser->getDac()->getStatus();
            
            if(laser->getDac()->hasStatusChanged() && (laserstatus!=OFXLASER_DACSTATUS_GOOD)) {
                // MAKE BEEP
                dacDisconnected = true;
            }
        }
    }
    if(dacDisconnected)  {
    }
     
    if(settingsNeedSave && (ofGetElapsedTimef()-lastSaveTime>1)) {
        saveSettings();
    }
    fillOn = false;
    strokeOn = true;
    
    currentId = 0;
    
}
int ManagerBase::getNextId() {
    int id = currentId;
    currentId++;
    return id;
    
}


void ManagerBase::send(){
    
    canvasTarget->processShapes();
    
    // here's where the magic happens.
    // 1 :
    // figure out which zones to send the shapes to
    // and send them. When the zones get the shape, they transform them
    // into local zone space.
    
    vector<ZoneContent> zonesContent;
    vector<std::shared_ptr<ObjectWithZoneId>>& zoneIds = canvasTarget->getZoneIds();
    
    
    // NEW ALGORITHM

    // from now on, add the shapes to the current zone as it's being drawn.
    // Current zone can be a beam zone or a canvas.
    // Beam zones / canvas stores a bunch of shapes.
    // This code below for the logic of how to send shapes to zones
    // is only relevant for the canvas.
    
    // is the ZoneContent object still relevant? Maybe!
    // So now we need to go through the ZoneTargets and convert them to ZoneContents.
    // The question is, how do we identify the zones so that the lasers know which
    // zones belong to them. Previously it was just a simple index number, but now
    // that won't work for canvas zones.
    //
    // How much do the lasers need to know about
    // the zones?
    // All they need is a UID right?
    // For beam zones, it's just an index (although this could change so need
    // a way to reorg)
    // For canvas zones, it's a canvas index then a canvas zone index I think
    
    
    for(std::shared_ptr<ObjectWithZoneId>& zoneIdObject : zoneIds) {
        std::shared_ptr<InputZone> inputZone = canvasTarget->getInputZoneForZoneId(zoneIdObject->zoneId);

        zonesContent.push_back(ZoneContent());
        ZoneContent& zoneContent = zonesContent.back();
        vector<std::shared_ptr<Shape>>& newshapes = zoneContent.shapes;
        zoneContent.zoneId = inputZone->getZoneId();
        zoneContent.sourceRectangle = inputZone->getRect();
        zoneContent.shapes = canvasTarget->getShapesForZoneId(inputZone->getZoneId());

    }

    // 2 :
    // The lasers go through each of their zones, and pull out each shape
    // it'd need to be in zone space, then as each shape is converted to points, that's
    // when we'd do the warp for the output space.
    
    // So - the shapes need to be sorted in output space but their points need to be
    // calculated at zone space. Otherwise the perspective distortion won't look right in
    // terms of brightness distribution.
    for(size_t i= 0; i<lasers.size(); i++) {
        
        Laser& laser = *lasers[i];
        
        laser.send(zonesContent, globalBrightness, nullptr);
        
        std::this_thread::yield();
        
    }
}


int ManagerBase :: getLaserPointRate(unsigned int lasernum ){
    if(lasernum>=lasers.size()) return -1;
    else return lasers.at(lasernum)->getPointRate();
}

float ManagerBase :: getLaserFrameRate(unsigned int lasernum ){
    if((lasernum>=0) && (lasernum<lasers.size())) {
        return lasers.at(lasernum)->getFrameRate();
    } else return 0;
}
void ManagerBase::sendRawPoints(const std::vector<ofxLaser::Point>& points, int lasernum, ZoneId* zoneId ){
     ofLog(OF_LOG_NOTICE, "ofxLaser::Manager::sendRawPoints(...) point count : "+ofToString(points.size()));
    if(lasernum>=lasers.size()) {
        ofLogError("Invalid laser number sent to ofxLaser::ManagerBase::sendRawPoints");
        return;
    }
    std::shared_ptr<Laser>& laser = lasers.at(lasernum);
    laser->sendRawPoints(points, zoneId, globalBrightness);
    
}


void ManagerBase::armAllLasers() {
    
    for(size_t i= 0; i<lasers.size(); i++) {
        lasers[i]->armed = true;
    } 
   
}
void ManagerBase::disarmAllLasers(){
    for(size_t i= 0; i<lasers.size(); i++) {
        lasers[i]->armed = false;
    }
}
void ManagerBase::updateGlobalTestPattern(){
    for(size_t i= 0; i<lasers.size(); i++) {
        lasers[i]->setGlobalTestPattern(testPatternGlobalActive, testPatternGlobal);
    }
}
void ManagerBase::hideContentDuringTestPatternChanged(bool& state) {
    
    for(std::shared_ptr<Laser>& laser : lasers) {
        laser->hideContentDuringTestPattern = hideContentDuringTestPattern;
    }
    saveSettings();
    
}

bool ManagerBase::loadSettings() {
    
    ofJson& json = loadedJson;
    const string filename ="ofxLaser/laserSettings.json";
    // General strategy:
    // settings I/O must never crash the render/update thread. On some launch
    // paths the process working directory can become invalid, which makes
    // ofToDataPath() throw from inside ofFile/ofLoadJson.
    try {
        if(ofFile(filename).exists()) {
            json = ofLoadJson(filename);
        }
    } catch(const std::exception& e) {
        ofLogError("ManagerBase::loadSettings") << "failed to read " << filename << " : " << e.what();
        json = ofJson{};
    } catch(...) {
        ofLogError("ManagerBase::loadSettings") << "failed to read " << filename << " : unknown exception";
        json = ofJson{};
    }
    // if the json didn't load then this shouldn't do anything
    ofDeserialize(json, params);
  
    if(json.contains("dacassigner")) {
        dacAssigner.deserialize(json["dacassigner"]);
    }
    
    
    canvasTarget->deserialize(json["canvastarget"]);
    loadAdditionalSettings(json);
    
    
    // reset the global brightness setting, despite what was in the settings.
    globalBrightness = 0.2;

    // NOW load the lasers
    
    // numLasers was saved in the json
    for(int i = 0; i<numLasers; i++) {
        
        // if we don't have a laser object already make one
        if(lasers.size()<i+1) {
            createAndAddLaser();
        } else {
            // if we already have a laser then make sure no dac is connected
            dacAssigner.disconnectDacFromLaser(lasers[i]);
        }
        std::shared_ptr<Laser>& laser = lasers[i];
        laser->loadSettings();
        
        
        // if the laser has a dac id saved in the settings,
        // tell the dacAssigner about it
        // if the dac isn't available, it'll make the data and store it
        // ready for when it loads
        if(!laser->dacLabel->empty()) {
            dacAssigner.assignToLaser(laser->dacLabel, laser);
        }
        
    }
    // if we had more lasers to start with than we needed, then resize
    // the vector (shouldn't be needed but it doesn't hurt)
    lasers.resize(numLasers);
    
    // shouldn't be needed but hey
    disarmAllLasers();
    
    
    return true;
    
    
    
}

bool ManagerBase::scheduleSaveSettings() {
    if(!settingsNeedSave) {
        settingsNeedSave = true;
        return true;
    } else {
        return false;
    }
}

bool ManagerBase::saveSettings() {
    
    // update the number of lasers for the laserNum param
    // (it's automatically saved with the params)
    if(numLasers!=lasers.size()) numLasers = lasers.size();
    
    ofJson json;
    ofSerialize(json, params);

    canvasTarget->serialize(json["canvastarget"]);
    dacAssigner.serialize(json["dacassigner"]);
    saveAdditionalSettings(json);
    
    bool savesuccess = true;
    // Strategy:
    // wrap every file operation so transient filesystem issues (for example
    // invalid current working directory) are reported and retried later instead
    // of aborting the whole app from an uncaught filesystem exception.
    try {
        savesuccess &= ofSaveJson("ofxLaser/laserSettings.json", json);
    } catch(const std::exception& e) {
        savesuccess = false;
        ofLogError("ManagerBase::saveSettings") << "failed to write ofxLaser/laserSettings.json : " << e.what();
    } catch(...) {
        savesuccess = false;
        ofLogError("ManagerBase::saveSettings") << "failed to write ofxLaser/laserSettings.json : unknown exception";
    }
    
    for(size_t i= 0; i<lasers.size(); i++) {
        try {
            savesuccess &= lasers[i]->saveSettings();
        } catch(const std::exception& e) {
            savesuccess = false;
            ofLogError("ManagerBase::saveSettings") << "laser " << i << " save failed : " << e.what();
        } catch(...) {
            savesuccess = false;
            ofLogError("ManagerBase::saveSettings") << "laser " << i << " save failed : unknown exception";
        }
    }
    
    try {
        savesuccess &= dacAssigner.dacAliasManager.save();
    } catch(const std::exception& e) {
        savesuccess = false;
        ofLogError("ManagerBase::saveSettings") << "DAC alias save failed : " << e.what();
    } catch(...) {
        savesuccess = false;
        ofLogError("ManagerBase::saveSettings") << "DAC alias save failed : unknown exception";
    }
   
    
    // TODO add laserMask saving to laser settings
    //savesuccess &= laserMask.saveSettings();
    
    lastSaveTime = ofGetElapsedTimef();
    // Only clear pending-save when everything succeeded.
    // On failure we keep it queued so a later save attempt can recover.
    settingsNeedSave = !savesuccess;
    
    return savesuccess;
    
}



void ManagerBase :: serialize(ofJson& json) {

   // ofJson& jsonLaserSettings = json; // ["managersettings"];
    ofSerialize(json, params);

    canvasTarget->serialize(json["canvas"]);
    dacAssigner.serialize(json["dacassigner"]);
    
    ofJson& jsonLasers = json["lasers"];
    for(size_t i= 0; i<lasers.size(); i++) {
        lasers[i]->serialize(jsonLasers[i]);
    }
    
    dacAssigner.dacAliasManager.serialize(json["dac_aliases"]);
    
    
    
    
    
}



bool ManagerBase::deserialize(ofJson& json) {
    
    ofDeserialize(json, params);
    if(json.contains("dacassigner")) {
        dacAssigner.deserialize(json["dacassigner"]);
    }
    
    ofJson& jsonLasers = json["lasers"];
    
    while(lasers.size()>0) {
        deleteLaser(lasers.back());
    }
    
    
    // numLasers was saved in the json
    for(int i = 0; i<numLasers; i++) {
        
        // if we don't have a laser object already make one
        //if(lasers.size()<i+1) {
            createAndAddLaser();
        //} else {
            // if we already have a laser then make sure no dac is connected
            //dacAssigner.disconnectDacFromLaser(lasers[i]);
        //}
        std::shared_ptr<Laser>& laser = lasers[i];
        laser->deserialize(jsonLasers[i]);
        if(!laser->dacLabel->empty()) {
            dacAssigner.assignToLaser(laser->dacLabel, laser);
        }
        
        // if the laser has a dac id saved in the settings,
        // tell the dacAssigner about it
        // if the dac isn't available, it'll make the data and store it
        // ready for when it loads
    }
    
    
    if(json.contains("canvas")) {
        canvasTarget->deserialize(json["canvas"]);
    }

    if(json.contains("dac_aliases")) {
        dacAssigner.dacAliasManager.deserialize(json["dac_aliases"]);
    }
    
    // shouldn't be needed but hey
    disarmAllLasers();
    
    //scheduleSaveSettings();
    
    return true;
    
    
    
}

std::shared_ptr<Laser>& ManagerBase::getLaser(int index){
    return lasers.at(index);
};

std::vector<std::shared_ptr<Laser>>& ManagerBase::getLasers(){
    return lasers;
};

bool ManagerBase::isLaserArmed(unsigned int i){
    if((i<0) || (i>=lasers.size())){
        return false;
    } else {
        return lasers[i]->armed;
    }
    
}


bool ManagerBase::areAllLasersArmed(){
    for(std::shared_ptr<Laser>& laser : lasers) {
        if(!laser->armed) return false;
    }
    return (lasers.size()==0)? false : true;
    
}


// ============================================================
// LaserState sync — copies scalar state before notifying views
// ============================================================
void ManagerBase::syncLaserState() {
    laserState.globalBrightness       = globalBrightness;
    laserState.testPatternGlobalActive = testPatternGlobalActive;
    laserState.testPatternGlobal      = testPatternGlobal;
    laserState.allLasersArmed         = areAllLasersArmed();
    laserState.numLasers              = (int)lasers.size();
}


// ============================================================
// View registration
// ============================================================
void ManagerBase::addLaserView(LaserBaseView* view) {
    view->laserState = &laserState;
    laserViews.push_back(view);
    syncLaserState();
    // Fire all signals so the new view has a complete initial state
    view->onLasersChanged();
    view->onZonesChanged();
    view->onCanvasChanged();
    view->onGlobalSettingsChanged();
    view->onDacStatusChanged();
    view->onTestPatternChanged();
    view->onMasksChanged();
}

bool ManagerBase::removeLaserView(LaserBaseView* view) {
    auto it = std::find(laserViews.begin(), laserViews.end(), view);
    if(it != laserViews.end()) {
        (*it)->laserState = nullptr;
        laserViews.erase(it);
        return true;
    }
    return false;
}


// ============================================================
// Deferred signal queue — fire*() sets a bit, flushPendingSignals()
// delivers on the next update() frame.
// ============================================================
void ManagerBase::fireLasersChanged()       { pendingSignals |= SIG_LASERS; }
void ManagerBase::fireZonesChanged()        { pendingSignals |= SIG_ZONES; }
void ManagerBase::fireCanvasChanged()       { pendingSignals |= SIG_CANVAS; }
void ManagerBase::fireDacStatusChanged()    { pendingSignals |= SIG_DAC_STATUS; }
void ManagerBase::fireGlobalSettingsChanged() { pendingSignals |= SIG_GLOBAL_SETTINGS; }
void ManagerBase::fireTestPatternChanged()  { pendingSignals |= SIG_TEST_PATTERN; }

void ManagerBase::fireMasksChanged() {
    pendingSignals |= SIG_MASKS;
}

void ManagerBase::flushPendingSignals() {
    if(pendingSignals == 0) return;

    // Snapshot and clear before dispatching — if a signal handler
    // triggers another fire*(), those accumulate for the NEXT flush.
    uint32_t signals = pendingSignals;
    pendingSignals = 0;

    syncLaserState();

    for(auto* v : laserViews) {
        if(signals & SIG_LASERS)          v->onLasersChanged();
        if(signals & SIG_ZONES)           v->onZonesChanged();
        if(signals & SIG_CANVAS)          v->onCanvasChanged();
        if(signals & SIG_DAC_STATUS)      v->onDacStatusChanged();
        if(signals & SIG_GLOBAL_SETTINGS) v->onGlobalSettingsChanged();
        if(signals & SIG_TEST_PATTERN)    v->onTestPatternChanged();
        if(signals & SIG_MASKS)           v->onMasksChanged();
    }
}


// ============================================================
// Message bus receiver — dispatch via std::visit
// ============================================================
void ManagerBase::receiveLaserMessage(LaserMsgEnvelope& env) {

    std::visit(overloaded{

        // --- Laser management ---
        [&](LaserMsg::AddLaser&) {
            createAndAddLaser();
            fireLasersChanged();
        },
        [&](LaserMsg::DeleteLaser& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                // Take a copy — deleteLaser erases from the vector,
                // invalidating the reference if passed by ref.
                auto laserCopy = lasers[m.laserIndex];
                deleteLaser(laserCopy);
                fireLasersChanged();
                fireZonesChanged();
            }
        },
        [&](LaserMsg::SelectLaser&) {
            // Selection is a UI concern — Manager subclass handles this
            // via its own override or signal
        },

        // --- Zone management ---
        [&](LaserMsg::AddZone& m) {
            addZone(m.x, m.y, m.w, m.h);
            fireCanvasChanged();
            fireZonesChanged();
        },
        [&](LaserMsg::DeleteZone& m) {
            auto zone = canvasTarget->getInputZoneForZoneIdUid(m.zoneUid);
            if(zone) {
                deleteZone(zone);
                fireCanvasChanged();
                fireZonesChanged();
            }
        },
        [&](LaserMsg::AddZoneToLaser& m) {
            // Find the ZoneId by uid
            auto obj = canvasTarget->getObjectForZoneIdUid(const_cast<string&>(m.zoneUid));
            if(obj) {
                addZoneToLaser(obj->zoneId, m.laserIndex);
            }
            fireZonesChanged();
        },
        [&](LaserMsg::ZoneMoved& m) {
            auto zone = canvasTarget->getInputZoneForZoneIdUid(m.zoneUid);
            if(zone) {
                zone->set(m.rect.x, m.rect.y, m.rect.width, m.rect.height);
                fireCanvasChanged();
            }
        },
        [&](LaserMsg::SetCanvasSize& m) {
            setCanvasSize(m.width, m.height);
            fireCanvasChanged();
        },

        // --- Global settings ---
        [&](LaserMsg::SetGlobalBrightness& m) {
            globalBrightness = m.value;
            fireGlobalSettingsChanged();
        },
        [&](LaserMsg::SetTestPattern& m) {
            testPatternGlobal = m.pattern;
            testPatternGlobalActive = m.active;
            updateGlobalTestPattern();
            fireTestPatternChanged();
        },
        [&](LaserMsg::SetGlobalLatency&) {
            // Latency is managed by Manager subclass (it's an ofParameter)
            fireGlobalSettingsChanged();
        },

        // --- Arm / disarm ---
        [&](LaserMsg::ArmAllLasers&) {
            armAllLasers();
            fireGlobalSettingsChanged();
        },
        [&](LaserMsg::DisarmAllLasers&) {
            disarmAllLasers();
            fireGlobalSettingsChanged();
        },

        // --- Zone transform changes ---
        [&](LaserMsg::ZoneTransformChanged&) {
            // Complex zone transform updates — handled by Manager override
            fireZonesChanged();
        },
        [&](LaserMsg::ZoneMuteChanged& m) {
            for(auto& laser : lasers) {
                for(auto& oz : laser->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid) {
                        oz->muted = m.muted;
                    }
                }
            }
            fireZonesChanged();
        },
        [&](LaserMsg::ZoneLockChanged& m) {
            for(auto& laser : lasers) {
                for(auto& oz : laser->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid) {
                        oz->locked = m.locked;
                    }
                }
            }
            fireZonesChanged();
        },
        [&](LaserMsg::ZoneTypeChanged& m) {
            for(auto& laser : lasers) {
                for(auto& oz : laser->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid) {
                        oz->transformType = m.transformType;
                    }
                }
            }
            fireZonesChanged();
        },
        [&](LaserMsg::ZoneResetTransform& m) {
            for(auto& laser : lasers) {
                for(auto& oz : laser->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid) {
                        oz->resetAllTransforms();
                    }
                }
            }
            fireZonesChanged();
        },
        [&](LaserMsg::DeleteOutputZone& m) {
            // Find the matching zone and remove it from its laser
            for(auto& laser : lasers) {
                for(auto& oz : laser->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid) {
                        laser->removeZone(oz);
                        fireZonesChanged();
                        return;
                    }
                }
            }
        },

        // --- Mask management ---
        [&](LaserMsg::MaskChanged&) {
            fireMasksChanged();
        },
        [&](LaserMsg::DeleteMask& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                auto& maskMgr = lasers[m.laserIndex]->maskManager;
                if(m.maskIndex >= 0 && m.maskIndex < (int)maskMgr.quads.size()) {
                    // Take a copy — deleteQuadMask erases from the vector
                    auto maskCopy = maskMgr.quads[m.maskIndex];
                    maskMgr.deleteQuadMask(maskCopy);
                    fireMasksChanged();
                }
            }
        },

        // --- View mode / GUI ---
        [&](LaserMsg::SetViewMode&) {
            // Handled by Manager subclass
        },
        [&](LaserMsg::ToggleGui&) {
            // Handled by Manager subclass
        },
        [&](LaserMsg::SetGuiVisible&) {
            // Handled by Manager subclass
        },

        // --- Settings persistence ---
        [&](LaserMsg::SaveSettings&) {
            saveSettings();
        },
        [&](LaserMsg::ResetAllLasers&) {
            resetAllLasersToDefault();
            fireLasersChanged();
            fireZonesChanged();
            fireCanvasChanged();
        },

        // --- DAC assignment ---
        [&](LaserMsg::AssignDac& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                dacAssigner.assignToLaser(m.dacLabel, lasers[m.laserIndex]);
                fireDacStatusChanged();
            }
        },
        [&](LaserMsg::DisconnectDac& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                auto& laser = lasers[m.laserIndex];
                string savedLabel = laser->dacLabel;
                dacAssigner.disconnectDacFromLaser(laser);
                // Preserve the label so reconnect can find it again
                if(!savedLabel.empty()) laser->dacLabel = savedLabel;
                fireDacStatusChanged();
            }
        },

        // --- Per-laser settings ---
        [&](LaserMsg::SetLaserArmed& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                lasers[m.laserIndex]->armed = m.armed;
                fireGlobalSettingsChanged();
            }
        },
        [&](LaserMsg::SetLaserTestPattern& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                lasers[m.laserIndex]->testPatternActive = m.active;
                lasers[m.laserIndex]->testPattern = m.pattern;
                fireLasersChanged();
            }
        },
        [&](LaserMsg::SetLaserFlipX& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                lasers[m.laserIndex]->flipX = m.flip;
                fireLasersChanged();
            }
        },
        [&](LaserMsg::SetLaserFlipY& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                lasers[m.laserIndex]->flipY = m.flip;
                fireLasersChanged();
            }
        },
        [&](LaserMsg::ResetLaserRotation& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                lasers[m.laserIndex]->rotation = 0;
                fireLasersChanged();
            }
        },
        [&](LaserMsg::ResetLaserOffset& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                lasers[m.laserIndex]->outputOffset.set(glm::vec2(0,0));
                fireLasersChanged();
            }
        },
        [&](LaserMsg::SetHideContentDuringTestPattern& m) {
            hideContentDuringTestPattern = m.hide;
            for(auto& laser : lasers) {
                laser->hideContentDuringTestPattern = m.hide;
            }
            scheduleSaveSettings();
            fireGlobalSettingsChanged();
        },

        // --- Zone assignment on a laser ---
        [&](LaserMsg::RemoveZoneFromLaser& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                ZoneId zoneId;
                for(auto& oz : lasers[m.laserIndex]->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid) {
                        zoneId = oz->getZoneId();
                        break;
                    }
                }
                lasers[m.laserIndex]->removeZone(zoneId);
                fireZonesChanged();
            }
        }

    }, env.payload);
}

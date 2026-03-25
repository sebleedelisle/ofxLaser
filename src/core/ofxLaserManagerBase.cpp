//
//  ofxLaserManager.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserManagerBase.h"

using namespace ofxLaser;

ManagerBase * ManagerBase :: laserManager = NULL;

ManagerBase * ManagerBase::instance() {
    if(laserManager == NULL) {
        laserManager = new ManagerBase();
    }
    return laserManager;
}


ManagerBase :: ManagerBase()
    : dacAssigner(*DacAssigner::instance())
    , laserState{ lasers, beamZoneContainer, canvasTarget, dacAssigner, laserMask }
{


    canvasTarget = std::make_shared<ShapeTargetCanvas>();
    
    if(laserManager == NULL) {
        laserManager = this;
    } else {
        ofLog(OF_LOG_ERROR, "Multiple ofxLaser::Manager instances created");
    }
    
    setCanvasSize(800,800);
    
    params.setName("Laser");
    params.add(globalBrightness.set("Global brightness", 0.2,0,1));

    params.add(numLasers.set("numLasers", 0));
    params.add(useAltZones.set("Use alternative zones", false));
    params.add(dontCalculateDisconnected.set("Don't calculate disconnected", false));
    params.add(hideContentDuringTestPattern.set("Hide content during test pattern", true));
    
    useAltZones.addListener(this, &ofxLaser::ManagerBase::useAltZonesChanged);
    
    testPatternGlobal = 1;
    testPatternGlobalActive = false; 

    currentShapeTarget = canvasTarget;
    
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
    beamZoneContainer.clearZones();
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
   // if(find(lasers.begin(), lasers.end(), laser) == lasers.end()) return false;
    
    // disconnect dac
    dacAssigner.disconnectDacFromLaser(laser);
    
    //vector<std::shared_ptr<Laser>> :: iterator it = find(lasers.begin(), lasers.end(), laser);
    //int index = it-lasers.begin();
    // hopefully should renumber current laser OK
    
    laser->deleteAllSettingsFiles();
    // TODO delete zones that are only assigned to this laser *************************
    if(deleteZones) {
        vector<std::shared_ptr<OutputZone>> zones = laser->getSortedOutputZones();
        vector<std::shared_ptr<OutputZone>>  altzones = laser->getSortedOutputAltZones();
        zones.insert(zones.end(), altzones.begin(), altzones.end());

        for(std::shared_ptr<OutputZone>& zone : zones) {
            ZoneId zoneid = zone->getZoneId();
            if(zoneid.getType() == ZoneId::BEAM) {
                deleteBeamZone(zone);
            }
           
        }
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

ZoneId ManagerBase::addCanvasZone(const ofRectangle& rect) {
    return addCanvasZone(rect.x, rect.y, rect.width, rect.height);
    
}

ZoneId  ManagerBase :: addCanvasZone(float x, float y, float w, float h) {
    if(w<=0) w = canvasTarget->getWidth();
    if(h<=0) h = canvasTarget->getHeight();
    return canvasTarget->addInputZone(x, y, w, h);
}


ZoneId ManagerBase :: createNewBeamZone() {
   
    return beamZoneContainer.addBeamZone();
}
    


bool ManagerBase :: deleteCanvasZone(std::shared_ptr<InputZone> inputZone) {
    if(inputZone==nullptr) return false;
    
    map<ZoneId, ZoneId>  changedzones = canvasTarget->removeZoneById(inputZone->getZoneId());
    for(std::shared_ptr<Laser>& laser : lasers) {
        laser->updateZones(changedzones);
        laser->removeZone(inputZone->getZoneId());
    }
    return true;
    
    
}

bool ManagerBase::deleteBeamZone(std::shared_ptr<OutputZone>& outputZone) {
    
    ZoneId zoneid = outputZone->getZoneId();
    
    bool changed = false;
    
    if(outputZone->getIsAlternate()) {
        for(std::shared_ptr<Laser>& laser : lasers) {
            changed = laser->removeAltZone(zoneid) || changed;
        }
        
    } else {
        map<ZoneId, ZoneId> changedZones = beamZoneContainer.removeZoneById(zoneid);
        // zone should actually only be in one of the lasers...
       
        for(std::shared_ptr<Laser>& laser : lasers) {
            changed = laser->removeAltZone(zoneid) || changed;
            changed = laser->removeZone(zoneid) || changed;
            changed = laser->updateZones(changedZones)|| changed;
        }
        
    }
    
    if (changed) {
        scheduleSaveSettings();
        return true;
    } else{
        return false;
    }
}


void ManagerBase::addZoneToLaser(ZoneId& zoneId, unsigned int lasernum) {
    if(lasers.size()<=lasernum) {
        ofLog(OF_LOG_ERROR, "Invalid laser number passed to addZoneToLaser(...)");
        return;
    }
    // Todo - check zone exists
    lasers[lasernum]->addZone(zoneId);
}

int ManagerBase::getLaserIndexForBeamZoneId(ZoneId& zoneId) {
        
    for(int i = 0; i<lasers.size(); i++) {
        Laser& laser = *lasers[i];
        if(laser.hasZone(zoneId)) {
            return i;
        }
        
    }
    return -1;
}

bool ManagerBase :: moveBeamZoneToIndex(int sourceindex, int targetindex) {
    if(sourceindex == targetindex) return false;
    if((sourceindex<0) || (sourceindex>=beamZoneContainer.getNumZoneIds()) || (targetindex<0) || (targetindex>=beamZoneContainer.getNumZoneIds()))
        return false;
    
    ZoneId zoneid = beamZoneContainer.getBeamZoneAtIndex(sourceindex)->zoneId;
    map<ZoneId, ZoneId> changedzones = beamZoneContainer.moveZoneByIdToIndex(zoneid,targetindex);
    
    for(std::shared_ptr<Laser>& laser : lasers) {
        laser->updateZones(changedzones);
    }
    return true;
    
}


void ManagerBase::createDefaultCanvasZone() {
    // check there aren't any zones yet?
    // create a zone equal to the width and height of the total output space
    addCanvasZone(0,0,canvasTarget->getWidth(),canvasTarget->getHeight());
    
}


void ManagerBase::drawDot( float x,  float y, const ofColor& col, float intensity, string profileLabel) {
    drawDot(glm::vec3(x, y, 0), col, intensity, profileLabel);
}
void ManagerBase::drawDot( const glm::vec2& p, const ofColor& col, float intensity, string profileLabel) {
    drawDot(glm::vec3(p.x, p.y, 0), col, intensity, profileLabel);
}


void ManagerBase::drawDot(const glm::vec3& p, const ofColor& col, float intensity, string profileLabel) {
    
    std::shared_ptr<Dot> d = std::make_shared<Dot>(getTransformed(p), col, intensity, profileLabel);
    currentShapeTarget->addShape(d, useClipRectangle, clipRectangle);
}



void ManagerBase::drawLine(float x1, float y1, float x2, float y2, const ofColor& col, string profileName){
    drawLine( glm::vec3(x1,y1,0), glm::vec3(x2,y2,0), col, profileName);
}

void ManagerBase::drawLine(const glm::vec2& start, const glm::vec2& end, const ofColor& col, string profileName){
    drawLine( glm::vec3(start.x, start.y, 0), glm::vec3(end.x, end.y, 0), col, profileName);
}

void ManagerBase::drawLine(const glm::vec3& start, const glm::vec3& end, const ofColor& col, string profileLabel) {
    
    std::shared_ptr<Line> l = std::make_shared<Line>(getTransformed(start), getTransformed(end), col, profileLabel);
    
    currentShapeTarget->addShape(l, useClipRectangle, clipRectangle);

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
    
    currentShapeTarget->addShape(c, useClipRectangle, clipRectangle);
    
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
        currentShapeTarget->addShape(p, useClipRectangle, clipRectangle);
    } else {
        // shape should get destroyed
    }
}

std::shared_ptr<ofxLaser::Polyline> ManagerBase::getPolyFromPoints(const vector<glm::vec3>& points, const vector<ofColor>& colours, bool closed, string profileName, float brightness){
    
    tmpPoints = points;
//    for(glm::vec3& v : tmpPoints) {
//        v = getTransformed(v);
//    }
//    
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
        //std::reverse(colours.begin(), colours.end());
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
            currentShapeTarget->addShape(poly, useClipRectangle, clipRectangle);
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
            currentShapeTarget->addShape(poly, useClipRectangle, clipRectangle);
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
            currentShapeTarget->addShape(poly, useClipRectangle, clipRectangle);
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
    // bit of a hack to check dacs that may take a little while to appear
    if(ofGetFrameNum()==1000) {
        dacAssigner.updateDacList();
    }
    
    // resets transformations
    resetTransformations();
    
    dacAssigner.update();
    
//    for(ofxLaser::Laser* laser : lasers) {
//        laser->emptyDac.dontCalculate = dontCalculateDisconnected.get();
//    }
    
    //if(useBitmapMask) laserMask.update();
    // delete all the shapes - all shape objects need a destructor!
    canvasTarget->deleteShapes();
    beamZoneContainer.deleteShapes(); 
    
    // updates all the zones. If zone->update returns true, then
    // it means that the zone has changed.
    bool updateZoneRects = false;
//    for(size_t i= 0; i<zones.size(); i++) {
//        if(zones[i]->update()) {
//           updateZoneRects  = true;
//        }
//    }
    
    
    
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
//        if(!beepSound.isPlaying()) {
//            beepSound.play();
//        }
    }
     
    if(useAltZones && (!hasAnyAltZones())) {
        useAltZones.set(false);
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
    
    for(int i = 0; i<beamZoneContainer.getNumBeamZones(); i++) {
        beamZoneContainer.getBeamZoneAtIndex(i)->processShapes();
    }
    
    // here's where the magic happens.
    // 1 :
    // figure out which zones to send the shapes to
    // and send them. When the zones get the shape, they transform them
    // into local zone space.
    
    //vector<deque<Shape*>> shapesByZoneIndex;
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
    for(std::shared_ptr<ObjectWithZoneId>& zoneIdObject : beamZoneContainer.getZoneIds()) {
        
        std::shared_ptr<ShapeTargetBeamZone> beamzone = beamZoneContainer.getBeamZoneForZoneId(zoneIdObject->zoneId);

        zonesContent.push_back(ZoneContent());
        ZoneContent& zoneContent = zonesContent.back();
        vector<std::shared_ptr<Shape>>& newshapes = zoneContent.shapes;
        zoneContent.zoneId = zoneIdObject->zoneId;
        zoneContent.sourceRectangle.set(0,0,800,800);
        zoneContent.shapes = beamzone->shapes;
        
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
        
        laser.send(zonesContent, globalBrightness, NULL);// useBitmapMask?laserMask.getPixels():NULL);
        
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
//    if(zonenum>=canvasTarget->zones.size()) {
//        ofLogError("Invalid zone number sent to ofxLaser::ManagerBase::sendRawPoints");
//        return;
//    }
    laser->sendRawPoints(points, zoneId, globalBrightness);
    
}


//
//void ManagerBase::armAllLasersListener() {
//    doArmAll = true;
//}
//
//void ManagerBase::disarmAllLasersListener(){
//    doDisarmAll = true;
//}
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
void ManagerBase::useAltZonesChanged(bool& state) {
    
    for(std::shared_ptr<Laser>& laser : lasers) {
        laser->useAlternate = useAltZones;
        
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
    
    
    if(!beamZoneContainer.deserialize(json["beamzones"])) {
        // try old format
        beamZoneContainer.deserialize(json["beamZones"]);
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
    
    // make sure the lasers adopt the zone labels from the beamZoneContainer
    updateZoneLabels();
    // shouldn't be needed but hey
    disarmAllLasers();
    
    
//
//
//    if(zones.size()==0) {
//
//    } else {
//        renumberCanvasZones();
//    }
//
//    json.clear();
//    serialize(json);
//    deserialize(json);
    
    return true;
    
    
    
}

bool ManagerBase::updateZoneLabels() {
    bool changed = false;
    for(std::shared_ptr<Laser>& laser : lasers) {
        if(laser->updateZoneLabels(beamZoneContainer.getZoneIds())) {
            changed = true;
        }
    }
    return changed; 
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

    beamZoneContainer.serialize(json["beamzones"]);
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
    
    // Save zones :
//    ofJson zoneJson;
    //TODO REPLACE THIS WITH canvasTarget->serialize
//
//    for(int i = 0; i<canvasTarget->zones.size(); i++) {
//        ofJson jsonGroup;
//        canvasTarget->zones[i]->serialize(jsonGroup);
//        zoneJson.push_back(jsonGroup);
//    }
//
//    ofSavePrettyJson("ofxLaser/zones.json", zoneJson);
    
    lastSaveTime = ofGetElapsedTimef();
    // Only clear pending-save when everything succeeded.
    // On failure we keep it queued so a later save attempt can recover.
    settingsNeedSave = !savesuccess;
    
    return savesuccess;
    
}



void ManagerBase :: serialize(ofJson& json) {

   // ofJson& jsonLaserSettings = json; // ["managersettings"];
    ofSerialize(json, params);

    beamZoneContainer.serialize(json["beamzones"]);
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
//        if(!laser->dacLabel->empty()) {
//            dacAssigner.assignToLaser(laser->dacLabel, laser);
//        }
        
    }
    // if we had more lasers to start with than we needed, then resize
    // the vector (shouldn't be needed but it doesn't hurt)
//    while(lasers.size()>numLasers) {
//        deleteLaser(lasers.back());
//    }
    //lasers.resize(numLasers);
    
    
    if(json.contains("canvas")) {
        canvasTarget->deserialize(json["canvas"]);
    }

    if(!beamZoneContainer.deserialize(json["beamzones"])) {
        // try old format
        beamZoneContainer.deserialize(json["beamZones"]);
    }
    
    vector<ZoneId> zonesToDelete;
    // ok sanity check time! Let's make sure all the beam zones have lasers
    for(int i = 0; i<beamZoneContainer.getNumZoneIds(); i++ ){
        ZoneId& zoneid = beamZoneContainer.getBeamZoneAtIndex(i)->zoneId;
        int laserindexforzone = getLaserIndexForBeamZoneId(zoneid);
        if(laserindexforzone<0) {
            zonesToDelete.push_back(zoneid);
        }
    }
    while(zonesToDelete.size()>0) {
        ZoneId zoneid = zonesToDelete.back();
        beamZoneContainer.removeZoneById(zoneid);
        zonesToDelete.pop_back();
    } 
    
    if(json.contains("dac_aliases")) {
        dacAssigner.dacAliasManager.deserialize(json["dac_aliases"]);
    }
    
    // shouldn't be needed but hey
    disarmAllLasers();
    
    //scheduleSaveSettings();
    
    return true;
    
    
    
}
//
//
//
//// converts openGL coords to screen coords //
//template<typename T>
//T ManagerBase::convert3DTo2D(T p, ofRectangle viewportrect, float fov ) {
//
//    T p1 = getTransformed(p);
//
//    if(p1.z==0) return p1;
//
//    float scale = fov/(-p1.z+fov);
//    p1.z = 0;
//    p1-=viewportrect.getCenter();
//    p1*=scale;
//    p1+=viewportrect.getCenter();
//
//    return p1;
// 
//}
//template<typename T>
//T ManagerBase::convert3DTo2D(T p) {
//    return convert3DTo2D(p, currentShapeTarget->getBounds());
//
//}
//

std::shared_ptr<Laser>& ManagerBase::getLaser(int index){
    return lasers.at(index);
};

std::vector<std::shared_ptr<Laser>>& ManagerBase::getLasers(){
    return lasers;
};

bool ManagerBase::setTargetBeamZone(int index) {
    std::shared_ptr<ShapeTargetBeamZone> beamzone = beamZoneContainer.getBeamZoneAtIndex(index);
    if(beamzone) {
        currentShapeTarget = beamzone;
        return true;
    } else {
        return false;
    }
}


std::shared_ptr<ShapeTargetBeamZone> ManagerBase::getBeamZoneByIndex(int index) {
    return beamZoneContainer.getBeamZoneAtIndex(index);
    
}

bool ManagerBase::setTargetCanvas(int index) {
    // NB index for future use
    currentShapeTarget = canvasTarget;
    return true; 
}


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

bool ManagerBase::areAllLasersUsingAlternateZones(){
    for(std::shared_ptr<Laser>& laser : lasers) {
        if(!laser->useAlternate)  return false;
    }
    return (lasers.size()==0)? false : true;
    
}
bool ManagerBase::hasAnyAltZones() {
    for(std::shared_ptr<Laser>& laser : lasers) {
        if(laser->hasAnyAltZones()) return true;
    }
    return false;
}


bool ManagerBase::toggleAltZones() {

    if (lasers.empty()) {
        return false; // no lasers, nothing toggled
    }

    bool newState = !lasers.front()->useAlternate;
    if(newState) setAllAltZones();
    else unSetAllAltZones();
    return newState; 


}

void ManagerBase::setAllAltZones() {
    for(std::shared_ptr<Laser>& laser : lasers) {
        laser->useAlternate = true;
    }
    
}
void ManagerBase::unSetAllAltZones(){
    for(std::shared_ptr<Laser>& laser : lasers) {
        laser->useAlternate = false;
    }
    
}


//------------------- DEPRECATED --------------------------


void ManagerBase::addProjector(DacBase& dac) {
    ofLogError("Lasers are no longer set up in code! Add them within the app instead.");
    throw;
    
}

void ManagerBase::addProjector() {
    ofLogError("Lasers are no longer set up in code! Add them within the app instead.");
    throw;
    
}
// DEPRECATED, showAdvanced parameter now redundant
void ManagerBase::initGui(bool showAdvanced) {
    ofLogError("ManagerBase::initGui(bool showAdvanced) initGui is no longer required");
    throw;
}


// ============================================================
// LaserState sync — copies scalar state before notifying views
// ============================================================
void ManagerBase::syncLaserState() {
    laserState.globalBrightness       = globalBrightness;
    laserState.testPatternGlobalActive = testPatternGlobalActive;
    laserState.testPatternGlobal      = testPatternGlobal;
    laserState.useAltZones            = useAltZones;
    laserState.allLasersArmed         = areAllLasersArmed();
    laserState.numLasers              = (int)lasers.size();
    laserState.numBeamZones           = (int)beamZoneContainer.getNumZoneIds();
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
// Signal-firing helpers
// ============================================================
void ManagerBase::fireLasersChanged() {
    syncLaserState();
    for(auto* v : laserViews) v->onLasersChanged();
}

void ManagerBase::fireZonesChanged() {
    syncLaserState();
    for(auto* v : laserViews) v->onZonesChanged();
}

void ManagerBase::fireCanvasChanged() {
    syncLaserState();
    for(auto* v : laserViews) v->onCanvasChanged();
}

void ManagerBase::fireDacStatusChanged() {
    syncLaserState();
    for(auto* v : laserViews) v->onDacStatusChanged();
}

void ManagerBase::fireGlobalSettingsChanged() {
    syncLaserState();
    for(auto* v : laserViews) v->onGlobalSettingsChanged();
}

void ManagerBase::fireTestPatternChanged() {
    syncLaserState();
    for(auto* v : laserViews) v->onTestPatternChanged();
}

void ManagerBase::fireMasksChanged() {
    syncLaserState();
    for(auto* v : laserViews) v->onMasksChanged();
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
                // Don't fire signals synchronously — deleteLaser does
                // complex teardown (deleteBeamZone, renumbering, saves).
                // Views poll on next update().
                scheduleSaveSettings();
            }
        },
        [&](LaserMsg::SelectLaser&) {
            // Selection is a UI concern — Manager subclass handles this
            // via its own override or signal
        },

        // --- Beam zone management ---
        [&](LaserMsg::CreateBeamZone&) {
            createNewBeamZone();
            fireZonesChanged();
        },
        [&](LaserMsg::DeleteBeamZone& m) {
            // Use DeleteOutputZone for actual beam zone deletion.
            // This handler is kept for direct beam-zone-by-UID deletion if needed.
        },
        [&](LaserMsg::MoveBeamZone& m) {
            moveBeamZoneToIndex(m.sourceIndex, m.targetIndex);
            fireZonesChanged();
        },
        [&](LaserMsg::AddZoneToLaser& m) {
            // Find the ZoneId by uid
            for(int i = 0; i < beamZoneContainer.getNumBeamZones(); i++) {
                auto bz = beamZoneContainer.getBeamZoneAtIndex(i);
                if(bz && bz->zoneId.getUid() == m.zoneUid) {
                    addZoneToLaser(bz->zoneId, m.laserIndex);
                    break;
                }
            }
            fireZonesChanged();
        },

        // --- Canvas zone management ---
        [&](LaserMsg::AddCanvasZone& m) {
            addCanvasZone(m.x, m.y, m.w, m.h);
            fireCanvasChanged();
        },
        [&](LaserMsg::DeleteCanvasZone& m) {
            auto zone = canvasTarget->getInputZoneForZoneIdUid(m.zoneUid);
            if(zone) {
                deleteCanvasZone(zone);
                // Don't fire signals synchronously — deleteCanvasZone
                // renumbers zones and modifies laser references.
                scheduleSaveSettings();
            }
        },
        [&](LaserMsg::CanvasZoneMoved& m) {
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
        [&](LaserMsg::SetUseAltZones& m) {
            useAltZones = m.state;
            fireGlobalSettingsChanged();
        },
        [&](LaserMsg::ToggleAltZones&) {
            toggleAltZones();
            fireGlobalSettingsChanged();
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
                    if(oz->getZoneId().getUid() == m.zoneUid && oz->getIsAlternate() == m.isAlt) {
                        oz->muted = m.muted;
                    }
                }
            }
            fireZonesChanged();
        },
        [&](LaserMsg::ZoneLockChanged& m) {
            for(auto& laser : lasers) {
                for(auto& oz : laser->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid && oz->getIsAlternate() == m.isAlt) {
                        oz->locked = m.locked;
                    }
                }
            }
            fireZonesChanged();
        },
        [&](LaserMsg::ZoneTypeChanged& m) {
            for(auto& laser : lasers) {
                for(auto& oz : laser->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid && oz->getIsAlternate() == m.isAlt) {
                        oz->transformType = m.transformType;
                    }
                }
            }
            fireZonesChanged();
        },
        [&](LaserMsg::ZoneResetTransform& m) {
            for(auto& laser : lasers) {
                for(auto& oz : laser->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid && oz->getIsAlternate() == m.isAlt) {
                        oz->resetAllTransforms();
                    }
                }
            }
            fireZonesChanged();
        },
        [&](LaserMsg::AddAltZone& m) {
            if(m.laserIndex >= 0 && m.laserIndex < (int)lasers.size()) {
                // Find matching ZoneId by UID from the laser's existing zones
                for(auto& oz : lasers[m.laserIndex]->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid && !oz->getIsAlternate()) {
                        lasers[m.laserIndex]->addAltZone(oz->getZoneId());
                        break;
                    }
                }
                fireZonesChanged();
            }
        },
        [&](LaserMsg::DeleteOutputZone& m) {
            // Find the matching zone first, then delete outside the iterator
            std::shared_ptr<OutputZone> found = nullptr;
            std::shared_ptr<Laser> ownerLaser = nullptr;
            for(auto& laser : lasers) {
                for(auto& oz : laser->outputZones) {
                    if(oz->getZoneId().getUid() == m.zoneUid && oz->getIsAlternate() == m.isAlt) {
                        found = oz;
                        ownerLaser = laser;
                        break;
                    }
                }
                if(found) break;
            }
            if(found && ownerLaser) {
                if(found->getZoneId().getType() == ZoneId::ZoneType::CANVAS) {
                    ownerLaser->removeZone(found);
                } else {
                    deleteBeamZone(found);
                }
                // Don't fire signals synchronously after destructive zone operations.
                // deleteBeamZone does complex renumbering + saves that leave intermediate state.
                // Views already poll for zone changes in their update() loop.
                scheduleSaveSettings();
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
                    // Don't fire synchronously — views poll next frame.
                    scheduleSaveSettings();
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
            // Don't fire signals synchronously — resetAllLasersToDefault
            // tears down and rebuilds the entire model. Views poll next frame.
            scheduleSaveSettings();
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
                // Don't fire synchronously — removeZone modifies outputZones
                // and calls saveSettings. Views poll next frame.
                scheduleSaveSettings();
            }
        }

    }, env.payload);
}

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


ManagerBase :: ManagerBase() : dacAssigner(*DacAssigner::instance()) {
    //ofLog(OF_LOG_NOTICE, "ofxLaser::Manager constructor");
    if(laserManager == NULL) {
        laserManager = this;
    } else {
        ofLog(OF_LOG_ERROR, "Multiple ofxLaser::Manager instances created");
    }
    
    setCanvasSize(800,800);
    //std::this_thread::
//    GetCurrentThread(std::this_thread);
//    //auto & thread = getNativeThread();
//
//#ifndef _MSC_VER
//    // only linux and osx
//    //http://www.yonch.com/tech/82-linux-thread-priority
//    struct sched_param param;
//    param.sched_priority = 89; // (highest) sched_get_priority_max(SCHED_FIFO);//89; // - higher is faster
//    pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &param );
//#else
//    // windows implementation
//    SetThreadPriority( thread.native_handle(), THREAD_PRIORITY_HIGHEST);
//#endif
    
    
    params.setName("Laser");
    params.add(globalBrightness.set("Global brightness", 0.2,0,1));

    //useBitmapMask = showBitmapMask = laserCanvasMaskOutlines = false;
    params.add(numLasers.set("numLasers", 0));
    params.add(useAltZones.set("Use alternative zones", false));
    params.add(dontCalculateDisconnected.set("Don't calculate disconnected", false));
    params.add(hideContentDuringTestPattern.set("Hide content during test pattern", true));
    
    useAltZones.addListener(this, &ofxLaser::ManagerBase::useAltZonesChanged);
    //hideContentDuringTestPattern.addListener(this, &ofxLaser::ManagerBase::hideContentDuringTestPatternChanged);
    testPatternGlobal = 1;
    testPatternGlobalActive = false; 
    
    //beepSound.load("Beep1.wav");
    currentShapeTarget = &canvasTarget;
    
    useClipRectangle = false; 
    
    
    ClipperUtils::initialise();
    
    
    
}

ManagerBase :: ~ManagerBase() {

    saveSettings();
    
}

//
//
void ManagerBase ::resetAllLasersToDefault() {
    vector<string> dacLabels;
    for(Laser* laser : lasers) {
        dacLabels.push_back(laser->dacLabel.get());
    }
    
    int numlasers = getNumLasers();
    while(lasers.size()>0) {
        deleteLaser(lasers[0]);
    }
    beamZoneContainer.clearZones();
    
    for(int lasernum = 0; lasernum<numlasers; lasernum++) {
        createAndAddLaser();
        ZoneId zoneId = createNewBeamZone();
        addZoneToLaser(zoneId, lasernum);
        //getLaser(lasernum).dacLabel = dacLabels[lasernum];
        dacAssigner.assignToLaser(dacLabels[lasernum], getLaser(lasernum));
    }
    
    saveSettings();
    //outputZone
    
}


void ManagerBase :: setCanvasSize(int w, int h){
    canvasTarget.setBounds(0,0,w,h);
}

void ManagerBase::createAndAddLaser() {
    
    // create and add new laser object
    
    Laser* laser = new Laser(lasers.size());
    lasers.push_back(laser);
    
    
    // TODO should this be here?
    laser->init();

    
}



bool ManagerBase :: deleteLaser(Laser* laser) {
    
    bool deleteZones = true;
    
    // check if laser exists and isn't null
    if(laser == nullptr) return false;
    
    if(find(lasers.begin(), lasers.end(), laser) == lasers.end()) return false;
    
    // disconnect dac
    dacAssigner.disconnectDacFromLaser(*laser);
    
    vector<Laser*> :: iterator it = find(lasers.begin(), lasers.end(), laser);
    //int index = it-lasers.begin();
    // hopefully should renumber current laser OK
    
    laser->deleteAllSettingsFiles();
    // TODO delete zones that are only assigned to this laser *************************
    if(deleteZones) {
        vector<OutputZone*> zones = laser->getSortedOutputZones();
        vector<OutputZone*>  altzones = laser->getSortedOutputAltZones();
        zones.insert(zones.end(), altzones.begin(), altzones.end());

        for(OutputZone* zone : zones) {
            ZoneId zoneid = zone->getZoneId();
            if(zoneid.getType() == ZoneId::BEAM) {
                deleteBeamZone(zone);
            }
        }
    }
    
    // remove laser from laser array
    lasers.erase(it);

    // delete laser object
    delete laser;
    
    //  delete laser settings files
    ofDirectory::removeDirectory("lasers/", true);
    
    // re-save remaining laser
    // TODO - Do we need to do that ?
    for(int i = 0; i<(int)lasers.size(); i++) {
        
        lasers[i]->laserIndex = i;
        lasers[i]->saveSettings();
        
    }
    
    
    
    return true;
}

ZoneId ManagerBase::addCanvasZone(const ofRectangle& rect) {
    return addCanvasZone(rect.x, rect.y, rect.width, rect.height);
    
}

ZoneId  ManagerBase :: addCanvasZone(float x, float y, float w, float h) {
    if(w<=0) w = canvasTarget.getWidth();
    if(h<=0) h = canvasTarget.getHeight();
    return canvasTarget.addInputZone(x, y, w, h);
}


ZoneId ManagerBase :: createNewBeamZone() {
   
    return beamZoneContainer.addBeamZone();
}
    


bool ManagerBase :: deleteCanvasZone(InputZone* inputZone) {
    if(inputZone==nullptr) return false;
    
    map<ZoneId, ZoneId>  changedzones = canvasTarget.removeZoneById(inputZone->getZoneId());
    for(Laser* laser : lasers) {
        laser->updateZones(changedzones);
        laser->removeZone(inputZone->getZoneId());
    }
    return true;
    
    
}

bool ManagerBase::deleteBeamZone(OutputZone* outputZone) {
    
    ZoneId zoneid = outputZone->getZoneId();
    
    bool changed = false;
    
    if(outputZone->getIsAlternate()) {
        for(Laser* laser : lasers) {
            changed = laser->removeAltZone(zoneid) || changed;
        }
        
    } else {
        map<ZoneId, ZoneId> changedZones = beamZoneContainer.removeZoneById(zoneid);
        // zone should actually only be in one of the lasers...
       
        for(Laser* laser : lasers) {
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
    
    for(Laser* laser : lasers) {
        laser->updateZones(changedzones);
    }
    return true;
    
}


void ManagerBase::createDefaultCanvasZone() {
    // check there aren't any zones yet?
    // create a zone equal to the width and height of the total output space
    addCanvasZone(0,0,canvasTarget.getWidth(),canvasTarget.getHeight());
    
}


void ManagerBase::drawDot( float x,  float y, const ofColor& col, float intensity, string profileLabel) {
    drawDot(glm::vec3(x, y, 0), col, intensity, profileLabel);
}
void ManagerBase::drawDot( const glm::vec2& p, const ofColor& col, float intensity, string profileLabel) {
    drawDot(glm::vec3(p.x, p.y, 0), col, intensity, profileLabel);
}


void ManagerBase::drawDot(const glm::vec3& p, const ofColor& col, float intensity, string profileLabel) {
    
    Dot* d = new Dot(getTransformed(p), col, intensity, profileLabel);
    currentShapeTarget->addShape(d, useClipRectangle, clipRectangle);
}



void ManagerBase::drawLine(float x1, float y1, float x2, float y2, const ofColor& col, string profileName){
    drawLine( glm::vec3(x1,y1,0), glm::vec3(x2,y2,0), col, profileName);
}

void ManagerBase::drawLine(const glm::vec2& start, const glm::vec2& end, const ofColor& col, string profileName){
    drawLine( glm::vec3(start.x, start.y, 0), glm::vec3(end.x, end.y, 0), col, profileName);
}

void ManagerBase::drawLine(const glm::vec3& start, const glm::vec3& end, const ofColor& col, string profileLabel) {
    
    Line* l = new Line(getTransformed(start), getTransformed(end), col, profileLabel);
    
    currentShapeTarget->addShape(l, useClipRectangle, clipRectangle);

}





void ManagerBase::drawCircle(const float& x, const float& y, const float& radius, const ofColor& col, string profileName){
    drawCircle(glm::vec3(x, y, 0), radius, col, profileName);
}
void ManagerBase::drawCircle(const glm::vec2& pos, const float& radius, const ofColor& col, string profileName){
    drawCircle(glm::vec3(pos.x, pos.y, 0), radius, col, profileName);
}
void ManagerBase::drawCircle(const glm::vec3 & centre, const float& radius, const ofColor& col,  string profileName){
    ofxLaser::Circle* c = new ofxLaser::Circle(centre, radius, col, profileName);
 
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
   

    ofxLaser::Polyline* p = getPolyFromPoints(points, colours, closed, profileName, brightness);
        
    if(p->getLength()>0.1) {
        //p->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
        currentShapeTarget->addShape(p, useClipRectangle, clipRectangle);
    } else {
        delete p;
    }
}

ofxLaser::Polyline* ManagerBase::getPolyFromPoints(const vector<glm::vec3>& points, const vector<ofColor>& colours, bool closed, string profileName, float brightness){
    
    tmpPoints = points;
    for(glm::vec3& v : tmpPoints) {
        v = getTransformed(v);
    }

    ofxLaser::Polyline* p =new ofxLaser::Polyline(tmpPoints, colours, profileName, brightness);
    
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
        ofxLaser::Polyline* poly = getPolyFromPoints(ofpolyline.getVertices(), colours[i], ofpolyline.isClosed(), profileName, brightness);
        poly->id = id;
        if(poly->getLength()>0.1) {
            currentShapeTarget->addShape(poly, useClipRectangle, clipRectangle);
        } else {
            delete poly;
        }
    }
    
    
    //ofxLaser::Polylines* polylines = new ofxLaser::Polylines();
    //TODO ******
    
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
    
    for(ofxLaser::Laser* laser : lasers) {
        laser->emptyDac.dontCalculate = dontCalculateDisconnected.get();
    }
    
    //if(useBitmapMask) laserMask.update();
    // delete all the shapes - all shape objects need a destructor!
    canvasTarget.deleteShapes();
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
    for(Laser* laser : lasers){
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
    
    canvasTarget.processShapes();
    
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
    vector<ObjectWithZoneId*>& zoneIds = canvasTarget.getZoneIds();
    
    
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
    
    
    for(ObjectWithZoneId* zoneIdObject : zoneIds) {
        InputZone* inputZone = canvasTarget.getInputZoneForZoneId(zoneIdObject->zoneId);

        zonesContent.push_back(ZoneContent());
        ZoneContent& zoneContent = zonesContent.back();
        vector<Shape*>& newshapes = zoneContent.shapes;
        zoneContent.zoneId = inputZone->getZoneId();
        zoneContent.sourceRectangle = inputZone->getRect();
        zoneContent.shapes = canvasTarget.getShapesForZoneId(inputZone->getZoneId());

    }
    for(ObjectWithZoneId* zoneIdObject : beamZoneContainer.getZoneIds()) {
        
        ShapeTargetBeamZone* beamzone = beamZoneContainer.getBeamZoneForZoneId(zoneIdObject->zoneId);

        
        zonesContent.push_back(ZoneContent());
        ZoneContent& zoneContent = zonesContent.back();
        vector<Shape*>& newshapes = zoneContent.shapes;
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
    Laser* laser = lasers.at(lasernum);
//    if(zonenum>=canvasTarget.zones.size()) {
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
    
    for(Laser* laser : lasers) {
        laser->useAlternate = useAltZones;
        
    }
    
}

void ManagerBase::hideContentDuringTestPatternChanged(bool& state) {
    
    for(Laser* laser : lasers) {
        laser->hideContentDuringTestPattern = hideContentDuringTestPattern;
    }
    saveSettings();
    
}

bool ManagerBase::loadSettings() {
    
    ofJson& json = loadedJson;
    string filename ="ofxLaser/laserSettings.json";
    if(ofFile(filename).exists()) {
        json = ofLoadJson(filename);
    }
    // if the json didn't load then this shouldn't do anything
    ofDeserialize(json, params);
  

    
    if(!beamZoneContainer.deserialize(json["beamzones"])) {
        // try old format
        beamZoneContainer.deserialize(json["beamZones"]);
    }
    canvasTarget.deserialize(json["canvastarget"]);
    
    
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
            dacAssigner.disconnectDacFromLaser(*lasers[i]);
        }
        Laser* laser = lasers[i];
        laser->loadSettings();
        
        
        // if the laser has a dac id saved in the settings,
        // tell the dacAssigner about it
        // if the dac isn't available, it'll make the data and store it
        // ready for when it loads
        if(!laser->dacLabel->empty()) {
            dacAssigner.assignToLaser(laser->dacLabel, *laser);
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
    for(Laser* laser : lasers) {
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
    canvasTarget.serialize(json["canvastarget"]);
    
    bool savesuccess = ofSavePrettyJson("ofxLaser/laserSettings.json", json);
    
    for(size_t i= 0; i<lasers.size(); i++) {
        savesuccess &= lasers[i]->saveSettings();
    }
    // TODO add laserMask saving to laser settings
    //savesuccess &= laserMask.saveSettings();
    
    // Save zones :
//    ofJson zoneJson;
    //TODO REPLACE THIS WITH canvasTarget.serialize
//
//    for(int i = 0; i<canvasTarget.zones.size(); i++) {
//        ofJson jsonGroup;
//        canvasTarget.zones[i]->serialize(jsonGroup);
//        zoneJson.push_back(jsonGroup);
//    }
//
//    ofSavePrettyJson("ofxLaser/zones.json", zoneJson);
    
    lastSaveTime = ofGetElapsedTimef();
    settingsNeedSave = false; 
    
    return savesuccess;
    
}



void ManagerBase :: serialize(ofJson& json) {

    ofJson& jsonLaserSettings = json; // ["managersettings"];
    ofSerialize(jsonLaserSettings, params);

    beamZoneContainer.serialize(jsonLaserSettings["beamzones"]);

    ofJson& jsonLasers = json["lasers"];
    for(size_t i= 0; i<lasers.size(); i++) {
        lasers[i]->serialize(jsonLasers[i]);
    }
    
}



bool ManagerBase::deserialize(ofJson& json) {
    
    //cout << json.dump(3) << endl;
    
    ofDeserialize(json, params);
    if((json.contains("canvaswidth")) && (json.contains("canvasheight"))){

    }

    if(!beamZoneContainer.deserialize(json["beamzones"])) {
        // try old format
        beamZoneContainer.deserialize(json["beamZones"]);
    }
 
    
    
    ofJson& jsonLasers = json["lasers"];
    
    // numLasers was saved in the json
    for(int i = 0; i<numLasers; i++) {
        
        // if we don't have a laser object already make one
        if(lasers.size()<i+1) {
            createAndAddLaser();
        } else {
            // if we already have a laser then make sure no dac is connected
            dacAssigner.disconnectDacFromLaser(*lasers[i]);
        }
        Laser* laser = lasers[i];
        laser->deserialize(jsonLasers[i]);
        
        // if the laser has a dac id saved in the settings,
        // tell the dacAssigner about it
        // if the dac isn't available, it'll make the data and store it
        // ready for when it loads
        if(!laser->dacLabel->empty()) {
            dacAssigner.assignToLaser(laser->dacLabel, *laser);
        }
        
    }
    // if we had more lasers to start with than we needed, then resize
    // the vector (shouldn't be needed but it doesn't hurt)
    lasers.resize(numLasers);
    
    // shouldn't be needed but hey
    disarmAllLasers();
    
    
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

Laser& ManagerBase::getLaser(int index){
    return *lasers.at(index);
};

std::vector<Laser*>& ManagerBase::getLasers(){
    return lasers;
};

bool ManagerBase::setTargetBeamZone(int index) {
    ShapeTargetBeamZone* beamzone = beamZoneContainer.getBeamZoneAtIndex(index);
    if(beamzone!=nullptr) {
        currentShapeTarget = beamzone;
        return true;
    } else {
        return false;
    }
}


ShapeTargetBeamZone* ManagerBase::getBeamZoneByIndex(int index) {
    return beamZoneContainer.getBeamZoneAtIndex(index);
    
}

bool ManagerBase::setTargetCanvas(int index) {
    // NB index for future use
    currentShapeTarget = &canvasTarget; 
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
    for(Laser* laser : lasers) {
        if(!laser->armed) return false;
    }
    return (lasers.size()==0)? false : true;
    
}

bool ManagerBase::areAllLasersUsingAlternateZones(){
    for(Laser* laser : lasers) {
        if(!laser->useAlternate)  return false;
    }
    return (lasers.size()==0)? false : true;
    
}
bool ManagerBase::hasAnyAltZones() {
    for(Laser* laser : lasers) {
        if(laser->hasAnyAltZones()) return true;
    }
    return false;
}

void ManagerBase::setAllAltZones() {
    for(Laser* laser : lasers) {
        laser->useAlternate = true;
    }
    
}
void ManagerBase::unSetAllAltZones(){
    for(Laser* laser : lasers) {
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

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

    
    params.add(canvasWidth.set("Canvas width", 800,0,5000));
    params.add(canvasHeight.set("Canvas height", 800,0,5000));
    canvasWidth.addListener(this, &ofxLaser::ManagerBase::canvasSizeChanged);
    canvasHeight.addListener(this, &ofxLaser::ManagerBase::canvasSizeChanged);
  
    
    //useBitmapMask = showBitmapMask = laserCanvasMaskOutlines = false;
    params.add(numLasers.set("numLasers", 0));
    params.add(useAltZones.set("Use alternative zones", false));
    params.add(dontCalculateDisconnected.set("Don't calculate disconnected", false));
    useAltZones.addListener(this, &ofxLaser::ManagerBase::useAltZonesChanged);
    
    testPatternGlobal = 1;
    testPatternGlobalActive = false; 
    
    //beepSound.load("Beep1.wav");
    
}

ManagerBase :: ~ManagerBase() {
    //ofLog(OF_LOG_NOTICE, "ofxLaser::Manager destructor");
    saveSettings();
    
    //    // clean up
    //    for(Laser* laser : lasers) {
    //        delete laser;
    //    }
    //    lasers.clear();
    //
    //    for(Zone* zone : zones) {
    //        delete zone;
    //    }
    //    zones.clear();
    //    // anything else?
    
}
void ManagerBase::canvasSizeChanged(int &size){
    laserMask.init(canvasWidth, canvasHeight);
}

void ManagerBase :: setCanvasSize(int w, int h){
    canvasWidth = w;
    canvasHeight = h;
    //laserMask.init(w,h);
    // canvasPreviewFbo.allocate(w, h, GL_RGB, 3);
//    ofRectangle canvasRectangle(0,0,w,h);
//    for(InputZone* zone : zones) {
//        zone->setConstrained(canvasRectangle);
//
//    }
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
    int index = it-lasers.begin();
    // hopefully should renumber current laser OK
    
    // remove laser from laser array
    lasers.erase(it);
    
    // TODO delete zones that are only assigned to this laser *************************
    if(deleteZones) {
        
        
    }
    
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

void ManagerBase::addZone(const ofRectangle& rect) {
    addZone(rect.x, rect.y, rect.width, rect.height);
    
}

void ManagerBase :: addZone(float x, float y, float w, float h) {
    if(w<=0) w = canvasWidth;
    if(h<=0) h = canvasHeight;
    zones.push_back(new InputZone( x, y, w, h));
    //zones.back()->loadSettings();
    renumberZones();
}

bool ManagerBase :: deleteZone(InputZone* zone) {
    
    vector<InputZone*>::iterator it = std::find(zones.begin(), zones.end(), zone);
    if(it == zones.end()) return false;
    
    
    zones.erase(it);
    renumberZones();
    
    for(Laser* laser : lasers) {
        laser->removeZone(zone->getIndex());
    }
    delete zone;
    
    return true;
    
}

void ManagerBase :: renumberZones() {
    for(int i = 0; i<zones.size(); i++ ) {
        zones[i]->setIndex(i);
    }
}


void ManagerBase::addZoneToLaser(unsigned int zonenum, unsigned int lasernum) {
    if(lasers.size()<=lasernum) {
        ofLog(OF_LOG_ERROR, "Invalid laser number passed to addZoneToLaser(...)");
        return;
    }
    if(zones.size()<=zonenum) {
        ofLog(OF_LOG_ERROR, "Invalid zone number passed to addZoneToLaser(...)");
        return;
    }
    InputZone* zone = zones[zonenum];
    lasers[lasernum]->addZone(zone->getIndex());
}

int ManagerBase::createDefaultZone() {
    // check there aren't any zones yet?
    // create a zone equal to the width and height of the total output space
    addZone(0,0,canvasWidth,canvasHeight);
    return (int)zones.size()-1;
    
}
void ManagerBase::drawLine(float x1, float y1, float x2, float y2, const ofColor& col, string profileName){
    drawLine( glm::vec3(x1,y1,0), glm::vec3(x2,y2,0), col, profileName);
    
}
void ManagerBase::drawLine(const glm::vec2& start, const glm::vec2& end, const ofColor& col, string profileName){
    drawLine( glm::vec3(start.x, start.y, 0), glm::vec3(end.x, end.y, 0), col, profileName);
    
}
void ManagerBase::drawLine(const glm::vec3& start, const glm::vec3& end, const ofColor& col, string profileLabel) {
    
    
    //Line l = new Line(gLProject(start), gLProject(end), ofFloatColor(col), 1, 1);
    Line* l = new Line(gLProject(start), gLProject(end), col, profileLabel);
    l->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
    shapes.push_back(l);
    
    
}


void ManagerBase::drawDot( float x,  float y, const ofColor& col, float intensity, string profileLabel) {
    drawDot(glm::vec3(x, y, 0), col, intensity, profileLabel);
}
void ManagerBase::drawDot( const glm::vec2& p, const ofColor& col, float intensity, string profileLabel) {
    drawDot(glm::vec3(p.x, p.y, 0), col, intensity, profileLabel);
}
void ManagerBase::drawDot(const glm::vec3& p, const ofColor& col, float intensity, string profileLabel) {
    
    Dot* d = new Dot(gLProject(p), col, intensity, profileLabel);
    d->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
    shapes.push_back(d);
}

void ManagerBase::drawPoly(const ofPolyline & poly, const ofColor& col, string profileName, float brightness){
    
    // quick error check to make sure our line has any data!
    // (useful for dynamically generated lines, or empty lines
    // that are often found in poorly compiled SVG files)
    
    if((poly.size()==0)||(poly.getPerimeter()<0.01)) return;
    
    ofPolyline& polyline = tmpPoly;
    polyline = poly;
    
    for(glm::vec3& v : polyline.getVertices()) {
        v = gLProject(v);
    }
    
    
    
    Polyline* p =new ofxLaser::Polyline(polyline, col*brightness, profileName);
    p->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
    shapes.push_back(p);
    
}

void ManagerBase::drawPoly(const ofPolyline & poly, std::vector<ofColor>& colours, string profileName, float brightness){
    
    // quick error check to make sure our line has any data!
    // (useful for dynamically generated lines, or empty lines
    // that are often found in poorly compiled SVG files)
    
    if((poly.size()==0)||(poly.getPerimeter()<0.1)) return;
    
    
    ofPolyline& polyline = tmpPoly;
    polyline = poly;
    
    for(glm::vec3& v : polyline.getVertices()) {
        v = gLProject(v);
    }
    if(brightness<1) {
        for(ofColor & c : colours) {
            c*=brightness;
            
        }
    }
    ofxLaser::Polyline* p =new ofxLaser::Polyline(polyline, colours, profileName);
    p->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
    shapes.push_back(p);
    
}

void ManagerBase::drawPolyFromPoints(const vector<glm::vec3>& points, const vector<ofColor>& colours, string profileName, float brightness){
    
    if(points.size()==0) return;
    tmpPoints = points;
    for(glm::vec3& v : tmpPoints) {
        v = gLProject(v);
    }
    //vector<ofColor>* colourPointer = &colours;
    
    //    if(brightness<1) {
    //        tmpColours = colours;
    //        for(ofColor & c : tmpColours) {
    //            c*=brightness;
    //
    //        }
    //    }
    ofxLaser::Polyline* p =new ofxLaser::Polyline(tmpPoints, colours, profileName, brightness);
    
    if(p->polylinePointer->getPerimeter()>0.1) {
        p->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
        shapes.push_back(p);
    } else {
        delete p;
    }
}


void ManagerBase::drawCircle(const float& x, const float& y, const float& radius, const ofColor& col,string profileName){
    drawCircle(glm::vec3(x, y, 0), radius, col, profileName);
}
void ManagerBase::drawCircle(const glm::vec2& pos, const float& radius, const ofColor& col,string profileName){
    drawCircle(glm::vec3(pos.x, pos.y, 0), radius, col, profileName);
}
void ManagerBase::drawCircle(const glm::vec3 & centre, const float& radius, const ofColor& col,string profileName){
    ofxLaser::Circle* c = new ofxLaser::Circle(centre,radius, col, profileName);
    c->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
    ofPolyline& polyline = c->polyline;
    
    for(glm::vec3& v : polyline.getVertices()) {
        v = gLProject(v);
    }
    shapes.push_back(c);
    
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
    
    dacAssigner.update();
    
    for(ofxLaser::Laser* laser : lasers) {
        laser->emptyDac.dontCalculate = dontCalculateDisconnected.get();
    }
    
    zonesChanged = false;
    
    //if(useBitmapMask) laserMask.update();
    // delete all the shapes - all shape objects need a destructor!
    for(size_t i= 0; i<shapes.size(); i++) {
        delete shapes[i];
    }
    shapes.clear();
    
    // updates all the zones. If zone->update returns true, then
    // it means that the zone has changed.
    bool updateZoneRects = false;
    for(size_t i= 0; i<zones.size(); i++) {
        if(zones[i]->update()) {
           updateZoneRects  = true;
        }
    }
    
    
    
    bool dacDisconnected = false;
    // update all the lasers which clears the points,
    // and updates all the zone settings
    for(Laser* laser : lasers){
        laser->update(updateZoneRects); // clears the points
        if(laser->hasDac()) {
            int laserstatus = laser->getDac()->getStatus();
            
            if(laser->getDac()->hasStatusChanged() && (laserstatus!=OFXLASER_DACSTATUS_GOOD)) {
                // MAKE BEEP
                dacDisconnected = true;
            }
        }
    }
    if(dacDisconnected)  {
        if(!beepSound.isPlaying()) {
            beepSound.play();
        }
    }
    zonesChanged = updateZoneRects;
    
    if(useAltZones && (!hasAnyAltZones())) {
        useAltZones.set(false);
    }
    
}

void ManagerBase::send(){
    
    // for when we reinstate canvas masks, we'll need to rework this
//    if(laserCanvasMaskOutlines) {
//        vector<ofPolyline*> polylines = laserMask.getLaserMaskShapes();
//        for(ofPolyline* poly : polylines) {
//            ofPoint centre= poly->getCentroid2D();
//            poly->translate(-centre);
//            poly->scale(1.01,1.01);
//            poly->translate(centre);
//        }
//        for(size_t i= 0; (i<zones.size()) && ((zoneMode==OFXLASER_ZONE_MANUAL) || (i<1));i++) {
//            setTargetZone((int)i);
//            for(ofPolyline* poly:polylines) {
//
//                drawPoly(*poly, ofColor::cyan);
//            }
//
//        }
//
//        for(ofPolyline* poly : polylines)  ofxLaser::Factory::releasePolyline(poly);
//    }
    
    
    // here's where the magic happens.
    // 1 :
    // figure out which zones to send the shapes to
    // and send them. When the zones get the shape, they transform them
    // into local zone space.
    
    //vector<deque<Shape*>> shapesByZoneIndex;
    vector<ZoneContent> zonesContent;
    
    
    zonesContent.resize(zones.size());
    
    if(zoneMode!=OFXLASER_ZONE_OPTIMISE) {
        for(size_t j = 0; j<zones.size(); j++) {
            InputZone& inputzone = *zones[j];
//            z.shapes.clear();
            //deque<Shape*>& newshapes = shapesByZoneIndex[j];
            ZoneContent& zoneContent = zonesContent[j];
            vector<Shape*>& newshapes = zoneContent.shapes;
            zoneContent.zoneIndex = j;
            zoneContent.sourceRectangle = inputzone.getRect(); 
            
            for(Shape* shape : shapes) { //size_t i= 0; i<shapes.size(); i++) {
               // Shape* s = shapes[i];
                // if (zone should have shape) then
                // TODO zone intersect shape test
                if(zoneMode == OFXLASER_ZONE_AUTOMATIC) {
                    if(shape->intersectsRect(inputzone.getRect())) {
                        newshapes.push_back(shape);
                    }
                    
                } else if(zoneMode == OFXLASER_ZONE_MANUAL) {
                    if((shape->getTargetZone() == (int)j) && (shape->intersectsRect(inputzone.getRect()))) {
                        newshapes.push_back(shape);
                    }
                }
            }
        }
    } else {
        // TODO : 	OPTIMISE ALGORITHM GOES HERE
        // figure out which shapes are in each zone
        // if a shape is entirely enclosed in a zone, then
        // mark the shape as such
        // if a shape is partially enclosed in a zone then
        // mark it as such
        
        // LOGIC IS AS FOLLOWS :
        // if a shape is entirely in one or more zones, then :
        // put it in the zone with the nearest other shape
        // UNLESS that zone already has too many shapes (greater than half
        // if we're dealing with two zones)
        // IF the shape is only in that zone, and it is already full, have
        // a look through the other shapes in the zone
        // 		IF the shape is in another zone
        // 			FIND the zone that
        //				a) has the nearest other shape
        //				b) AND isn't too full
        
        
        
        
    }
    
    // 2 :
    // The lasers go through each of their zones, and pull out each shape
    // it'd need to be in zone space, then as each shape is converted to points, that's
    // when we'd do the warp for the output space.
    
    // So - the shapes need to be sorted in output space but their points need to be
    // calculated at zone space. Otherwise the perspective distortion won't look right in
    // terms of brightness distribution.
    for(size_t i= 0; i<lasers.size(); i++) {
        
        Laser& p = *lasers[i];
        
        p.send(zonesContent, globalBrightness, NULL);// useBitmapMask?laserMask.getPixels():NULL);
        
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
void ManagerBase::sendRawPoints(const std::vector<ofxLaser::Point>& points, int lasernum, int zonenum){
     ofLog(OF_LOG_NOTICE, "ofxLaser::Manager::sendRawPoints(...) point count : "+ofToString(points.size()));
    if(lasernum>=lasers.size()) {
        ofLogError("Invalid laser number sent to ofxLaser::ManagerBase::sendRawPoints");
        return;
    }
    Laser* laser = lasers.at(lasernum);
    if(zonenum>=zones.size()) {
        ofLogError("Invalid zone number sent to ofxLaser::ManagerBase::sendRawPoints");
        return;
    }
    laser->sendRawPoints(points, zonenum, globalBrightness);
    
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

bool ManagerBase::loadSettings() {
    ofJson& json = loadedJson;
    string filename ="ofxLaser/laserSettings.json";
    if(ofFile(filename).exists()) {
        json = ofLoadJson("ofxLaser/laserSettings.json");
    }
    // if the json didn't load then this shouldn't do anything
    ofDeserialize(json, params);
    if((json.contains("canvasWidth")) && (json.contains("canvasHeight"))){
        
        
    }
        
    
    // reset the global brightness setting, despite what was in the settings.
    globalBrightness = 0.2;
    
    
    // load the zone config files
    
    ofJson zonesJson;
    filename ="ofxLaser/zones.json";
    if(ofFile(filename).exists()) {
        zonesJson= ofLoadJson(filename);
    }
    zones.clear();
    for(ofJson& zoneJson : zonesJson) {
        zones.push_back(new InputZone());
        zones.back()->deserialize(zoneJson);
        
    }
    
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
    
    // shouldn't be needed but hey
    disarmAllLasers();
    
    
    
    
    if(zones.size()==0) {
        
    } else {
        renumberZones();
    }
    
    
    return true;
    
    
    
}

bool ManagerBase::saveSettings() {
    
    // update the number of lasers for the laserNum param
    // (it's automatically saved with the params)
    if(numLasers!=lasers.size()) numLasers = lasers.size();
    
    ofJson json;
    ofSerialize(json, params);
//    json["canvasWidth"] = canvasWidth;
//    json["canvasHeight"] = canvasHeight;
    
    bool savesuccess = ofSavePrettyJson("ofxLaser/laserSettings.json", json);
    
    for(size_t i= 0; i<lasers.size(); i++) {
        savesuccess &= lasers[i]->saveSettings();
    }
    // TODO add laserMask saving to laser settings
    //savesuccess &= laserMask.saveSettings();
    
    // Save zones :
    ofJson zoneJson;
    for(int i = 0; i<zones.size(); i++) {
        ofJson jsonGroup;
        zones[i]->serialize(jsonGroup);
        zoneJson.push_back(jsonGroup);
    }
    
    ofSavePrettyJson("ofxLaser/zones.json", zoneJson);
    
    
    return savesuccess;
    
}

// converts openGL coords to screen coords //
ofPoint ManagerBase::gLProject(ofPoint p) {
    return gLProject(p.x, p.y, p.z);
    
}
ofPoint ManagerBase::gLProject( float x, float y, float z ) {
    
    ofRectangle rViewport = ofGetCurrentViewport();
    
    glm::mat4 modelview, projection;
    glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(modelview));
    glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projection));
    glm::mat4 mat = ofGetCurrentOrientationMatrix();
    mat = glm::inverse(mat);
    mat *=projection * modelview;
    glm::vec4 dScreen4 = mat * glm::vec4(x,y,z,1.0);
    glm::vec3 dScreen = glm::vec3(dScreen4) / dScreen4.w;
    dScreen += glm::vec3(1.0) ;
    dScreen *= 0.5;
    
    dScreen.x *= rViewport.width;
    dScreen.x += rViewport.x;
    
    dScreen.y *= rViewport.height;
    dScreen.y += rViewport.y;
    
    return ofPoint(dScreen.x, dScreen.y, 0.0f);// - offset;
    
}


Laser& ManagerBase::getLaser(int index){
    return *lasers.at(index);
};

std::vector<Laser*>& ManagerBase::getLasers(){
    return lasers;
};



//
//Zone& ManagerBase::getZone(int zonenum) {
//    // TODO bounds check?
//    return *zones.at(zonenum);
//    
//}
InputZone* ManagerBase::getZone(int zonenum) {
    // TODO bounds check?
    if((zonenum>=0) && (zonenum<zones.size())) {
        return zones.at(zonenum);
    } else {
        return nullptr;
    }
    
}

int ManagerBase::getNumZones() {
    return (int)zones.size();
}

bool ManagerBase::setTargetZone(unsigned int zone){  // only for OFX_ZONE_MANUAL
    if(zone>=zones.size()) return false;
    else if(zone<0) return false;
    else {
        targetZone = zone;
        return true;
    }
}
int ManagerBase::getTargetZone() {
    return targetZone;
}

bool ManagerBase::setZoneMode(ofxLaserZoneMode newmode) {
    zoneMode = newmode;
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

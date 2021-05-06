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
    
	guiProjectorPanelWidth = 320;
	guiSpacing = 8;
	dacStatusBoxHeight = 88;
	dacStatusBoxSmallWidth = 160;
	showInputPreview = true;
	lockInputZones = false;
	
    initAndLoadSettings();
    
    // if no projectors loaded make one and add a zone
    if(projectors.size()==0) {
        createAndAddProjector();
        //addZone(0,0,800,800);
        if(zones.size()==0) createDefaultZone();
        for(Projector* projector : projectors) {
            projector->addZone(zones[0],800,800);
        }
        showProjectorSettings = true;
        
    }
	
}
ManagerBase :: ~ManagerBase() {
	//ofLog(OF_LOG_NOTICE, "ofxLaser::Manager destructor");
	saveSettings();
    
//    // clean up
//    for(Projector* projector : projectors) {
//        delete projector;
//    }
//    projectors.clear();
//
//    for(Zone* zone : zones) {
//        delete zone;
//    }
//    zones.clear();
//    // anything else?
    
}

void ManagerBase :: setCanvasSize(int w, int h){
	width = w;
	height = h;
	laserMask.init(w,h);
    previewFbo.allocate(w, h, GL_RGBA, 3);
}

void ManagerBase::addProjector(DacBase& dac) {
    ofLogError("Projectors are no longer set up in code! Add them within the app instead.");
    throw;
    
}

void ManagerBase::addProjector() {
    ofLogError("Projectors are no longer set up in code! Add them within the app instead.");
    throw;
    
}
void ManagerBase::createAndAddProjector() {
	
	// create and add new projector object
	
	Projector* projector = new Projector(projectors.size());
	projectors.push_back(projector);
    
    /*
	// If we have no zones set up then create a big default zone.
	if(zones.size()==0) {
		addZoneToProjector(createDefaultZone(), (int)projectors.size()-1);
	} else if(zones.size()==1) {
		addZoneToProjector(0, (int)projectors.size()-1);
	}*/
	
	projector->setDefaultHandleSize(defaultHandleSize);
    
    // TODO should this be here?
    projector->init();
    
}

bool ManagerBase :: deleteProjector(Projector* projector) {
    
    bool deleteZones = true;
    
    // check if projector exists and isn't null
    if(projector == nullptr) return false;

    if(find(projectors.begin(), projectors.end(), projector) == projectors.end()) return false;
  
    // disconnect dac
    dacAssigner.disconnectDacFromProjector(*projector);

    vector<Projector*> :: iterator it = find(projectors.begin(), projectors.end(), projector);
    int index = it-projectors.begin();
    // hopefully should renumber current projector OK
  
    // remove projector from projector array
    projectors.erase(it);
    
    // delete zones that are only assigned to this projector *************************
    if(deleteZones) {
        
        
    }
    
    // delete projector object
    delete projector;
    
    //  delete projector settings files
    ofDirectory::removeDirectory("projectors/", true);
   
    // re-save remaining projectors
    // TODO - Do we need to do that ?
    for(int i = 0; i<(int)projectors.size(); i++) {
        
        projectors[i]->projectorIndex = i;
        projectors[i]->saveSettings();
        
    }
    
    
    
    return true;
}

void ManagerBase::addZone(const ofRectangle& rect) {
	addZone(rect.x, rect.y, rect.width, rect.height);
	
}

void ManagerBase :: addZone(float x, float y, float w, float h) {
	if(w<=0) w = width;
	if(h<=0) h = height;
	zones.push_back(new Zone( x, y, w, h));
	//zones.back()->loadSettings();
    renumberZones();
}

bool ManagerBase :: deleteZone(Zone* zone) {
    
    vector<Zone*>::iterator it = std::find(zones.begin(), zones.end(), zone);
    if(it == zones.end()) return false;
    
   
    zones.erase(it);
    renumberZones();
    
    for(Projector* projector : projectors) {
        projector->removeZone(zone);
    }
    delete zone;
    
    return true;
    
}

void ManagerBase :: renumberZones() {
    for(int i = 0; i<zones.size(); i++ ) {
        zones[i]->setIndex(i);
    }
}


void ManagerBase::addZoneToProjector(unsigned int zonenum, unsigned int projnum) {
	if(projectors.size()<=projnum) {
		ofLog(OF_LOG_ERROR, "Invalid projector number passed to AddZoneToProjector(...)");
		return;
	}
	if(zones.size()<=zonenum) {
		ofLog(OF_LOG_ERROR, "Invalid zone number passed to AddZoneToProjector(...)");
		return;
	}
	
	projectors[projnum]->addZone(zones[zonenum], width, height);
}

int ManagerBase::createDefaultZone() {
	// check there aren't any zones yet?
	// create a zone equal to the width and height of the total output space
	addZone(0,0,width,height);
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

void ManagerBase::drawPoly(const ofPolyline & poly, const ofColor& col, string profileName){
	
	// quick error check to make sure our line has any data!
	// (useful for dynamically generated lines, or empty lines
	// that are often found in poorly compiled SVG files)
	
	if((poly.size()==0)||(poly.getPerimeter()<0.01)) return;
	
	ofPolyline& polyline = tmpPoly;
	polyline = poly;
	
	for(glm::vec3& v : polyline.getVertices()) {
		v = gLProject(v);
	}
	
	
	
	Polyline* p =new ofxLaser::Polyline(polyline, col, profileName);
	p->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
	shapes.push_back(p);
	
}

void ManagerBase::drawPoly(const ofPolyline & poly, std::vector<ofColor>& colours, string profileName){
	
	// quick error check to make sure our line has any data!
	// (useful for dynamically generated lines, or empty lines
	// that are often found in poorly compiled SVG files)
	
	if((poly.size()==0)||(poly.getPerimeter()<0.1)) return;
	
	
	ofPolyline& polyline = tmpPoly;
	polyline = poly;
	
	for(glm::vec3& v : polyline.getVertices()) {
		v = gLProject(v);
	}
	
	ofxLaser::Polyline* p =new ofxLaser::Polyline(polyline, colours, profileName);
	p->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
	shapes.push_back(p);
	
	
	
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

void ManagerBase:: update(){
	if(doArmAll) armAllProjectors();
	if(doDisarmAll) disarmAllProjectors();
	zonesChanged = false;
	
	if(useBitmapMask) laserMask.update();
	// delete all the shapes - all shape objects need a destructor!
	for(size_t i= 0; i<shapes.size(); i++) {
		delete shapes[i];
	}
	shapes.clear();
	
	// updates all the zones. If zone->update returns true, then
	// it means that the zone has changed.
	bool updateZoneRects = false;
	for(size_t i= 0; i<zones.size(); i++) {
		//zones[i]->setVisible(currentProjector==-1);
        zones[i]->setEditable((!lockInputZones));
		updateZoneRects = zones[i]->update() | updateZoneRects  ; // is this dangerous? Optimisation may stop the function being called.
	}
	
	// update all the projectors which clears the points,
	// and updates all the zone settings
	for(size_t i= 0; i<projectors.size(); i++) {
		projectors[i]->update(updateZoneRects); // clears the points
	}
	zonesChanged = updateZoneRects;
	
}

void ManagerBase::beginDraw() {
    ofViewport((ofGetWidth()-width)/-2, (ofGetHeight()-height)/-2, ofGetWidth(), ofGetHeight()) ;
    ofPushMatrix();
    ofTranslate((ofGetWidth()-width)/2, (ofGetHeight()-height)/2);
    
}
void ManagerBase::endDraw() {
    ofPopMatrix();
    ofViewport(0,0,ofGetWidth(), ofGetHeight());
}

void ManagerBase::send(){
	
	if(laserMasks) {
		vector<ofPolyline*> polylines = laserMask.getLaserMaskShapes();
		for(ofPolyline* poly : polylines) {
			ofPoint centre= poly->getCentroid2D();
			poly->translate(-centre);
			poly->scale(1.01,1.01);
			poly->translate(centre);
		}
		for(size_t i= 0; (i<zones.size()) && ((zoneMode==OFXLASER_ZONE_MANUAL) || (i<1));i++) {
			setTargetZone((int)i);
			for(ofPolyline* poly:polylines) {
				
				drawPoly(*poly, ofColor::cyan);
			}
			
		}
		
		for(ofPolyline* poly : polylines)  ofxLaser::Factory::releasePolyline(poly);
	}
	
	
	// here's where the magic happens.
	// 1 :
	// figure out which zones to send the shapes to
	// and send them. When the zones get the shape, they transform them
	// into local zone space.
	
	if(zoneMode!=OFXLASER_ZONE_OPTIMISE) {
		for(size_t j = 0; j<zones.size(); j++) {
			Zone& z = *zones[j];
			z.shapes.clear();
			
			for(size_t i= 0; i<shapes.size(); i++) {
				Shape* s = shapes[i];
				// if (zone should have shape) then
				// TODO zone intersect shape test
				if(zoneMode == OFXLASER_ZONE_AUTOMATIC) {
					bool shapeAdded = z.addShape(s);
				} else if(zoneMode == OFXLASER_ZONE_MANUAL) {
					if(s->getTargetZone() == (int)j) z.addShape(s);
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
	// The projectors go through each of their zones, and pull out each shape
	// it'd need to be in zone space, then as each shape is converted to points, that's
	// when we'd do the warp for the projector space.
	
	// So - the shapes need to be sorted in projector space but their points need to be
	// calculated at zone space. Otherwise the perspective distortion won't look right in
	// terms of brightness distribution.
	for(size_t i= 0; i<projectors.size(); i++) {
		
		Projector& p = *projectors[i];
		
		p.send(useBitmapMask?laserMask.getPixels():NULL, globalBrightness);
		
	}
}


int ManagerBase :: getProjectorPointRate(unsigned int projectornum ){
	return projectors.at(projectornum)->getPointRate();
}

float ManagerBase :: getProjectorFrameRate(unsigned int projectornum ){
	if((projectornum>=0) && (projectornum<projectors.size())) {
		return projectors.at(projectornum)->getFrameRate();
	} else return 0;
}
void ManagerBase::sendRawPoints(const std::vector<ofxLaser::Point>& points, int projectornum, int zonenum){
	// ofLog(OF_LOG_NOTICE, "ofxLaser::Manager::sendRawPoints(...) point count : "+ofToString(points.size()));
	Projector* proj = projectors.at(projectornum);
   
	proj->sendRawPoints(points, &getZone(zonenum), globalBrightness);
	
}


// DEPRECATED, showAdvanced parameter now redundant
void ManagerBase::initGui(bool showAdvanced) {
    ofLogError("Projectors are no longer set up in code! Add them within the app instead.");
    throw;
}

void ManagerBase::initAndLoadSettings() {
    if(initialised) {
        ofLogError("ofxLaser::Manager::initAndLoadSettings() called twice - NB you no longer need to call this in your code, it happens automatically");
        return ;
    }
    ofxLaser::UI::setupGui();
   
	params.setName("Laser");
	params.add(globalBrightness.set("Global brightness", 0.1,0,1));
	params.add(showProjectorSettings.set("Edit projector", false));
	params.add(testPattern.set("Global test pattern", 0,0,9));
	testPattern.addListener(this, &ofxLaser::ManagerBase::testPatternAllProjectors);
	
	interfaceParams.setName("Interface");
	interfaceParams.add(lockInputZones.set("Lock input zones", true));
	interfaceParams.add(showInputPreview.set("Show preview", true));
	interfaceParams.add(showOutputPreviews.set("Show path previews", true));
	//interfaceParams.add(useBitmapMask.set("Use bitmap mask", false));
	//interfaceParams.add(showBitmapMask.set("Show bitmap mask", false));
	//interfaceParams.add(laserMasks.set("Laser mask shapes", false));
    useBitmapMask = showBitmapMask = laserMasks = false;
	params.add(interfaceParams);
	
	customParams.setName("Custom");
	params.add(customParams);
    params.add(numLasers.set("numLasers", 0));
	
    loadSettings();
    
	showInputPreview = true;
    testPattern = 0;
    initialised = true;
	
}

void ManagerBase::armAllProjectorsListener() {
	doArmAll = true;
}

void ManagerBase::disarmAllProjectorsListener(){
	doDisarmAll = true;
}
void ManagerBase::armAllProjectors() {
	
	for(size_t i= 0; i<projectors.size(); i++) {
		projectors[i]->armed = true;
	}
	doArmAll = false;
}
void ManagerBase::disarmAllProjectors(){
	for(size_t i= 0; i<projectors.size(); i++) {
		projectors[i]->armed = false;
	}
	doDisarmAll = false;
}
void ManagerBase::testPatternAllProjectors(int &pattern){
	for(size_t i= 0; i<projectors.size(); i++) {
		projectors[i]->testPattern = testPattern;
	}
}

bool ManagerBase::loadSettings() {
    ofJson json = ofLoadJson("laserSettings.json");
    
    // if the json didn't load then this shouldn't do anything
    ofDeserialize(json, params);
    
    // reset the global brightness setting, despite what was in the settings.
    globalBrightness = 0.1;

    
    // load the zone config files - [zone config also knows which projectors have which zones] < do they tho?
    
    ofJson zonesJson = ofLoadJson("zones.json");
    for(ofJson& zoneJson : zonesJson) {
        zones.push_back(new Zone());
        zones.back()->deserialize(zoneJson);
        
    }
    
   
    
    // NOW load the projectors
    
    // numLasers was saved in the json
    for(int i = 0; i<numLasers; i++) {
        
        // if we don't have a projector object already make one
        if(projectors.size()<i+1) {
            createAndAddProjector();
        } else {
            // if we already have a projector then make sure no dac is connected
            dacAssigner.disconnectDacFromProjector(*projectors[i]);
        }
        Projector* projector = projectors[i];
        projector->loadSettings(zones);
        
        // if the projector has a dac id saved in the settings,
        // tell the dacAssigner about it
        if(!projector->dacId->empty()) {
            dacAssigner.assignToProjector(projector->dacId, *projector);
        }
        
    }
    // if we had more projectors to start with than we needed, then resize
    // the vector (shouldn't be needed but it doesn't hurt)
    projectors.resize(numLasers);

    // shouldn't be needed but hey
    disarmAllProjectors();
    
   
    
    
    if(zones.size()==0) {
//        createDefaultZone();
//        for(Projector* projector : projectors) {
//            projector->addZone(zones[0],800,800);
//        }
    } else {
        renumberZones();
    }

    
    return true;



}

bool ManagerBase::saveSettings() {
	 
    // update the number of lasers for the laserNum param
    // (it's automatically saved with the params)
    numLasers = projectors.size();
    
    ofJson json;
    ofSerialize(json, params);
    

    bool savesuccess = ofSavePrettyJson("laserSettings.json", json);
    
	for(size_t i= 0; i<projectors.size(); i++) {
        savesuccess &= projectors[i]->saveSettings();
//        savesuccess &= projectors[i]->saveZoneSettings();
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
    
    ofSavePrettyJson("zones.json", zoneJson);
   
    // SAVE all the ProjectorZones
    
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


Projector& ManagerBase::getProjector(int index){
	return *projectors.at(index);
};

std::vector<Projector*>& ManagerBase::getProjectors(){
	return projectors;
};


bool ManagerBase::setGuideImage(string filename){
	return guideImage.load(filename);
	
	
}

Zone& ManagerBase::getZone(int zonenum) {
	// TODO bounds check?
	return *zones.at(zonenum);
	
}

int ManagerBase::getNumZones() {
	return (int)zones.size();
}

Zone* ManagerBase::getSelectedZone() {
    for(Zone* zone : zones) {
        if(zone->selected) {
            return zone;
        }
    }
    return nullptr;
    
}

bool ManagerBase::setTargetZone(unsigned int zone){  // only for OFX_ZONE_MANUAL
	if(zone>=zones.size()) return false;
	else if(zone<0) return false;
	else {
		targetZone = zone;
		return true;
	}
}

bool ManagerBase::setZoneMode(ofxLaserZoneMode newmode) {
	zoneMode = newmode;
	return true;
}

bool ManagerBase::isProjectorArmed(unsigned int i){
	if((i<0) || (i>=projectors.size())){
		return false;
	} else {
		return projectors[i]->armed;
	}
	
}


bool ManagerBase::areAllLasersArmed(){
	for(Projector* projector : projectors) {
		if(!projector->armed) return false;
	}
	return (projectors.size()==0)? false : true;
	
}

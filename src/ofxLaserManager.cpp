//
//  ofxLaserManager.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserManager.h"

using namespace ofxLaser;

Manager * Manager :: laserManager = NULL;

Manager * Manager::instance() {
	if(laserManager == NULL) {
		laserManager = new Manager();
	}
	return laserManager;
}


Manager :: Manager() : dacAssigner(*DacAssigner::instance()) {
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
	showPreview = true;
	showZones = false;
	currentProjector = -1;
	guiIsVisible = true;
    
    initAndLoadSettings();
    
    if(projectors.size()==0) {
        createAndAddProjector();
        //addZone(0,0,800,800);
        
        showProjectorSettings = true;
    }
	
}
Manager :: ~Manager() {
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

void Manager :: setCanvasSize(int w, int h){
	width = w;
	height = h;
	laserMask.init(w,h);
    previewFbo.allocate(w, h, GL_RGBA, 3);
}

void Manager::addProjector(DacBase& dac) {
    ofLogError("Projectors are no longer set up in code! Add them within the app instead.");
    throw;
    
}

void Manager::addProjector() {
    ofLogError("Projectors are no longer set up in code! Add them within the app instead.");
    throw;
    
}
void Manager::createAndAddProjector() {
	
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

bool Manager :: deleteProjector(Projector* projector) {
    
    bool deleteZones = true;
    
    // check if projector exists and isn't null
    if(projector == nullptr) return false;

    if(find(projectors.begin(), projectors.end(), projector) == projectors.end()) return false;
  
    // disconnect dac
    dacAssigner.disconnectDacFromProjector(*projector);

    vector<Projector*> :: iterator it = find(projectors.begin(), projectors.end(), projector);
    int index = it-projectors.begin();
    // hopefully should renumber current projector OK
    if(currentProjector==index) currentProjector =-1;
    else if(currentProjector>index) currentProjector--;
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

void Manager::addZone(const ofRectangle& rect) {
	addZone(rect.x, rect.y, rect.width, rect.height);
	
}

void Manager :: addZone(float x, float y, float w, float h) {
	if(w<=0) w = width;
	if(h<=0) h = height;
	zones.push_back(new Zone( x, y, w, h));
	//zones.back()->loadSettings();
    renumberZones();
}

bool Manager :: deleteZone(Zone* zone) {
    
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

void Manager :: renumberZones() {
    for(int i = 0; i<zones.size(); i++ ) {
        zones[i]->setIndex(i);
    }
}


void Manager::addZoneToProjector(unsigned int zonenum, unsigned int projnum) {
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

int Manager::createDefaultZone() {
	// check there aren't any zones yet?
	// create a zone equal to the width and height of the total output space
	addZone(0,0,width,height);
	return (int)zones.size()-1;
	
}
void Manager::drawLine(float x1, float y1, float x2, float y2, const ofColor& col, string profileName){
    drawLine( glm::vec3(x1,y1,0), glm::vec3(x2,y2,0), col, profileName);
    
}
void Manager::drawLine(const glm::vec2& start, const glm::vec2& end, const ofColor& col, string profileName){
    drawLine( glm::vec3(start.x, start.y, 0), glm::vec3(end.x, end.y, 0), col, profileName);
    
}
void Manager::drawLine(const glm::vec3& start, const glm::vec3& end, const ofColor& col, string profileLabel) {
	
		
	//Line l = new Line(gLProject(start), gLProject(end), ofFloatColor(col), 1, 1);
	Line* l = new Line(gLProject(start), gLProject(end), col, profileLabel);
	l->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
	shapes.push_back(l);
	
	
}


void Manager::drawDot( float x,  float y, const ofColor& col, float intensity, string profileLabel) {
    drawDot(glm::vec3(x, y, 0), col, intensity, profileLabel);
}
void Manager::drawDot( const glm::vec2& p, const ofColor& col, float intensity, string profileLabel) {
    drawDot(glm::vec3(p.x, p.y, 0), col, intensity, profileLabel);
}
void Manager::drawDot(const glm::vec3& p, const ofColor& col, float intensity, string profileLabel) {

	Dot* d = new Dot(gLProject(p), col, intensity, profileLabel);
	d->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
	shapes.push_back(d);
}

void Manager::drawPoly(const ofPolyline & poly, const ofColor& col, string profileName){
	
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

void Manager::drawPoly(const ofPolyline & poly, std::vector<ofColor>& colours, string profileName){
	
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
void Manager::drawCircle(const float& x, const float& y, const float& radius, const ofColor& col,string profileName){
    drawCircle(glm::vec3(x, y, 0), radius, col, profileName);
}
void Manager::drawCircle(const glm::vec2& pos, const float& radius, const ofColor& col,string profileName){
    drawCircle(glm::vec3(pos.x, pos.y, 0), radius, col, profileName);
}
void Manager::drawCircle(const glm::vec3 & centre, const float& radius, const ofColor& col,string profileName){
	ofxLaser::Circle* c = new ofxLaser::Circle(centre,radius, col, profileName);
	c->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
	shapes.push_back(c);
	
}

void Manager:: update(){
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
		zones[i]->setVisible(currentProjector==-1);
		updateZoneRects = updateZoneRects | zones[i]->update(); // is this dangerous? Optimisation may stop the function being called.
	}
	
	// update all the projectors which clears the points,
	// and updates all the zone settings
	for(size_t i= 0; i<projectors.size(); i++) {
		projectors[i]->update(updateZoneRects); // clears the points
	}
	zonesChanged = updateZoneRects;
	
}


void Manager::send(){
	
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

void Manager:: drawUI(bool expandPreview){
    
    drawPreviews(expandPreview);
    
    ofxLaser::UI::updateGui();
    ofxLaser::UI::startGui();
    
    drawLaserGui();
    ofxLaser::UI::render();
	
}

void Manager :: drawPreviews(bool expandPreview) {
    
    
    // if expandPreview is true, then we expand the preview area to the
    // maximum space that we have available.
    
    
    // figure out the top and bottom section height
    
    // If the height of the laser is > 2/3 of the screen height (including spacing)
    // then shrink it to be 2/3 of the height.
    
    
    
    // we're showing the previews then they go at the top
    

    int lowerSectionHeight = 310;
    
    int thirdOfHeight = (ofGetHeight()-(guiSpacing*3))/3;
  
    if(lowerSectionHeight>thirdOfHeight) lowerSectionHeight = thirdOfHeight;
    
    // showPreview determines whether we show the preview
    // laser graphics on screen or not.
    if(showPreview) {
        
        ofPushStyle();
        
        // work out the scale for the preview...
        // default scale is 1 with an 8 pixel margin
        previewScale = 1;
        previewOffset = glm::vec2(guiSpacing,guiSpacing);
        
        if(height>(thirdOfHeight*2)) {
            previewScale = (float)(thirdOfHeight*2) / (float)height;
        }
        // but if we're viewing a projector warp ui
        // then shrink the preview down and move it underneath
        if(currentProjector>=0) {
            int positionY = 800 + guiSpacing*2;
            if((thirdOfHeight*2)+(guiSpacing*2) < positionY ) {
                positionY = (thirdOfHeight*2)+(guiSpacing*2);
                
            }
            previewOffset = glm::vec2(guiSpacing,positionY);
            previewScale = (float)lowerSectionHeight/(float)height;
            
            
            // but if we're expanding the preview, then work out the scale
            // to fill the whole screen
        } else if(expandPreview) {
            previewOffset = glm::vec2(0,0);
            previewScale = (float)ofGetWidth()/(float)width;
            if(height*previewScale>ofGetHeight()) {
                previewScale = (float)ofGetHeight()/(float)height;
            }
            
        }
        
        renderPreview();
        
        // this renders the input zones in the graphics source space
        for(size_t i= 0; i<zones.size(); i++) {
            zones[i]->setVisible(showZones);
           // zones[i]->setActive(showZones && (currentProjector<0));
            
            zones[i]->offset.set(previewOffset);
            zones[i]->scale = previewScale;
            
            zones[i]->draw();
        }
        
        ofPushMatrix();
        laserMask.setOffsetAndScale(previewOffset,previewScale);
        laserMask.draw(showBitmapMask);
        ofTranslate(previewOffset);
        ofScale(previewScale, previewScale);
        
        
        ofPopMatrix();
        ofPopStyle();
        
       
        
        
    }
    
    
    ofPushStyle();
    
    // if none of the projectors are selected then draw as many as we can on screen
    if(currentProjector==-1) {
        ofPushMatrix();
        float scale = 1 ;
        if((lowerSectionHeight+guiSpacing)*(int)projectors.size()>ofGetWidth()-(guiSpacing*2)) {
            scale = ((float)ofGetWidth()-(guiSpacing*2))/((float)(lowerSectionHeight+guiSpacing)*(float)projectors.size());
            //ofScale(scale, scale);
        }
        
        ofTranslate(guiSpacing,(height*previewScale)+(guiSpacing*2));
        
        for(size_t i= 0; i<projectors.size(); i++) {
            if((!expandPreview)&&(showPathPreviews)) {
                ofFill();
                ofSetColor(0);
                ofRectangle projectorPreviewRect(((lowerSectionHeight*scale) +guiSpacing)*i,0,lowerSectionHeight*scale, lowerSectionHeight*scale);
                ofDrawRectangle(projectorPreviewRect);
                projectors[i]->drawLaserPath(projectorPreviewRect);
            }
            // disables the warp interfaces
            projectors[i]->hideWarpGui();
        }
        
        ofPopMatrix();
        
        // if we're not filling the preview to fit the screen, draw the projector
        // gui elements
        
        
    } else  {
        // ELSE we have a currently selected projector, so draw the various UI elements
        // for that...
        
        for(size_t i= 0; i<projectors.size(); i++) {
            if((int)i==currentProjector) {
                
                ofFill();
                ofSetColor(0);
                float size = 800;
                if(size>thirdOfHeight*2) size = thirdOfHeight*2;
                if(expandPreview) size =  (float)ofGetHeight()-(guiSpacing*2);
                
                ofDrawRectangle(guiSpacing,guiSpacing,size,size);
                projectors[i]->showWarpGui();
                projectors[i]->drawWarpUI(guiSpacing,guiSpacing,size,size);
                projectors[i]->drawLaserPath(guiSpacing,guiSpacing,size,size);
                
                
            } else {
                projectors[i]->hideWarpGui();
            }
            
        }
        
    }
    
    ofPopStyle();
    

    if((!expandPreview) && (guiIsVisible)) {
        
        // if this is the current projector or we have 2 or fewer projectors, then render the gui
        if(!showProjectorSettings) {
            
    
            
            int w = dacStatusBoxSmallWidth;
            int x = ofGetWidth() - 220; // gui.getPosition().x-w-guiSpacing;
            
            // draw all the status boxes but small
            
            for(size_t i= 0; i<projectors.size(); i++) {

                //projectors[i]->renderStatusBox(x, i*(dacStatusBoxHeight+guiSpacing)+10, w,dacStatusBoxHeight);
            }
            
            
        }
        
        
    }
    
    
}
void Manager :: renderPreview() {
    

    if((previewFbo.getWidth()!=width*previewScale) || (previewFbo.getHeight()!=height*previewScale)) {
       // previewFbo.clear();
        previewFbo.allocate(width*previewScale, height*previewScale, GL_RGBA, 3);
    }
    
    previewFbo.begin();
    
    ofClear(0,0,0,0);
    ofPushStyle();
    // ofEnableSmoothing();
    ofPushMatrix();
    //ofTranslate(previewOffset);
    ofScale(previewScale, previewScale);
    
    
    // draw outline of laser output area
    ofSetColor(0);
    ofFill();
    ofDrawRectangle(0,0,width,height);
    ofSetColor(50);
    ofNoFill();
    ofDrawRectangle(0,0,width,height);
    
    // Draw laser graphics preview ----------------
    ofMesh mesh;
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    mesh.clear();
    mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    // Draw the preview laser graphics, with zones overlaid
    for(size_t i= 0; i<shapes.size(); i++) {
        shapes[i]->addPreviewToMesh(mesh);
    }
    
    ofRectangle laserRect(0,0,width, height);
    if(useBitmapMask) {
        const vector<glm::vec3>& points = mesh.getVertices();
        std::vector<ofFloatColor>& colours = mesh.getColors();
        
        for(size_t i= 0;i<points.size(); i++ ){
            
            ofFloatColor& c = colours.at(i);
            const glm::vec3& p = points[i];
            
            
            float brightness;
            
            if(laserRect.inside(p)) {
                brightness = laserMask.getBrightness(p.x, p.y);
            } else {
                brightness = 0;
            }
            
            c.r*=brightness;
            c.g*=brightness;
            c.b*=brightness;
            
        }
    }
    
    ofSetLineWidth(1.5);
    mesh.draw();
    
    std::vector<ofFloatColor>& colours = mesh.getColors();
    
    for(size_t i= 0; i<colours.size(); i++) {
        colours[i].r*=0.4;
        colours[i].g*=0.4;
        colours[i].b*=0.4;
    }
    
    ofSetLineWidth(4);
    mesh.draw();
    
    ofDisableBlendMode();
    // ofDisableSmoothing();
    
    ofPopMatrix();
    
    ofPopStyle();
    previewFbo.end();
    previewFbo.draw(previewOffset);
}

bool Manager ::toggleGui(){
	guiIsVisible = !guiIsVisible;
	return guiIsVisible;
}
void Manager ::setGuiVisible(bool visible){
	guiIsVisible = visible;
}
bool Manager::isGuiVisible() {
	return guiIsVisible;
}


int Manager :: getProjectorPointRate(unsigned int projectornum ){
	return projectors.at(projectornum)->getPointRate();
}

float Manager :: getProjectorFrameRate(unsigned int projectornum ){
	if((projectornum>=0) && (projectornum<projectors.size())) {
		return projectors.at(projectornum)->getFrameRate();
	} else return 0;
}
void Manager::sendRawPoints(const std::vector<ofxLaser::Point>& points, int projectornum, int zonenum){
	// ofLog(OF_LOG_NOTICE, "ofxLaser::Manager::sendRawPoints(...) point count : "+ofToString(points.size()));
	Projector* proj = projectors.at(projectornum);
   
	proj->sendRawPoints(points, &getZone(zonenum), globalBrightness);
	
}


void Manager::nextProjector() {
	currentProjector++;
	if(currentProjector>=(int)projectors.size()) currentProjector=-1;
	
}


void Manager::previousProjector() {
	currentProjector--;
	if(currentProjector<-1) currentProjector=(int)projectors.size()-1;
	
}
// DEPRECATED, showAdvanced parameter now redundant
void Manager::initGui(bool showAdvanced) {
    ofLogError("Projectors are no longer set up in code! Add them within the app instead.");
    throw;
}

void Manager::initAndLoadSettings() {
    if(initialised) {
        ofLogError("ofxLaser::Manager::initAndLoadSettings() called twice - NB you no longer need to call this in your code, it happens automatically");
        return ;
    }
    ofxLaser::UI::setupGui();
   
	params.setName("Laser");
	params.add(globalBrightness.set("Master Intensity", 0.1,0,1));
	params.add(showProjectorSettings.set("Edit Projector", false));
	params.add(testPattern.set("Test Pattern", 0,0,9));
	testPattern.addListener(this, &ofxLaser::Manager::testPatternAllProjectors);
	
	interfaceParams.setName("Interface");
	interfaceParams.add(showZones.set("Show zones", false));
	interfaceParams.add(showPreview.set("Show preview", true));
	interfaceParams.add(showPathPreviews.set("Show path previews", true));
	interfaceParams.add(useBitmapMask.set("Use bitmap mask", false));
	interfaceParams.add(showBitmapMask.set("Show bitmap mask", false));
	interfaceParams.add(laserMasks.set("Laser mask shapes", false));
	
	params.add(interfaceParams);
	
	customParams.setName("Custom");
	params.add(customParams);
    params.add(numLasers.set("numLasers", 0));
	
    loadSettings();
    
	showPreview = true;
    testPattern = 0;
    initialised = true;
	
}

void Manager::setDefaultHandleSize(float size) {
	defaultHandleSize = size;
	for(Projector* projector : projectors) {
		projector->setDefaultHandleSize(defaultHandleSize);
	}
	
}

void Manager::addCustomParameter(ofAbstractParameter& param){
	customParams.add(param);
}


void Manager::armAllProjectorsListener() {
	doArmAll = true;
}

void Manager::disarmAllProjectorsListener(){
	doDisarmAll = true;
}
void Manager::armAllProjectors() {
	
	for(size_t i= 0; i<projectors.size(); i++) {
		projectors[i]->armed = true;
	}
	doArmAll = false;
}
void Manager::disarmAllProjectors(){
	for(size_t i= 0; i<projectors.size(); i++) {
		projectors[i]->armed = false;
	}
	doDisarmAll = false;
}
void Manager::testPatternAllProjectors(int &pattern){
	for(size_t i= 0; i<projectors.size(); i++) {
		projectors[i]->testPattern = testPattern;
	}
}

bool Manager::loadSettings() {
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
        createDefaultZone();
        for(Projector* projector : projectors) {
            projector->addZone(zones[0],800,800);
        }
    } else {
        renumberZones();
    }

    
    return true;



}

bool Manager::saveSettings() {
	 
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
    savesuccess &= laserMask.saveSettings();
    
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
ofPoint Manager::gLProject(ofPoint p) {
	return gLProject(p.x, p.y, p.z);
	
}
ofPoint Manager::gLProject( float x, float y, float z ) {
	
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


Projector& Manager::getProjector(int index){
	return *projectors.at(index);
};

std::vector<Projector*>& Manager::getProjectors(){
	return projectors;
};


bool Manager::setGuideImage(string filename){
	return guideImage.load(filename);
	
	
}

bool Manager::togglePreview(){
	showPreview = !showPreview;
	return showPreview;
};


Zone& Manager::getZone(int zonenum) {
	// TODO bounds check?
	return *zones.at(zonenum);
	
}

int Manager::getNumZones() {
	return (int)zones.size();
}

Zone* Manager::getSelectedZone() {
    for(Zone* zone : zones) {
        if(zone->selected) {
            return zone;
        }
    }
    return nullptr;
    
}

bool Manager::setTargetZone(unsigned int zone){  // only for OFX_ZONE_MANUAL
	if(zone>=zones.size()) return false;
	else if(zone<0) return false;
	else {
		targetZone = zone;
		return true;
	}
}

bool Manager::setZoneMode(ofxLaserZoneMode newmode) {
	zoneMode = newmode;
	return true;
}

bool Manager::isProjectorArmed(unsigned int i){
	if((i<0) || (i>=projectors.size())){
		return false;
	} else {
		return projectors[i]->armed;
	}
	
}


bool Manager::areAllLasersArmed(){
	for(Projector* projector : projectors) {
		if(!projector->armed) return false;
	}
	return (projectors.size()==0)? false : true;
	
}

void Manager::drawLaserGui() {
    
    ofxLaser::Manager& laser = *this;
    // auto mainSettings = ofxImGui::Settings();
    
    int mainpanelwidth = 270;
    int projectorpanelwidth = 280;
    int spacing = 8;
    
//    ImGuiWindowFlags window_flags = 0;
//
//    window_flags |= ImGuiWindowFlags_NoMove;
//    window_flags |= ImGuiWindowFlags_NoResize;
//    window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
//    window_flags |= ImGuiWindowFlags_NoNav;
   
    
    
    // calculate x position of main window
    int x = ofGetWidth() - mainpanelwidth - spacing;
    // if we're also showing the projector settings, make space
    // TODO max 2 projectors
    if(laser.showProjectorSettings){
        //int numProjectors =laser.getNumProjectors();
        //if(numProjectors>2) numProjectors = 1;
        int numProjectors = 1;
        x-=(numProjectors*projectorpanelwidth);
        x-=(spacing*numProjectors);
    }
    
    UI::startWindow("Settings", ImVec2(x, spacing), ImVec2(mainpanelwidth, 0), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize, true );
    
    
    // some custom styles - to do put in a theme?
//    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 5.0f)); // 1 Spacing between items
//    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(6.0f, 6.0f)); // 2 gap between element and label
    
    
    // START BIG BUTTONS
    UI::largeItemStart();

    // the arm and disarm buttons
    bool useRedButton =laser.areAllLasersArmed();
    if(useRedButton) UI::secondaryColourButtonStart();
    // change the colour for the arm all button if we're armed
    int buttonwidth = (mainpanelwidth-(spacing*3))/2;
    if(ImGui::Button("ARM ALL LASERS", ImVec2(buttonwidth, 0.0f) )) {
        laser.armAllProjectors();
    }
    if(useRedButton) UI::secondaryColourButtonEnd();
    
    ImGui::SameLine();
    if(ImGui::Button("DISARM ALL LASERS",  ImVec2(buttonwidth, 0.0f))) {
        laser.disarmAllProjectors();
    }
    
      // change width of slider vs label
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 120.0f);
    
    // add intensity slide
    //UI::addFloatAsIntSlider(laser.masterIntensity, 100);
    ImGui::PushItemWidth(mainpanelwidth-(spacing*2));
    float multiplier = 100;
    int value = laser.globalBrightness*multiplier;
    if (ImGui::SliderInt("##int", &value, laser.globalBrightness.getMin()*multiplier, laser.globalBrightness.getMax()*multiplier, "GLOBAL BRIGHTNESS %d")) {
        laser.globalBrightness.set((float)value/multiplier);
        
    }
    ImGui::PopItemWidth();
    
    string label;
    if(projectors.size()>0) {
        label = showProjectorSettings? "CLOSE PROJECTOR SETTINGS" : "OPEN PROJECTOR SETTINGS";
        
        if(ImGui::Button(label.c_str())) {
            showProjectorSettings = !showProjectorSettings;
        }
    }
        
    
    if(ImGui::Button("ADD PROJECTOR", ImVec2(buttonwidth, 0.0f))) {
        createAndAddProjector();
    }
    ImGui::SameLine();
    if(ImGui::Button("ADD ZONE", ImVec2(buttonwidth, 0.0f))) {
        addZone();
        showZones = true;
    }
    // END BIG BUTTONS
    UI::largeItemEnd();
  
    for(int i = 0; i<laser.getNumProjectors(); i++) {
        ofxLaser::Projector& projector = laser.getProjector(i);
        string label = ofToString(i+1) + " ARMED";
        ImGui::Checkbox(label.c_str(), (bool*)&projector.armed.get());
        
    }

    UI::addIntSlider(laser.testPattern);
    
    UI::addParameterGroup(laser.interfaceParams);
    
    
    if(showZones && (currentProjector ==-1)) {
        
        ImGui::Separator();
        
        for(Zone* zone : zones) {
        
//            ImGui::Separator();
//            ImGui::Text("%s Settings", zone->displayLabel.c_str());
//            for(Projector* projector : projectors) {
//                bool checked = projector->hasZone(zone);
//                string label = projector->getLabel()+ " "+zone->displayLabel;
//                if(ImGui::Checkbox(label.c_str(), &checked)) {
//                    ofLogNotice("Checkbox " + ofToString(checked));
//                    if(checked) {
//                        projector->addZone(zone, width, height);
//                    } else {
//                        projector->removeZone(zone);
//                    }
//
//                }
//
//            }
           
            // the arm and disarm buttons
            //int buttonwidth = (mainpanelwidth-(spacing*3))/2;
            string buttonlabel ="DELETE "+zone->displayLabel;
            string modallabel ="Delete "+zone->displayLabel+"?";
            
            UI::secondaryColourButtonStart();
            if(ImGui::Button(buttonlabel.c_str())) {
                ImGui::OpenPopup(modallabel.c_str());
            }
            UI::secondaryColourButtonEnd();
            
            if (ImGui::BeginPopupModal(modallabel.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Are you sure? All the zone settings will be deleted.\n\n");
                ImGui::Separator();

                UI::secondaryColourButtonStart();
                if (ImGui::Button("DELETE", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    deleteZone(zone);
                    
                }
                UI::secondaryColourButtonEnd();
                
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    
                }
                ImGui::EndPopup();
            }

             
            
            
        
        }
    }
    
    if(laser.customParams.size()>0) {
        ImGui::Separator();
        ImGui::Text("CUSTOM PARAMETERS");
        UI::addParameterGroup(laser.customParams); 

    }
    
    
   // ImGui::PopStyleVar(2);
    
    ImGui::PopItemWidth();
    
    UI::endWindow();
   // ImGui::End();
    
    
    // show projector settings :
    
    if(laser.showProjectorSettings) {
        x+=mainpanelwidth+spacing;
        // CODE THAT DOES A MAXIMUM OF 2 PROJECTOR SETTINGS
//        if(projectors.size()<3) {
//            for(ofxLaser::Projector* projector : laser.getProjectors()) {
//
//                drawProjectorPanel(projector, projectorpanelwidth, spacing, x);
//                x+=projectorpanelwidth+spacing;
//
//            }
//        } else {
            int projectorIndexToShow = currentProjector;
            if(projectorIndexToShow ==-1) projectorIndexToShow = 0;
            drawProjectorPanel(&getProjector(projectorIndexToShow), projectorpanelwidth, spacing, x);
            
        //}
        
    }
    
    // Show projector zone settings mute / solo / etc
    if(currentProjector!=-1)  {
        
        
        // PROJECTOR ZONE SETTINGS
        Projector* projector = projectors[currentProjector];
        
        glm::vec2 projectorZonePos = previewOffset + (previewScale*glm::vec2(width, 0));
        
        UI::startWindow("Projector zones", ImVec2(projectorZonePos.x+spacing, projectorZonePos.y), ImVec2(0,0), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
    
        
        for(ProjectorZone* projectorZone : projector->projectorZones) {
            
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, projectorZone->enabled?0.5f:1.0f);
            
            string muteLabel = "M##"+projectorZone->getLabel();
            if(ImGui::Button(muteLabel.c_str(), ImVec2(20,20))) {
                projectorZone->muted = !projectorZone->muted;
            };
            ImGui::PopStyleVar();
            
            ImGui::SameLine();
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, projectorZone->soloed?1.0f:0.5f);
            string soloLabel = "S##"+projectorZone->getLabel();
            if(ImGui::Button(soloLabel.c_str(), ImVec2(20,20))){
                projectorZone->soloed = !projectorZone->soloed;
            }
            ImGui::PopStyleVar();
            ImGui::SameLine();
            ImGui::Text("%s",projectorZone->getLabel().c_str());
            
        }

        for(Zone* zone : zones) {
            bool checked = projector->hasZone(zone);
            
            if(ImGui::Checkbox(zone->displayLabel.c_str(), &checked)) {
                if(checked) {
                    projector->addZone(zone, width, height);
                } else {
                    
                    projector->removeZone(zone);
                }
                
            }
        }
        
        UI::addCheckbox(projector->hideContentDuringTestPattern);
        
        UI::endWindow();
        
        
        // Projector Masks
        
        
        
        
        
        for(ProjectorZone* projectorZone : projector->projectorZones) {
        
            if(projectorZone->zoneTransform.getSelected()) {
                ImVec2 pos(projectorZone->zoneTransform.getRight(),projectorZone->zoneTransform.getCentre().y);
                ImVec2 size(200,0);
                UI::startWindow(projectorZone->getLabel()+"##"+projector->getLabel(),pos, size, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);


                UI::addParameterGroup(projectorZone->zoneTransform.params);
                ImGui::Text("Edge masks");
                UI::addFloatSlider(projectorZone->bottomEdge);
                UI::addFloatSlider(projectorZone->topEdge);
                UI::addFloatSlider(projectorZone->leftEdge);
                UI::addFloatSlider(projectorZone->rightEdge);
                
                UI::endWindow();
            }
        }
        
       
        
       
        
        
        
        
    }
    
}


void Manager :: drawProjectorPanel(ofxLaser::Projector* projector, float projectorpanelwidth, float spacing, float x) {
    
//    ImGuiWindowFlags window_flags = 0;
//
//    window_flags |= ImGuiWindowFlags_NoMove;
//    window_flags |= ImGuiWindowFlags_NoResize;
//    window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
//    window_flags |= ImGuiWindowFlags_NoNav;
//
//    ImGui::SetNextWindowSize(ImVec2(projectorpanelwidth,0), ImGuiCond_Appearing);
//    ImGui::SetNextWindowPos(ImVec2(x,spacing));
//
//
//    ImGui::Begin(projector->getLabel().c_str(), NULL, window_flags);
    
    UI::startWindow(projector->getLabel(), ImVec2(x, spacing), ImVec2(projectorpanelwidth,0), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize, true);
    
    
    UI::largeItemStart();
    // change width of slider vs label
    ImGui::PushItemWidth(140);
    
    UI::addCheckbox(projector->armed);
    //UI::addFloatAsIntSlider(projector->intensity, 100);
    ImGui::PushItemWidth(projectorpanelwidth-(spacing*2));
    float multiplier = 100;
    int value = projector->intensity*multiplier;
    if (ImGui::SliderInt("##int", &value, projector->intensity.getMin()*multiplier, projector->intensity.getMax()*multiplier, "BRIGHTNESS %d")) {
        projector->intensity.set((float)value/multiplier);
        
    }
    ImGui::PopItemWidth();
    
    
    UI::largeItemEnd();
    
    UI::addIntSlider(projector->testPattern);
    //UI::addButton(resetDac);
    
    
    // THE DAC STATUS COLOUR - TO BE IMPROVED
    ImGui::Separator();
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 size = ImVec2(19,19); // ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
      
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImU32 col = projector->getDacConnectedState() ?  ImGui::GetColorU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)): ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        
        draw_list->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), col);
        ImGui::InvisibleButton("##gradient2", size - ImVec2(2,2));
       
    }
    
    ImGui::SameLine();
    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0,2));
    //ImGui::Text("DAC:");
    //ImGui::SameLine();
    // TODO add a method in projector that tells us if
    // it's using the empty dac?
    if((projector->dac == &projector->emptyDac) && (projector->dacId.get()!="") ) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
        ImGui::Text("Waiting for %s", projector->dacId.get().c_str());
        ImGui::PopStyleVar();
    } else {
        ImGui::Text("%s", projector->getDacLabel().c_str());
    }

    ImGui::SameLine(projectorpanelwidth-30);
    if(ImGui::Button("^", ImVec2(19,19))){
        
        
    }
    
    // DAC LIST -------------------------------------------------------------
    
    ImGui::PushItemWidth(projectorpanelwidth-spacing*2);
    
    // get the dacs from the dacAssigner
    const vector<DacData>& dacList = dacAssigner.getDacList();
//	string comboLabel;
//	if(dacList.size()>0) comboLabel = projector->getDacLabel();
//	else comboLabel = "No DACs discovered";
	    
    if (ImGui::ListBoxHeader("##listbox", MIN(5, MAX(1,dacList.size())))){
        
        if(dacList.empty()) {
         
            ImGui::Selectable("No laser controllers found", false, ImGuiSelectableFlags_Disabled );
   
        } else {
                
                
            // add a combo box item for every element in the list
            for(const DacData& dacdata : dacList) {
                
                // get the dac label (usually type + unique ID)
                string itemlabel = dacdata.label;
                
                ImGuiSelectableFlags selectableflags = 0;
                
                if(!dacdata.available) {
                   // ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
                   //itemlabel += " - no longer available";
                    selectableflags|=ImGuiSelectableFlags_Disabled;
                } else {
                   //
                }
                // if this dac is assigned to a projector, show which projector
                //  - this could be done at the other end?
                
                
                
                if (ImGui::Selectable(itemlabel.c_str(), (dacdata.assignedProjector == projector), selectableflags)) {
                    // then select dac
                    // TODO : show a warning yes / no if :
                    //      - we already are connected to a DAC
                    //      - the chosen DAC is already being used by another projector
                    dacAssigner.assignToProjector(dacdata.label, *projector);
                }
                
                if(dacdata.assignedProjector!=nullptr) {
                    ImGui::SameLine(projectorpanelwidth - 100);
                    string label =" > " + dacdata.assignedProjector->getLabel();
                    ImGui::Text("%s",label.c_str());
                }
                
                //ImGui::PopStyleVar();
            }
        }
        //    if (is_selected)
        //       ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        //ImGui::EndCombo();
        ImGui::ListBoxFooter();
    }
    //ImGui::PopStyleVar();
    ImGui::PopItemWidth();
    
    if(projector->dac != &projector->emptyDac) {
        if(ImGui::Button("Disconnect DAC")) {
            dacAssigner.disconnectDacFromProjector(*projector);
        }
        ImGui::SameLine();
    }
    
    if(ImGui::Button("Refresh DAC list")) {
        dacAssigner.updateDacList();
        
    }
    // ----------------------------------------------
    
    

    ImGui::Separator();
    ImGui::Text("OUTPUT / ZONE SETTINGS");

    // ZONES
    UI::addCheckbox(projector->flipX);
    UI::addCheckbox(projector->flipY);
    
    // FINE OUTPUT SETTINGS
    bool treevisible = ImGui::TreeNode("Fine position adjustments");
        
    UI::toolTip("These affect all output zones for this projector and can be used to re-align projectors if they have moved slightly since setting them up");
    if (treevisible){
        
       /// ImGui::PushItemWidth(projectorpanelwidth-120);
        
        ofParameter<float>& param = projector->rotation;
        if(ImGui::DragFloat("Rotation", (float*)&param.get(), 0.01f,-10,10)) { //  param.getMin(), param.getMax())) {
            param.set(param.get());
            
        }
        //UI::addFloatSlider(projector->rotation, "%.2f", 2.0);
        if(projector->rotation!=0) {
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::Button("Reset")) projector->rotation = 0;
        }
        
        ofParameter<glm::vec2>& param2 = projector->outputOffset;
        if(ImGui::DragFloat2("Position", (float*)&param2.get().x, 0.01f, -50.0f,50.0f)) { //  param.getMin(), param.getMax())) {
            param2.set(param2.get());
            
        }
        //UI::addFloat2Slider(projector->outputOffset, "%.2f", 2.0);
        
        
        
        glm::vec2 zero2;
        if(projector->outputOffset.get()!=zero2) {
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::Button("Reset")) projector->outputOffset.set(zero2);
        }
       // ImGui::PopItemWidth();
        ImGui::TreePop();
    }

   
    
//
//
//    if(ImGui::TreeNode("Zone edge masks")){
//        //for(size_t i = 0; i< projector->projectorZones.size(); i++) {
//        for(ProjectorZone* projectorZone : projector->projectorZones) {
//            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
//            if(ImGui::TreeNode(projectorZone->getLabel().c_str())){
//
//                UI::addFloatSlider(projectorZone->bottomEdge);
//                UI::addFloatSlider(projectorZone->topEdge);
//                UI::addFloatSlider(projectorZone->leftEdge);
//                UI::addFloatSlider(projectorZone->rightEdge);
//
//                ImGui::TreePop();
//            }
//
//
//        }
//        ImGui::TreePop();
//
//    }
//
//    if(ImGui::TreeNode("Zone Warp Settings")){
//        for(ProjectorZone* projectorZone : projector->projectorZones) {
//            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
//            if(ImGui::TreeNode(projectorZone->getLabel().c_str())){
//                UI::addParameterGroup(projectorZone->zoneTransform.params);
//
//                ImGui::TreePop();
//            }
//
//
//        }
//        ImGui::TreePop();
//
//    }
//
    
    // TODO IMPLEMENT PROJECTOR PROFILES
    /*
    // PROJECTOR PROFILE
    ImGui::Separator();
    ImGui::Text("PROJECTOR PROFILE");
    
    // TODO :
    // check if the settings are different from the preset, if they are
    // show a "save" button, also save as?
    //
    // when an option is selected, update all the params
    
    if (ImGui::BeginCombo("##combo", "LightSpace Unicorn RGB11000")) // The second parameter is the label previewed before opening the combo.
    {
        
        if (ImGui::Selectable("LightSpace Unicorn RGB11000", true)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("OPT PD4", false)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("OPT PD25", false)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("LaserCube 2W", false)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("LaserCube 1W", false)) {
            //item_current = items[n];
        }
        
        //    if (is_selected)
        //       ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        ImGui::EndCombo();
    }
    */
    
    // SCANNER PROFILE SETTINGS
    
    
    ImGui::Separator();
    ImGui::Text("SCANNER SETTINGS");
    
    // TODO :
    // check if the settings are different from the preset, if they are
    // show a "save" button, also save as?
    //
    // when an option is selected, update all the params
    /*
    if (ImGui::BeginCombo("Scanner profile (placeholder)", "DT50")) // The second parameter is the label previewed before opening the combo.
    {
        
        if (ImGui::Selectable("DT50", true)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("EMS8000", false)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("PT-A40HP", false)) {
            //item_current = items[n];
        }
        if (ImGui::Selectable("LaserCube", false)) {
            //item_current = items[n];
        }
        
        //    if (is_selected)
        //       ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        ImGui::EndCombo();
    }
    */
    // SCANNER SETTINGS
    UI::addFloatSlider(projector->colourChangeShift);
    UI::toolTip("Shifts the laser colours to match the scanner position (AKA blank shift)");
    
    
    UI::addFloatSlider(projector->moveSpeed, "%.2f", 3.0f);
    UI::toolTip("How quickly the mirrors move between shapes");
    
    UI::addIntSlider(projector->shapePreBlank);
    UI::toolTip("The length of time that the laser is switched off and held at the beginning of a shape");
    UI::addIntSlider(projector->shapePreOn);
    UI::toolTip("The length of time that the laser is switched on and held at the beginning of a shape");
    UI::addIntSlider(projector->shapePostOn);
    UI::toolTip("The length of time that the laser is switched on and held at the end of a shape");
    UI::addIntSlider(projector->shapePostBlank);
    UI::toolTip("The length of time that the laser is switched off and held at the end of a shape");
    
    ImGui::Text("Scanner profiles");
    UI::toolTip("There are three profiles for different qualities of laser effects. Unless otherwise specified, the default profile is used. The fast setting is good for long curvy lines, the high detail setting is good for complex pointy shapes.");
    
    bool firsttreeopen = true;
    for (auto & renderProfile : projector->renderProfiles) {
        
        RenderProfile& profile = renderProfile.second;
        
        string name =renderProfile.first;
        ImGui::SetNextItemOpen(firsttreeopen, ImGuiCond_Once);
        firsttreeopen = false;
        
        if(ImGui::TreeNode(profile.params.getName().c_str())){
            
            UI::addFloatSlider(profile.speed, "%.1f", 3.0f);
            UI::addFloatSlider(profile.acceleration, "%.2f", 3.0f);
            UI::addIntSlider(profile.dotMaxPoints);
            UI::addFloatSlider(profile.cornerThreshold);
            
            ImGui::TreePop();
        }
        
    }
    
    // POINT RATE
    UI::addIntSlider(projector->pps);
    
    
    
    // COLOUR SETTINGS
    ImGui::Separator();
    ImGui::Text("COLOUR");
   
    if(ImGui::TreeNode("Colour calibration")){
        
        UI::addParameterGroup(projector->colourParams);
        
        ImGui::TreePop();
    }
    
   
    
    ImGui::Separator();
    ImGui::Text("ADVANCED SETTINGS");

    // ADVANCED
    UI::addCheckbox(projector->laserOnWhileMoving);
    if(ImGui::TreeNode("Advanced")){
        UI::addParameterGroup(projector->advancedParams);
        ImGui::TreePop();
    }
    
    
    // the arm and disarm buttons
    //int buttonwidth = (mainpanelwidth-(spacing*3))/2;
    UI::secondaryColourButtonStart();
    if(ImGui::Button("DELETE PROJECTOR")) {
        ImGui::OpenPopup("Delete?");
    }
    UI::secondaryColourButtonEnd();
    
    
    if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure? All the zone settings will be deleted.\n\n");
        ImGui::Separator();

        //static int dummy_i = 0;
        //ImGui::Combo("Combo", &dummy_i, "Delete\0Delete harder\0");

//        static bool dont_ask_me_next_time = false;
//        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
//        ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
//        ImGui::PopStyleVar();

        
        UI::secondaryColourButtonStart();
        
        if (ImGui::Button("DELETE", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            deleteProjector(projector);
            
        }
        UI::secondaryColourButtonEnd();
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::EndPopup();
    }

     
    
  
    
    
    ImGui::PopItemWidth();
    
    //ImGui::End();
    UI::endWindow();
}


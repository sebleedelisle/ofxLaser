//
//  ofxLaserManager.cpp
//  ofxLaserRewrite
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


Manager :: Manager() {
	ofLog(OF_LOG_NOTICE, "ofxLaser::Manager constructor");
	if(laserManager == NULL) {
		laserManager = this;
	} else {
		ofLog(OF_LOG_WARNING, "Multiple ofxLaser::Manager instances created");
	}
	width = 800;
	height = 800;
	guiProjectorPanelWidth = 320;
	guiSpacing = 8;
	dacStatusBoxHeight = 88; 
	showPreview = true;
	showZones = false;
	currentProjector = -1;
    guiIsVisible = true;
	
    ofAddListener(ofEvents().windowResized, this, &Manager::updateScreenSize, OF_EVENT_ORDER_BEFORE_APP);
	
}
Manager :: ~Manager() {
    ofLog(OF_LOG_NOTICE, "ofxLaser::Manager destructor");
    ofRemoveListener(ofEvents().windowResized, this, &Manager::updateScreenSize);
}

void Manager :: setup(int w, int h){
	width = w;
	height = h;
    laserMask.init(w,h);
}


void Manager::addProjector(DacBase& dac) {

	// create and add new projector object
	
	Projector* projector = new Projector("Projector"+ofToString(projectors.size()+1), dac);
	projectors.push_back(projector);
	// If we have no zones set up then create a big default zone.
	if(zones.size()==0) {
		addZoneToProjector(createDefaultZone(), projectors.size()-1);
		
	}
	Projector& proj = *projectors.back();

	//proj.gui->setPosition(width+320,10);
	
	//updateScreenSize();
	
}

void Manager::addZone(const ofRectangle& rect) {
	addZone(rect.x, rect.y, rect.width, rect.height);
	
}

void Manager :: addZone(float x, float y, float w, float h) {
	if(w<=0) w = width;
	if(h<=0) h = height;
	zones.push_back(new Zone(zones.size(), x, y, w, h));
	zones.back()->load();
	
}

void Manager::addZoneToProjector(int zonenum, int projnum) {
	if(projectors.size()<=projnum) {
		ofLog(OF_LOG_ERROR, "Invalid projector number passed to AddZoneToProjector(...)");
		return;
	}
	if(zones.size()<=zonenum) {
		ofLog(OF_LOG_ERROR, "Invalid zone number passed to AddZoneToProjector(...)");
		return;
	}
	
	projectors[projnum]->addZone(zones[zonenum], width, height);
	//projectorGuis[projnum]->add(projectors[projnum].params);
}

int Manager::createDefaultZone() {
	// check there aren't any zones yet?
	// create a zone equal to the width and height of the total output space
	addZone(0,0,width,height);
	return zones.size()-1;
	
}

void Manager::drawLine(const ofPoint& start, const ofPoint& end, const ofColor& col, string profileLabel) {
	
	// we should probably adjust the lines for the given glcontext at this point
	
	
	// store the shapes in the manager. Then we can figure out which zones
	// they belong in at draw time. In OFXLASER_ZONE_MANUAL we'll need to also store
	// which zone each shape belongs in.
	
	//Line l = new Line(gLProject(start), gLProject(end), ofFloatColor(col), 1, 1);
	Line* l = new Line(gLProject(start), gLProject(end), col, profileLabel);
	l->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
	shapes.push_back(l);
	
	
}

void Manager::drawDot(const ofPoint& p, const ofColor& col, float intensity, string profileLabel) {
	
	// we should probably adjust the lines for the given glcontext at this point
	
	
	// store the shapes in the manager. Then we can figure out which zones
	// they belong in at draw time. In OFXLASER_ZONE_MANUAL we'll need to also store
	// which zone each shape belongs in.
	
	//Line l = new Line(gLProject(start), gLProject(end), ofFloatColor(col), 1, 1);
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

void Manager::drawCircle(const ofPoint & centre, const float& radius, const ofColor& col,string profileName){
	ofxLaser::Circle* c = new ofxLaser::Circle(centre,radius, col, profileName);
	c->setTargetZone(targetZone); // only relevant for OFXLASER_ZONE_MANUAL
	shapes.push_back(c);
	
}

void Manager:: update(){
	
	zonesChanged = false;
	
    if(useBitmapMask) laserMask.update();
	// delete all the shapes - all shape objects need a destructor!
	for(int i = 0; i<shapes.size(); i++) {
		delete shapes[i];
	}
	shapes.clear();
    
    // updates all the zones. If zone->update returns true, then
    // it means that the zone has changed.
	bool updateZoneRects = false;
	for(int i = 0; i<zones.size(); i++) {
		zones[i]->visible= (currentProjector==-1);
		updateZoneRects = updateZoneRects | zones[i]->update(); // is this dangerous? Optimisation may stop the function being called. 
	}
    
    // update all the projectors which clears the points,
    // and updates all the zone settings
	for(int i = 0; i<projectors.size(); i++) {
		projectors[i]->update(updateZoneRects); // clears the points
	}
	zonesChanged = updateZoneRects;
	
}


void Manager::send(){
	
	// here's where the magic happens.
	// 1 :
	// figure out which zones to send the shapes to
	// and send them. When the zones get the shape, they transform them
	// into local zone space.
	
	if(zoneMode!=OFXLASER_ZONE_OPTIMISE) {
		for(int j = 0; j<zones.size(); j++) {
			Zone& z = *zones[j];
			z.shapes.clear();
		
			for(int i = 0; i<shapes.size(); i++) {
				Shape* s = shapes[i];
				// if (zone should have shape) then
				// TODO zone intersect shape test
				if(zoneMode == OFXLASER_ZONE_AUTOMATIC) {
					bool shapeAdded = z.addShape(s);
				} else if(zoneMode == OFXLASER_ZONE_MANUAL) {
					if(s->getTargetZone() == j) z.addShape(s);
				}
			}
		}
	} else {
		// OPTIMISE ALGORITHM GOES HERE
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
	for(int i = 0; i<projectors.size(); i++) {
		
		Projector& p = *projectors[i];
		
		p.send(useBitmapMask?laserMask.getPixels():NULL, masterIntensity);
		
	}
}

void Manager:: drawUI(bool expandPreview){
	
	// if expandPreview is true, then we expand the preview area to the
	// maximum space that we have available.
	
	
	int smallPreviewHeight = 310;
	
	
	// showPreview determines whether we show the preview
	// laser graphics on screen or not.
	if(showPreview) {

		ofPushStyle();
		
		// work out the scale for the preview...
		// default scale is 1 with an 8 pixel margin
		previewScale = 1;
		previewOffset.set(guiSpacing,guiSpacing);
		
		// but if we're viewing a projector warp ui
		// then shrink the preview down and move it underneath
		if(currentProjector>=0) {
			previewOffset.set(guiSpacing,800+16);
			previewScale = (float)smallPreviewHeight/(float)height;
		// but if we're expanding the preview, then work out the scale
		// to fill the whole screen
		} else if(expandPreview) {
			previewOffset.set(0,0); 
			previewScale = (float)ofGetWidth()/(float)width;
			if(height*previewScale>ofGetHeight()) {
				previewScale = (float)ofGetHeight()/(float)height;
			}
		}
		
		renderPreview();
		
		// this renders the zones in the graphics source space
		for(int i = 0; i<zones.size(); i++) {
			zones[i]->visible = showZones;
			zones[i]->active = showZones && (currentProjector<0);
			
			zones[i]->offset.set(previewOffset);
			zones[i]->scale = previewScale;
			
			zones[i]->draw();
		}
		
		laserMask.draw(showBitmapMask);

		ofPopStyle();

	}
	

	ofPushStyle();
	
    // if none of the projectors are selected then draw as many as we can on screen
	if(currentProjector==-1) {
		ofPushMatrix();
		float scale = 1 ;
		if((smallPreviewHeight+guiSpacing)*projectors.size()>ofGetWidth()-(guiSpacing*2)) {
			scale = ((float)ofGetWidth()-(guiSpacing*2))/((float)(smallPreviewHeight+guiSpacing)*(float)projectors.size());
			//ofScale(scale, scale);
		}
		
		ofTranslate(guiSpacing,height+(guiSpacing*2));
		
		for(int i = 0; i<projectors.size(); i++) {
			if((!expandPreview)&&(showPathPreviews)) {
				ofFill();
				ofSetColor(0);
				ofRectangle projectorPreviewRect(((smallPreviewHeight*scale) +guiSpacing)*i,0,smallPreviewHeight*scale,smallPreviewHeight*scale);
				ofDrawRectangle(projectorPreviewRect);
				projectors[i]->drawLaserPath(projectorPreviewRect);
			}
			// disables the warp interfaces
			projectors[i]->hideWarpGui();
		}
		
		ofPopMatrix();
		
		// if we're not filling the preview to fit the screen, draw the projector
		// gui elements
		if(!expandPreview) {
			if(projectors.size()>2) {
				for(int i = 0; i<projectors.size(); i++) {
					int x = projectors[i]->gui->getPosition().x;
					projectors[i]->renderStatusBox(x, i*(dacStatusBoxHeight+guiSpacing)+guiSpacing, guiProjectorPanelWidth,dacStatusBoxHeight);
				}
			} else {
				for(int i = 0; i<projectors.size(); i++) {
					int x = projectors[i]->gui->getPosition().x;
					int y = projectors[i]->gui->getPosition().y;
					int w = projectors[i]->gui->getWidth();
					projectors[i]->renderStatusBox(x, y-guiSpacing-dacStatusBoxHeight, guiProjectorPanelWidth,dacStatusBoxHeight);
					projectors[i]->showWarpGui();
					projectors[i]->gui->draw();
				}
			}
		}
		
	} else  {
		// ELSE we have a currently selected projector, so draw the various UI elements
		// for that...
		
        for(int i = 0; i<projectors.size(); i++) {
			if(i==currentProjector) {
				
				ofFill();
				ofSetColor(0);
				float size = expandPreview ? (float)ofGetHeight()-(guiSpacing*2) : 800;
				
				ofDrawRectangle(guiSpacing,guiSpacing,size,size);
				projectors[i]->showWarpGui();
                projectors[i]->drawWarpUI(guiSpacing,guiSpacing,size,size);
				projectors[i]->drawLaserPath(guiSpacing,guiSpacing,size,size);
				

			} else {
				projectors[i]->hideWarpGui();
			}
			
			// if this is the current projector or we have 2 or fewer projectors, then render the gui
			if(guiIsVisible && ((i==currentProjector) || (projectors.size()<=2))) {
				if(guiIsVisible) {
					int x = projectors[i]->gui->getPosition().x;
					int y = guiSpacing;
					projectors[i]->renderStatusBox(x, y, guiProjectorPanelWidth,dacStatusBoxHeight);
					projectors[i]->gui->draw();
					
				}
			}
        }
	
	
    }
 
    
//    for(int i = 0; i<projectors.size(); i++) {
//		//projectors[i]->guiIsVisible = guiIsVisible;
//		if(guiIsVisible && projectors[i]->guiIsVisible) projectors[i]->gui->draw();
//	}
	ofPopStyle();
    if(!expandPreview) gui.draw();
    
	
}

void Manager :: renderPreview() {
	ofPushStyle();
	
	ofPushMatrix();
	ofTranslate(previewOffset);
	ofScale(previewScale, previewScale);
	
	
    // draw outline of laser output area
	ofSetColor(0);
	ofFill();
	ofDrawRectangle(0,0,width,height);
	ofSetColor(50);
    ofNoFill();
    ofDrawRectangle(0,0,width,height);
	
	// render guide if we have one
	if((showGuide) && (guideImage.isAllocated())){
		
		ofPushMatrix();
		
		ofPushStyle();
		
		ofSetColor(guideBrightness);
		
		guideImage.setAnchorPercent(0.5, 0.5);
		
		ofTranslate(width/2, height/2);
		ofScale(height/guideImage.getHeight(), height/guideImage.getHeight());
		guideImage.draw(0,0);
		
		
		
		ofPopStyle();
		
		ofPopMatrix();
	}
	
    // Draw laser graphics preview ----------------
    ofMesh mesh;
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    mesh.clear();
    mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    // Draw the preview laser graphics, with zones overlaid
    for(int i = 0; i<shapes.size(); i++) {
        shapes[i]->addPreviewToMesh(mesh);
    }
	
	ofRectangle laserRect(0,0,width, height);
    if(useBitmapMask) {
		const vector<glm::vec3>& points = mesh.getVertices();
        std::vector<ofFloatColor>& colours = mesh.getColors();
        
        for(int i = 0;i<points.size(); i++ ){
 
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
    
    for(int i = 0; i<colours.size(); i++) {
        colours[i].r*=0.4;
        colours[i].g*=0.4;
        colours[i].b*=0.4;
    }
    
    ofSetLineWidth(4);
    mesh.draw();
    
    ofDisableBlendMode();

	ofPopMatrix();
	
    ofPopStyle();
	
}
void Manager ::updateScreenSize(ofResizeEventArgs &e){
    
    updateScreenSize();
}

void Manager :: updateScreenSize() {
	screenHeight = ofGetHeight();
	updateGuiPositions();
	
}

void Manager :: updateGuiPositions() {

	

	for(int i = 0; i<projectors.size(); i++) {
		int x = ofGetWidth()-(guiProjectorPanelWidth+guiSpacing);
		if(projectors.size()<=2){
			x = ofMap(i, 0, projectors.size(),ofGetWidth()-((guiProjectorPanelWidth+guiSpacing)*projectors.size()), ofGetWidth());
		}
		
		projectors[i]->gui->setPosition(x,dacStatusBoxHeight+(guiSpacing*2));
		//projectors[i]->gui->setPosition(x,(currentProjector>-1)?guiSpacing:dacStatusBoxHeight+(guiSpacing*2));

	}
	
	if(projectors.size()>2) {
		gui.setPosition(ofGetWidth()-(guiProjectorPanelWidth+guiSpacing) - guiSpacing- 220,guiSpacing);
	} else if(projectors.size()>0){
		int x = projectors[0]->gui->getPosition().x-220-guiSpacing;
		gui.setPosition(x, guiSpacing);
	} else {
		ofLog(OF_LOG_WARNING, "ofxLaser::Manager - no projectors added");
		gui.setPosition(ofGetWidth()-(guiProjectorPanelWidth+guiSpacing), guiSpacing);
	}
}
ofxPanel& Manager ::getGui(){
	return gui;
}

int Manager :: getProjectorPointRate(int projectornum ){
    return projectors.at(projectornum)->getPointRate();
}

float Manager :: getProjectorFrameRate(int projectornum ){
	if((projectornum>=0) && (projectornum<projectors.size())) {
    	return projectors.at(projectornum)->getFrameRate();
	} else return 0;
}
void Manager::sendRawPoints(const std::vector<ofxLaser::Point>& points, int projectornum, int zonenum){
   // ofLog(OF_LOG_NOTICE, "ofxLaser::Manager::sendRawPoints(...) point count : "+ofToString(points.size())); 
    Projector* proj = projectors.at(projectornum);
    proj->sendRawPoints(points, zonenum, masterIntensity);

}


void Manager::nextProjector() {
	currentProjector++;
	if(currentProjector>=projectors.size()) currentProjector=-1;
	updateGuiPositions();
	
}


void Manager::previousProjector() {
	currentProjector--;
	if(currentProjector<-1) currentProjector=projectors.size()-1;
	updateGuiPositions();
	
}

void Manager::initGui(bool showAdvanced) {
	
	// TODO - warn if called more than once.
	
	ofxGuiSetDefaultWidth(220);
	ofxGuiSetFillColor(ofColor::fromHsb(144,100,112));
	gui.setup("Laser", "laserSettings.xml");
	
	
	gui.setDefaultHeight(40);

	gui.add(armAllButton.setup("ARM ALL"));
	gui.add(disarmAllButton.setup("DISARM ALL"));
	gui.add(masterIntensity.set("Master Intensity", 1,0,1));

	gui.setDefaultHeight(20);

	armAllButton.addListener(this, &ofxLaser::Manager::armAllProjectors);
	disarmAllButton.addListener(this, &ofxLaser::Manager::disarmAllProjectors);
	testPattern.addListener(this, &ofxLaser::Manager::testPatternAllProjectors);

	
	for(int i = 0; i<projectors.size();i++) {
		gui.add(projectors[i]->armed);
		
	}
	
	gui.add(testPattern.set("Test Pattern", 0,0,8));
	
	//ofParameterGroup params;
	params.setName("Interface");
	params.add(showZones.set("Show zones", false));
	params.add(showPreview.set("Show preview", true));
	params.add(showPathPreviews.set("Show path previews", true));
    params.add(useBitmapMask.set("Use bitmap mask", false));
    params.add(showBitmapMask.set("Show bitmap mask", false));
	
	if(guideImage.isAllocated()) {
		params.add(showGuide.set("show guide image", true));
		params.add(guideBrightness.set("guide brightness", 150,0,255));
	}
	
	gui.add(params);
	if(customParams.size()>0) {
		customParams.setName("Custom");
		gui.add(customParams);
	}
    
	gui.loadFromFile("laserSettings.xml");
    showPreview = true;
	showPathPreviews = true;
	//gui.setPosition(width+10, 8);
	
    
	// TODO - check font exists?
	//gui.loadFont("fonts/Verdana.ttf", 8, false);
	
	ofxGuiSetDefaultWidth(guiProjectorPanelWidth);
	for(int i = 0; i<projectors.size(); i++) {
		projectors[i]->initGui(showAdvanced);
		projectors[i]->armed.setName(ofToString(i+1)+" ARMED");
	}
	
	updateScreenSize();

	ofxGuiSetDefaultWidth(220);
	
}

void Manager::addCustomParameter(ofAbstractParameter& param){
	customParams.add(param);
	
}

void Manager::armAllProjectors() {
	for(int i = 0; i<projectors.size(); i++) {
		projectors[i]->armed = true;
	}
}
void Manager::disarmAllProjectors(){
	for(int i = 0; i<projectors.size(); i++) {
		projectors[i]->armed = false;
	}
}
void Manager::testPatternAllProjectors(int &pattern){
	for(int i = 0; i<projectors.size(); i++) {
		projectors[i]->testPattern = testPattern;
	}
}
void Manager::saveSettings() {
	gui.saveToFile("laserSettings.xml");
	for(int i = 0; i<projectors.size(); i++) {
		projectors[i]->saveSettings();
		

	}
    laserMask.saveSettings();
	
}

// converts openGL coords to screen coords //
ofPoint Manager::gLProject(ofPoint p) {
	return gLProject(p.x, p.y, p.z);
	
}
ofPoint Manager::gLProject( float ax, float ay, float az ) {
#ifdef _MSC_VER	
	return ofPoint(ax, ay, 0);
#else	
	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble X, Y, Z = 0;
	gluProject( ax, ay, az, model_view, projection, viewport, &X, &Y, &Z);
	
	// bit of a hack - if you're rendering into an Fbo then y is inverted
	if(projection[5]<0) Y = screenHeight-Y;//ofGetHeight()-Y;
   // return(ofPoint(ax, ay));
	return ofPoint(X, Y, 0.0f);
#endif
}


Projector& Manager::getProjector(int index){
	return *projectors.at(index);
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
	return *zones[zonenum-1];
	
}

bool Manager::setTargetZone(int zone){  // only for OFX_ZONE_MANUAL
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

bool Manager::isProjectorArmed(int i){
	if((i<0) || (i>=projectors.size())){
		return false;
	} else {
		return projectors[i]->armed; 
	}
	
}

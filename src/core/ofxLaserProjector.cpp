//
//  ofxLaserProjector.cpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserProjector.h"
#include "ofxLaserManager.h"

using namespace ofxLaser;

Projector::Projector(string projectorlabel, DacBase& laserdac) {
	
	dac = &laserdac;
	laserHomePosition = ofPoint(400,400);
	label = projectorlabel;
	//maskRectangle.set(0,0,800,800);
	
	renderProfiles.emplace(OFXLASER_PROFILE_FAST, ofToString("Fast"));
	renderProfiles.emplace(OFXLASER_PROFILE_DEFAULT, ofToString("Default"));
	renderProfiles.emplace(OFXLASER_PROFILE_DETAIL, ofToString("High quality"));
	
	numTestPatterns = 9;
    //guiIsVisible = true;
	
	guiInitialised = false;
	
};

void Projector :: initGui(bool showAdvanced) {
	
	gui = new ofxPanel();
	
	gui->setup(label, label+".json");
	gui->setName(label);
	
	gui->add(armed.set("ARMED", false));
	gui->add(intensity.set("Intensity", 1,0,1));
	gui->add(testPattern.set("Test Pattern", 0,0,numTestPatterns));
	gui->add(resetDac.set("Reset DAC", false));
	
	ofParameterGroup projectorparams;
	projectorparams.setName("Projector settings");
	projectorparams.add(pps.set("Points per second", 30000,1000,80000));
	pps.addListener(this, &Projector::ppsChanged);
    
	
    
	projectorparams.add(colourChangeOffset.set("Colour change offset", 0,-4,4));
	projectorparams.add(laserOnWhileMoving.set("Laser on while moving", false));
	projectorparams.add(moveSpeed.set("Move Speed", 5,0.1,15));
	projectorparams.add(shapePreBlank.set("Blank points before", 1,0,8));
	projectorparams.add(shapePreOn.set("On points before", 1,0,8));
	projectorparams.add(shapePostOn.set("On points after", 1,0,8));
	projectorparams.add(shapePostBlank.set("Blank points after", 1,0,8));
	
	projectorparams.add(flipX.set("Flip X", false));
	projectorparams.add(flipY.set("Flip Y",false));
	projectorparams.add(outputOffset.set("Output position offset", glm::vec2(0,0), glm::vec2(-20,-20),glm::vec2(20,20)));
	projectorparams.add(rotation.set("Output rotation",0,-90,90));

	
	ofParameterGroup advanced;
	advanced.setName("Advanced");
	advanced.add(speedMultiplier.set("Speed multiplier", 1,0.01,2));
	advanced.add(smoothHomePosition.set("Smooth home position", true));
	advanced.add(targetFramerate.set("Target framerate (experimental)", 25, 23, 35));
	advanced.add(syncToTargetFramerate.set("Sync to Target framerate", false));

	if(showAdvanced) {
		projectorparams.add(advanced);
	}
	else speedMultiplier = 1;
	
	gui->add(projectorparams);
	
	ofParameterGroup renderparams;
	renderparams.setName("Render profiles");
	
	// TODO set up default profiles
	RenderProfile& fast = renderProfiles.at(OFXLASER_PROFILE_FAST);
	RenderProfile& defaultProfile = renderProfiles.at(OFXLASER_PROFILE_DEFAULT);
	RenderProfile& detail = renderProfiles.at(OFXLASER_PROFILE_DETAIL);

	renderparams.add(defaultProfile.params);
	renderparams.add(fast.params);
	renderparams.add(detail.params);
    
    defaultProfile.speed = 10;
    defaultProfile.acceleration = 2;
    defaultProfile.cornerThreshold  = 60;
    defaultProfile.dotMaxPoints = 20;
    
    fast.speed = 20;
    fast.acceleration = 4;
    fast.cornerThreshold  = 90;
    fast.dotMaxPoints = 10;
    
    detail.speed = 5;
    detail.acceleration = 1;
    detail.cornerThreshold  = 20;
    detail.dotMaxPoints = 40;
    
	gui->add(renderparams);
	
	
	
	ofParameterGroup colourparams;
	colourparams.setName("Colour calibration");
	
	colourparams.add(red100.set("red 100", 1,0,1));
	colourparams.add(red75.set("red 75", 0.75,0,1));
	colourparams.add(red50.set("red 50", 0.5,0,1));
	colourparams.add(red25.set("red 25", 0.25,0,1));
	colourparams.add(red0.set("red 0", 0,0,1));
	
	colourparams.add(green100.set("green 100", 1,0,1));
	colourparams.add(green75.set("green 75", 0.75,0,1));
	colourparams.add(green50.set("green 50", 0.5,0,1));
	colourparams.add(green25.set("green 25", 0.25,0,1));
	colourparams.add(green0.set("green 0", 0,0,1));
	
	colourparams.add(blue100.set("blue 100", 1,0,1));
	colourparams.add(blue75.set("blue 75", 0.75,0,1));
	colourparams.add(blue50.set("blue 50", 0.5,0,1));
	colourparams.add(blue25.set("blue 25", 0.25,0,1));
	colourparams.add(blue0.set("blue 0", 0,0,1));
	
	gui->add(colourparams);
	
    zonesEnabled.resize(zones.size());
    
	ofParameterGroup zonemutegroup;
	zonemutegroup.setName("Mute Zones");
	zonesMuted.resize(zones.size());
	for(size_t i = 0; i<zones.size(); i++) {
		zonemutegroup.add(zonesMuted[i].set(zones[i]->label, false));
	}
	gui->add(zonemutegroup);
    
    ofParameterGroup zonesologroup;
    zonesologroup.setName("Solo Zones");
    zonesSoloed.resize(zones.size());
    for(size_t i = 0; i<zones.size(); i++) {
        zonesologroup.add(zonesSoloed[i].set(zones[i]->label, false));
    }
    
    gui->add(zonesologroup);
	
	
    
	for(size_t i = 0; i<zones.size(); i++) {
		
		ofParameter<float>& leftEdge = leftEdges[i];
		ofParameter<float>& rightEdge = rightEdges[i];
		ofParameter<float>& topEdge = topEdges[i];
		ofParameter<float>& bottomEdge = bottomEdges[i];
		
		ofParameterGroup zonemaskgroup;
		zonemaskgroup.setName(zones[i]->label);
		zonemaskgroup.add(bottomEdge.set("Bottom Edge", 0,0,1));
		zonemaskgroup.add(topEdge.set("Top Edge", 0,0,1));
		zonemaskgroup.add(leftEdge.set("Left Edge", 0,0,1));
		zonemaskgroup.add(rightEdge.set("Right Edge", 0,0,1));
		
		ofAddListener(zonemaskgroup.parameterChangedE(), this, &Projector::zoneMaskChanged);
		
		gui->add(zonemaskgroup);
		
		ofParameterGroup zonewarpgroup;
		zonewarpgroup.setName(zones[i]->label+"warp");
		zonewarpgroup.add(zoneTransforms[i]->params);
		gui->add(zonewarpgroup);
	
	}
	

	// try loading the xml file for legacy reasons
	gui->loadFromFile(label+".xml");
	gui->loadFromFile(label+".json");
	
	minimiseGui();
	
	for(size_t i = 0; i<zoneTransforms.size(); i++) {
		zoneTransforms[i]->initGuiListeners();
		zoneTransforms[i]->loadSettings(); 
	}
	
	armed = false;
	testPattern = 0;
    for(size_t i = 0; i<zonesSoloed.size(); i++ ) zonesSoloed[i] = false;
    
    initListeners();
    
	guiInitialised = true;

	
}



void Projector :: minimiseGui() {
	gui->minimizeAll();
	ofxGuiGroup* g;
	//	g = dynamic_cast <ofxGuiGroup *>(gui->getControl(projectorlabel));
	//	if(g) g->maximize();
	ofxGuiGroup* projsettings = dynamic_cast <ofxGuiGroup *>(gui->getControl("Projector settings"));
	if(projsettings) {
		projsettings->maximize();
    	g = dynamic_cast <ofxGuiGroup *>(projsettings->getControl("Output position offset"));
    	if(g) g->minimize();
		g = dynamic_cast <ofxGuiGroup *>(projsettings->getControl("Advanced"));
		if(g) g->minimize();
	}
}

Projector::~Projector() {
	ofLog(OF_LOG_NOTICE, "ofxLaser::Projector destructor called");
	pps.removeListener(this, &Projector::ppsChanged);
	delete gui; 
	
}

void Projector:: ppsChanged(int& e){
	ofLog(OF_LOG_NOTICE, "ppsChanged"+ofToString(pps));
	pps=round(pps/100)*100;
	if(pps<=100) pps =100;
	dac->setPointsPerSecond(pps);
}


void Projector::addZone(Zone* zone, float srcwidth, float srcheight) {

	if(std::find(zones.begin(), zones.end(), zone) != zones.end()) {
		ofLog(OF_LOG_ERROR, "Projector::addZone(...) - Projector already contains zone");
		return;
	}
	
	zones.push_back(zone);
	
	zoneTransforms.push_back(new ZoneTransform(zone->index, "Warp"+label+"Zone"+ofToString(zone->index+1)));
	ZoneTransform& zoneTransform = *zoneTransforms.back();
    
    zonesMuted.resize(zones.size());
    zonesSoloed.resize(zones.size());
    zonesEnabled.resize(zones.size());

	zoneTransform.init(zone->rect); 
	zoneTransform.setSrc(zone->rect);
	ofRectangle destRect = zone->rect;
	destRect.scale(800/srcwidth, 800/srcheight);
	destRect.x*=800/srcwidth;
	destRect.y*=800/srcheight;
	zoneTransform.setDst(destRect);
	zoneTransform.scale = 1;
	zoneTransform.offset.set(0,0);
	
	//warp.updateHomography();
	
	zoneMasks.emplace_back(zone->rect);
	
	leftEdges.push_back(0);
	rightEdges.push_back(0);
	topEdges.push_back(0);
	bottomEdges.push_back(0);
	
	zoneTransform.loadSettings();
}

void Projector::zoneMaskChanged(ofAbstractParameter& e) {
	updateZoneMasks();
}

void Projector::updateZoneMasks() {
	
	for(size_t i = 0; i<zoneMasks.size(); i++) {
		
		ofRectangle& zoneMask = zoneMasks[i];
		Zone& zone = *zones[i];
		
		ofParameter<float>& leftEdge = leftEdges[i];
		ofParameter<float>& rightEdge = rightEdges[i];
		ofParameter<float>& topEdge = topEdges[i];
		ofParameter<float>& bottomEdge = bottomEdges[i];
		zoneMask.setX(zone.rect.getLeft()+(leftEdge*zone.rect.getWidth()));
		zoneMask.setY(zone.rect.getTop()+(topEdge*zone.rect.getHeight()));
		zoneMask.setWidth(zone.rect.getWidth()*(1-leftEdge-rightEdge));
		zoneMask.setHeight(zone.rect.getHeight()*(1-topEdge-bottomEdge));
	}
}

void Projector::renderStatusBox(float x, float y, float w, float h) {
	ofPushStyle();
	ofPushMatrix();
	ofDisableBlendMode();
	ofTranslate(x,y);
	ofFill();
	ofSetColor(50);
	ofDrawRectangle(0,0,w,h);
	ofSetColor(100);
	ofDrawRectangle(0,0,w,18);
	
	// draw name of projector
	//ofSetText
	ofSetColor(20);
	ofDrawBitmapString(ofToString(label.back()) + " "+dac->getLabel(), 8, 13);
	string framerate = ofToString(round(smoothedFrameRate));
	ofDrawBitmapString(framerate, w-(framerate.size()*8)- 18,13);
	ofSetColor(dac->getStatusColour());
	ofDrawRectangle(w-18+4,4,10,10); 
	//ofNoFill();
	//ofSetColor(255);
	//ofDrawRectangle(0,0,w,h);
	ofTranslate(0,24);
	const vector<ofAbstractParameter*>& displaydata = dac->getDisplayData();
	for(size_t i = 0; i<displaydata.size(); i++) {
		
		ofAbstractParameter* dataelement = displaydata[i];
		
		ofSetColor(0);
		ofDrawRectangle(8,i*16,w-16,10);
		
		ofParameter<int>* intparam = dynamic_cast<ofParameter<int>*>(dataelement);
		if(intparam!=nullptr) {
			ofSetColor(100);
			ofDrawBitmapString(dataelement->getName(),8,i*16+10);
			ofSetColor(150);
			
			float size = ofMap(intparam->get(), intparam->getMin(), intparam->getMax(), 0,w-16, true);
			ofDrawRectangle(8, i*16, size,10);
		}
		ofParameter<string>* stringparam = dynamic_cast<ofParameter<string>*>(dataelement);
		if(stringparam!=nullptr) {
			ofSetColor(100);
			ofDrawBitmapString(stringparam->get(),8,i*16+10);
			
		}
	}
	
	
	
	ofPopMatrix();
	ofPopStyle();
	
}
void Projector::drawWarpUI(float x, float y, float w, float h) {
	
	ofPushStyle();
	ofNoFill();

    float warpscale = w/800.0f ;
//    ofPushMatrix();
//	ofTranslate(x,y);
//    ofScale(warpscale,warpscale);
//
//    ofPopMatrix();
//
	

	// draw the handles for all the warp UIs
	for(size_t i = 0; i<zoneTransforms.size(); i++) {
		if(!zonesEnabled[i]) continue;
		ZoneTransform& warp = *zoneTransforms[i];
		warp.offset = ofPoint(x,y) + (ofPoint(outputOffset)*warpscale);
		warp.scale = warpscale;
		warp.draw();
	}
	
	ofPushMatrix();
	ofTranslate(x,y);
	ofScale(warpscale,warpscale);
	ofTranslate(outputOffset);
	
	// go through and draw blue rectangles around the warper
	ofPoint p;
	for(size_t i = 0; i<zoneTransforms.size(); i++) {

		ZoneTransform& warp = *zoneTransforms[i];
		
		warp.setVisible(zonesEnabled[i]);
		
		if(!zonesEnabled[i]) continue;
		
		ofRectangle& mask = zoneMasks[i];
		
		//ofPushMatrix();
		//ofTranslate();
		ofEnableAlphaBlending();
		ofFill();
		ofSetColor(0,0,255,30);
		
		ofBeginShape();
		p = warp.getWarpedPoint((ofPoint)mask.getTopLeft());
		ofVertex(p);
		p = warp.getWarpedPoint((ofPoint)mask.getTopRight());
		ofVertex(p);
		p = warp.getWarpedPoint((ofPoint)mask.getBottomRight());
		ofVertex(p);
		p = warp.getWarpedPoint((ofPoint)mask.getBottomLeft());
		ofVertex(p);
		ofEndShape(true);
		
//		ofNoFill();
//		ofSetColor(0,0,255,200);
//
//		ofBeginShape();
//		p = warp.getWarpedPoint((ofPoint)mask.getTopLeft());
//		ofVertex(p);
//		p = warp.getWarpedPoint((ofPoint)mask.getTopRight());
//		ofVertex(p);
//		p = warp.getWarpedPoint((ofPoint)mask.getBottomRight());
//		ofVertex(p);
//		p = warp.getWarpedPoint((ofPoint)mask.getBottomLeft());
//		ofVertex(p);
//		ofEndShape(true);
		
		
		//ofPopMatrix();
		
	}
	
	ofPopMatrix();
	ofPopStyle();
	
	//drawLaserPath(0,0,warpscale*800,warpscale*800);
	
}

void Projector :: drawLaserPath(ofRectangle rect) {
	drawLaserPath(rect.x, rect.y, rect.width, rect.height); 
}
void Projector :: drawLaserPath(float x, float y, float w, float h) {
	ofPushStyle();
	
	ofSetColor(255);

	ofDrawBitmapString(label.back(), x+w-20, y+20);
	ofSetColor(100);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofPushMatrix();
	ofTranslate(x,y);
	ofScale(w/800, h/800);

	
	ofTranslate(outputOffset);

	ofPoint p;

	for(size_t i = 0; i<zoneTransforms.size(); i++) {
		ZoneTransform& warp = *zoneTransforms[i];

		warp.setVisible(zonesEnabled[i]);

		if(!zonesEnabled[i]) continue;

		ofRectangle& mask = zoneMasks[i];


		ofNoFill();
		ofSetColor(50,50,255,150);

		ofBeginShape();
		p = warp.getWarpedPoint((ofPoint)mask.getTopLeft());
		ofVertex(p);
		p = warp.getWarpedPoint((ofPoint)mask.getTopRight());
		ofVertex(p);
		p = warp.getWarpedPoint((ofPoint)mask.getBottomRight());
		ofVertex(p);
		p = warp.getWarpedPoint((ofPoint)mask.getBottomLeft());
		ofVertex(p);
		ofEndShape(true);


	}


	ofNoFill();
	//ofSetColor(255);
	ofSetColor(MIN(255 * w / 800.0f, 255));
	ofSetLineWidth(1);
	
	previewPathMesh.setMode(OF_PRIMITIVE_POINTS);
	previewPathMesh.draw();
	
	for(size_t i = 0; i<previewPathMesh.getNumVertices();i++) {
		previewPathMesh.addColor(ofColor(ofMap(i,0,previewPathMesh.getNumVertices(), 200,20),0,200));
	}

	
	
	ofSetLineWidth(0.5);
	previewPathMesh.setMode(OF_PRIMITIVE_LINE_STRIP);
	previewPathMesh.draw();
	
	// draws the laser path
	
	if(previewPathMesh.getNumVertices()>0) {
		
		float time = previewPathMesh.getNumVertices()/50.0f;
		int pointindex =ofMap(fmod(ofGetElapsedTimef(),time),0,time,0,previewPathMesh.getNumVertices());
		ofPoint p = previewPathMesh.getVertex(pointindex);
		ofSetColor(255);
		ofDrawCircle(p, 3);
		ofFill();
		Point& lp =laserPoints[pointindex];
		ofSetColor(lp.getColour()*255);
		ofDrawCircle(p, 3);
	}
	

	ofDisableBlendMode();
	ofPopStyle();
	
	
	ofPopMatrix();
	
//	ofNoFill();
//	ofSetColor(255,0,0);
//	ofDrawRectangle(x,y,w,h);
//
	
}

void Projector :: hideWarpGui() {
	
	// disable warps
	for(size_t i = 0; i<zoneTransforms.size(); i++) {
		
		
		ZoneTransform& warp = *zoneTransforms[i];
		
		warp.setVisible(false);
		
	}
	
	// move ui panel out the way
	//gui->setPosition(ofGetWidth(), 8);
	
}
void Projector :: showWarpGui() {
	
	for(size_t i = 0; i<zoneTransforms.size(); i++) {
		
		
		ZoneTransform& warp = *zoneTransforms[i];
		
		warp.setVisible(false);
		
	}
	
	// move ui panel back into view
	//gui->setPosition(ofGetWidth()-330, 8);
	
}


void Projector :: initListeners() {
    ofAddListener(ofEvents().mousePressed, this, &Projector::mousePressed, OF_EVENT_ORDER_APP);
}

bool Projector :: mousePressed(ofMouseEventArgs &e){
    
	return false;
}

void Projector::update(bool updateZones) {
	
	// updateZones will be true if any of the zones source points have changed.
	
	// todo should prob do this on a param changed message...
	float x = round(((ofPoint)outputOffset).x*10)/10.0f;
	float y = round(((ofPoint)outputOffset).y*10)/10.0f;
	outputOffset = ofVec2f(x,y);

	if(resetDac) {
		resetDac = false; 
		dac->reset();
	}
    bool soloMode = false;
    for(size_t i = 0; i<zonesSoloed.size(); i++) {
        if(zonesSoloed[i]) {
            soloMode = true;
            break;
        }
    }
    if(soloMode) {
        for(size_t i = 0; i<zonesEnabled.size(); i++ ) {
            zonesEnabled[i] = zonesSoloed[i];
        }
        
    } else {
        for(size_t i = 0; i<zonesEnabled.size(); i++ ) {
            zonesEnabled[i] = !zonesMuted[i];
        }
    }
    
    
	laserPoints.clear();
	previewPathMesh.clear();
	for(size_t i = 0; i<zoneTransforms.size(); i++) {
		zoneTransforms[i]->update();
	}
	
	// if any of the source rectangles have changed then update all the warps
	if(updateZones) {
		for(size_t i = 0; i<zoneTransforms.size(); i++) {
			ZoneTransform& warp = *zoneTransforms[i];
			warp.setSrc(zones[i]->rect);
			warp.updateHomography();
			updateZoneMasks();
		}
	}
	
	smoothedFrameRate += (getFrameRate() - smoothedFrameRate)*0.2;

}


void Projector::sendRawPoints(const vector<ofxLaser::Point>& points, int zonenum, float masterIntensity ){
    
     //ofLog(OF_LOG_NOTICE, "ofxLaser::Projector::sendRawPoints(...) point count : "+ofToString(points.size()));
    
    Zone& zone = *zones.at(zonenum);
    ofRectangle& maskRectangle = zoneMasks.at(zonenum);
    ZoneTransform& warp = *zoneTransforms.at(zonenum);
    bool offScreen = true;
    
    vector<Point>segmentpoints;
    
    //iterate through the points
    for(int k = 0; k<points.size(); k++) {
        
        Point p = points[k];
//        previewPathMesh.addVertex(p);
//        ofColor c = p.getColor();
//        previewPathMesh.addColor(c);
        // mask the points
        
        // are we outside the mask? NB can't use inside because I want points on the edge
        //
        
        if(p.x<maskRectangle.getLeft() ||
           p.x>maskRectangle.getRight() ||
           p.y<maskRectangle.getTop() ||
           p.y>maskRectangle.getBottom())  {
            
            if(!offScreen) {
                offScreen = true;
                // if we already have points then add an inbetween point
                if(k>0) {
                    Point lastpoint = p;
                    
                    // TODO better point on edge rather than just clamp
                    lastpoint.x = ofClamp(lastpoint.x, maskRectangle.getLeft(), maskRectangle.getRight());
                    lastpoint.y = ofClamp(lastpoint.y, maskRectangle.getTop(), maskRectangle.getBottom());
                    segmentpoints.push_back(lastpoint);
                    
                    
                }
            }
            
        } else {
            // we're on screen!
            if(offScreen) {

                offScreen = false;
                if(k>0) {
                    Point lastpoint = points[k-1];
                    
                    // TODO better point on edge rather than just clamp
                    lastpoint.x = ofClamp(lastpoint.x, maskRectangle.getLeft(), maskRectangle.getRight());
                    lastpoint.y = ofClamp(lastpoint.y, maskRectangle.getTop(), maskRectangle.getBottom());
                    
                    segmentpoints.push_back(lastpoint);
                }
            }
            segmentpoints.push_back(p);
        }
        
        // create a point object for it
        
    } // end shapepoints
    // add the segment points to the points for the zone


    
    // go through all the points and warp them into projector space

    for(int k= 0; k<segmentpoints.size(); k++) {
        addPoint(warp.getWarpedPoint(segmentpoints[k]));
    }
    
    
    
    processPoints(masterIntensity, false);
    dac->sendPoints(laserPoints);
    
   
}


                        

void Projector::send(ofPixels* pixels, float masterIntensity) {

	if(!guiInitialised) {
		ofLog(OF_LOG_ERROR, "Error, ofxLaser::projector not initialised yet. (Probably missing a ofxLaser::Manager.initGui() call...");
		return;
	}
	
	vector<ShapePoints> allzoneshapepoints;

	// TODO add speed multiplier to getPointsForMove function
	getAllShapePoints(&allzoneshapepoints, pixels, speedMultiplier);
	
	vector<ShapePoints*> sortedshapepoints;
	
	// sort the point objects
	if(allzoneshapepoints.size()>0) {
		bool reversed = false;
		int currentIndex = 0;
		float shortestDistance = INFINITY;
		int nextDotIndex = -1;

		
		do {
			
			ShapePoints& shapePoints1 = allzoneshapepoints[currentIndex];
			
			shapePoints1.tested = true;
			sortedshapepoints.push_back(&shapePoints1);
			shapePoints1.reversed = reversed;
			
			shortestDistance = INFINITY;
			nextDotIndex = -1;
			
			
			for(int j = 0; j<allzoneshapepoints.size(); j++) {
				
				ShapePoints& shapePoints2 = allzoneshapepoints[j];
				if((&shapePoints1==&shapePoints2) || (shapePoints2.tested)) continue;
				
				shapePoints2.reversed = false;
				
				if(shapePoints1.getEnd().squareDistance(shapePoints2.getStart()) < shortestDistance) {
					shortestDistance = shapePoints1.getEnd().squareDistance(shapePoints2.getStart());
					nextDotIndex = j;
					reversed = false;
				}
				
				if((shapePoints2.reversable) && (shapePoints1.getEnd().squareDistance(shapePoints2.getEnd()) < shortestDistance)) {
					shortestDistance = shapePoints1.getEnd().squareDistance(shapePoints2.getEnd());
					nextDotIndex = j;
					reversed = true;
				}
				
				
			}
			
			currentIndex = nextDotIndex;
			
			
			
		} while (currentIndex>-1);

		// go through the point objects
		// add move between each one
		// add points to the laser
		
		ofPoint currentPosition = laserHomePosition; // MUST be in projector space
		
		for(int j = 0; j<sortedshapepoints.size(); j++) {
			ShapePoints& shapepoints = *sortedshapepoints[j];
			if(shapepoints.size()==0) continue;
			
			if(currentPosition.distance(shapepoints.getStart())>2){
				addPointsForMoveTo(currentPosition, shapepoints.getStart());
			
				for(int k = 0; k<shapePreBlank; k++) {
					addPoint((ofPoint)shapepoints.getStart(), ofColor(0));
				}
				for(int k = 0;k<shapePreOn;k++) {
					addPoint(shapepoints.getStart());
				}
			}
			
			addPoints(shapepoints, shapepoints.reversed);

			currentPosition = shapepoints.getEnd();
			
			ShapePoints* nextshapepoints = nullptr;
			
			if(j<sortedshapepoints.size()-1) {
				nextshapepoints = sortedshapepoints[j+1];
			}
			if((nextshapepoints==nullptr) || (currentPosition.distance(nextshapepoints->getStart())>2)){
				for(int k = 0;k<shapePostOn;k++) {
					addPoint(shapepoints.getEnd());
				}
				for(int k = 0; k<shapePostBlank; k++) {
					addPoint((ofPoint)shapepoints.getEnd(), ofColor(0));
				}
			}
		
			
			
		}
		if(smoothHomePosition) addPointsForMoveTo(currentPosition, laserHomePosition);
		
	}
	
	if (laserPoints.size() == 0) {
		laserPoints.push_back(Point(laserHomePosition, ofColor(0)));
	}
	
	// TODO add system to speed up if too much stuff to draw
	if (syncToTargetFramerate) {
		//targetFramerate = round(targetFramerate * 100) / 100.0f;
		float targetNumPoints = (float)pps / targetFramerate;
		// TODO : CHANGE TO PARAMETER
		if (ofGetKeyPressed(OF_KEY_LEFT)) targetNumPoints -= 10;
		if (ofGetKeyPressed(OF_KEY_RIGHT)) targetNumPoints += 10;
		while (laserPoints.size() < targetNumPoints) {
			addPoint(laserHomePosition, ofColor::black);
		}
	}
	
	processPoints(masterIntensity);
	
	dac->sendFrame(laserPoints);
    numPoints = laserPoints.size();
	
	if(sortedshapepoints.size()>0) {
		if(smoothHomePosition) {
			laserHomePosition += (sortedshapepoints.front()->getStart()-laserHomePosition)*0.01;
		} else {
			laserHomePosition = sortedshapepoints.back()->getEnd();
		}
	}
}


void Projector ::getAllShapePoints(vector<ShapePoints>* shapepointscontainer, ofPixels*pixels, float speedmultiplier){
	
	vector<ShapePoints>& allzoneshapepoints = *shapepointscontainer;
	
	// temp vectors for storing the shapes in
	vector<ShapePoints> zoneshapepoints;
	vector<Point> shapepoints;
	
	// go through each zone
	for(size_t i = 0; i<zones.size(); i++) {
		
		if(!zonesEnabled[i]) continue;
		Zone& zone = *zones[i];
		ZoneTransform& warp = *zoneTransforms[i];
		ofRectangle& maskRectangle = zoneMasks[i];
		deque<Shape*> zoneshapes = zone.shapes;
		
		// add testpattern points for this zone...
		// get array of segmentpoints and append them
		deque<Shape*> testPatternShapes = getTestPatternShapesForZone(i);
		
		zoneshapes.insert(zoneshapes.end(), testPatternShapes.begin(), testPatternShapes.end());
		
		zoneshapepoints.clear();
		
		// go through each shape in the zone
		
		for(int j = 0; j<zoneshapes.size(); j++)  {
			
			// get the points
			Shape& shape = *(zoneshapes[j]);
			
			RenderProfile& renderProfile = getRenderProfile(shape.profileLabel);
			
			shapepoints.clear();
			shape.appendPointsToVector(shapepoints, renderProfile, speedmultiplier);
			
			bool offScreen = true;
			
			ShapePoints segmentpoints;
			
			//iterate through the points
			for(int k = 0; k<shapepoints.size(); k++) {
				
				Point& p = shapepoints[k];
				
				// mask the points
				
				// are we outside the mask? NB can't use inside because I want points on the edge
				//
				
				if(p.x<maskRectangle.getLeft() ||
				   p.x>maskRectangle.getRight() ||
				   p.y<maskRectangle.getTop() ||
				   p.y>maskRectangle.getBottom())  {
					
					if(!offScreen) {
						offScreen = true;
						// if we already have points then add an inbetween point
						if(k>0) {
							Point lastpoint = p;
							
							// TODO better point on edge rather than just clamp
							lastpoint.x = ofClamp(lastpoint.x, maskRectangle.getLeft(), maskRectangle.getRight());
							lastpoint.y = ofClamp(lastpoint.y, maskRectangle.getTop(), maskRectangle.getBottom());
							segmentpoints.push_back(lastpoint);
							
							// add this bunch to the collection
							zoneshapepoints.push_back(segmentpoints); // should copy
							
							//clear the vector and start again
							segmentpoints.clear();
							
						}
					}
					
				} else {
					// we're on screen!
					if(offScreen) {
						
						segmentpoints.clear();
						offScreen = false;
						if(k>0) {
							Point lastpoint = shapepoints[k-1];
							
							// TODO better point on edge rather than just clamp
							lastpoint.x = ofClamp(lastpoint.x, maskRectangle.getLeft(), maskRectangle.getRight());
							lastpoint.y = ofClamp(lastpoint.y, maskRectangle.getTop(), maskRectangle.getBottom());
							
							segmentpoints.push_back(lastpoint);
						}
					}
					segmentpoints.push_back(p);
				}
				
				// create a point object for it
				
			} // end shapepoints
			// add the segment points to the points for the zone
			if(segmentpoints.size()>0) {
				zoneshapepoints.push_back(segmentpoints);
			}
			
		} // end zoneshapes
		
		
		// go through all the points and warp them into projector space
		for(int j = 0; j<zoneshapepoints.size(); j++) {
			ShapePoints& segmentpoints = zoneshapepoints[j];
			for(int k= 0; k<segmentpoints.size(); k++) {
				
				// Check against the mask image
				if(pixels!=NULL) {
					Point& p = segmentpoints[k];
					ofFloatColor c = pixels->getColor(p.x, p.y);
					float brightness = c.getBrightness();
					p.r*=brightness;
					p.g*=brightness;
					p.b*=brightness;
				}
				
				segmentpoints[k] = warp.getWarpedPoint(segmentpoints[k]);
			}
		}
		
		
		// add all the segments for the zone into the big container for all the segs
		allzoneshapepoints.insert(allzoneshapepoints.end(), zoneshapepoints.begin(), zoneshapepoints.end());
		
		// delete all the test pattern shapes
		for(int j = 0; j<testPatternShapes.size(); j++) {
			delete testPatternShapes[j];
		}
		testPatternShapes.clear();
		
	} // end zones
	
	
	
	
}



RenderProfile& Projector::getRenderProfile(string profilelabel) {
	
		if(renderProfiles.count(profilelabel) == 0) {
			// if we don't have a profile with that name then
			// something has seriously gone wrong
			profilelabel = OFXLASER_PROFILE_DEFAULT;
		}
		return renderProfiles.at(profilelabel);
	
}

deque<Shape*> Projector ::getTestPatternShapesForZone(int zoneindex) {
	
	deque<Shape*> shapes;
	Zone& zone = *zones[zoneindex];
	
	ofRectangle& maskRectangle = zoneMasks[zoneindex];
	
	if(testPattern==1) {
		
		ofRectangle& rect = maskRectangle;
		
		ofColor col = ofColor(0,255,0);
		shapes.push_back(new Line(rect.getTopLeft(), rect.getTopRight(), col, OFXLASER_PROFILE_DEFAULT));
		shapes.push_back(new Line(rect.getTopRight(), rect.getBottomRight(), col, OFXLASER_PROFILE_DEFAULT));
		shapes.push_back(new Line(rect.getBottomRight(), rect.getBottomLeft(), col, OFXLASER_PROFILE_DEFAULT));
		shapes.push_back(new Line(rect.getBottomLeft(), rect.getTopLeft(), col, OFXLASER_PROFILE_DEFAULT));
		shapes.push_back(new Line(rect.getTopLeft(), rect.getBottomRight(), col, OFXLASER_PROFILE_DEFAULT));
		shapes.push_back(new Line(rect.getTopRight(), rect.getBottomLeft(), col, OFXLASER_PROFILE_DEFAULT));

			
	} else if(testPattern==2) {
		
		ofRectangle& rect = zone.rect;
		
		ofPoint v = rect.getBottomRight() - rect.getTopLeft()-ofPoint(0.2,0.2);
		for(float y = 0; y<=1.1; y+=0.333333333) {
			
			shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1, rect.getTop()+0.1+v.y*y),ofPoint(rect.getRight()-0.1, rect.getTop()+0.1+v.y*y), ofColor(255), OFXLASER_PROFILE_DEFAULT));
		}
		
		for(float x =0 ; x<=1.1; x+=0.3333333333) {
			
			
			shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1+ v.x*x, rect.getTop()+0.1),ofPoint(rect.getLeft()+0.1 + v.x*x, rect.getBottom()-0.1), ofColor(255,0,0), OFXLASER_PROFILE_DEFAULT ));
			
		}
		
		shapes.push_back(new Circle(rect.getCenter(), rect.getWidth()/12, ofColor(0,0,255), OFXLASER_PROFILE_DEFAULT));
		shapes.push_back(new Circle(rect.getCenter(), rect.getWidth()/6, ofFloatColor(0,1,0), OFXLASER_PROFILE_DEFAULT));
		
		
		
	}else if(testPattern==3) {
		
		ofRectangle& rect = zone.rect;
		
		ofPoint v = rect.getBottomRight() - rect.getTopLeft()-ofPoint(0.2,0.2);
		
		for(float y = 0; y<=1.1; y+=0.333333333) {
			
			shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1, rect.getTop()+0.1+v.y*y),ofPoint(rect.getRight()-0.1, rect.getTop()+0.1+v.y*y), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		}
		shapes.push_back(new Line(rect.getTopLeft(),  glm::mix( rect.getTopLeft(), rect.getBottomLeft(), 1.0f/3.0f ), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		shapes.push_back(new Line(rect.getBottomLeft(), glm::mix(rect.getTopLeft(), rect.getBottomLeft(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		shapes.push_back(new Line( glm::mix(rect.getTopRight(), rect.getBottomRight(), 1.0f/3.0f), mix(rect.getTopRight(), rect.getBottomRight(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		
	} else if(testPattern==4) {
		
		ofRectangle& rect = zone.rect;
		
		ofPoint v = rect.getBottomRight() - rect.getTopLeft()-ofPoint(0.2,0.2);
		
		for(float x =0 ; x<=1.1; x+=0.3333333333) {
			shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1+ v.x*x, rect.getTop()+0.1),ofPoint(rect.getLeft()+0.1 + v.x*x, rect.getBottom()-0.1), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT ));
			
		}
		
		shapes.push_back(new Line(rect.getTopLeft(), glm::mix( rect.getTopLeft(), rect.getTopRight(), 1.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		shapes.push_back(new Line(rect.getTopRight(), glm::mix( rect.getTopLeft(), rect.getTopRight(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		shapes.push_back(new Line(glm::mix(rect.getBottomLeft(), rect.getBottomRight(), 1.0f/3.0f), glm::mix(rect.getBottomLeft(), rect.getBottomRight(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		
	} else if((testPattern>=5) && (testPattern<=8)) {
		ofColor c;
		
		ofRectangle rect = maskRectangle;
		
		rect.scaleFromCenter(0.5, 0.1);
		vector<ofPoint> points;
		vector<ofColor> colours;
		
		ofPoint currentPosition = rect.getTopLeft();
		
		for(int row = 0; row<5; row ++ ) {
			
			
			float y =rect.getTop() + (rect.getHeight()*row/4);
			
			ofPoint left = ofPoint(rect.getLeft(), y);
			
			ofPoint right = ofPoint(rect.getRight(), y);
			
			int moveIterations = currentPosition.distance(left)/1;
			
			for(int i = 0; i<moveIterations; i++) {
				points.push_back(currentPosition.getInterpolated(left, (float)i/(float)moveIterations));
				colours.push_back(ofColor(0));
				
			}
			currentPosition = right;
			
			if(testPattern == 5) c.set(255,0,0);
			else if(testPattern == 6) c.set(0,255,0);
			else if(testPattern == 7) c.set(0,0,255);
			else if(testPattern == 8) c.set(255,255,255);
			
			switch (row) {
				case 0 :
					c.r *= red100;
					c.g *= green100;
					c.b *= blue100;
					break;
				case 1 :
					c.r *= red75;
					c.g *= green75;
					c.b *= blue75;
					break;
				case 2 :
					c.r *= red50;
					c.g *= green50;
					c.b *= blue50;
					break;
				case 3 :
					c.r *= red25;
					c.g *= green25;
					c.b *= blue25;
					break;
				case 4 :
					c.r *= red0;
					c.g *= green0;
					c.b *= blue0;
					break;
			}
			
			float speed = 10 * ( 1- (row*0.25));
			if(speed<2.5) speed = 2.5;
			
			int blanks = 5;
			for(int i = 0; i< blanks; i++) {
				points.push_back(left);
				colours.push_back(ofColor(0));
			}
			for(float x =left.x ; x<=right.x; x+=speed) {
				points.push_back(ofPoint(x,y));
				colours.push_back(c);
			}
			
			for(int i = 0; i< blanks; i++) {
				points.push_back(right);
				colours.push_back(ofColor(0));
			}
			
			
		}
		shapes.push_back(new ManualShape(points, colours, false,OFXLASER_PROFILE_DEFAULT));
		
	} else if(testPattern ==9) {
		ofRectangle rect = maskRectangle;
		
		shapes.push_back(new Dot(rect.getTopLeft(), ofColor(255,255,255), 1, OFXLASER_PROFILE_DEFAULT));
		shapes.push_back(new Dot(rect.getTopRight(), ofColor(255,255,255), 1, OFXLASER_PROFILE_DEFAULT));
		shapes.push_back(new Dot(rect.getBottomLeft(), ofColor(255,255,255), 1, OFXLASER_PROFILE_DEFAULT));
		shapes.push_back(new Dot(rect.getBottomRight(), ofColor(255,255,255), 1, OFXLASER_PROFILE_DEFAULT));
		
	}
	return shapes; 
	
}


void Projector :: addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint, vector<Point>&points){
	
	ofPoint target = targetpoint;
	ofPoint start = currentPosition;
	
	ofPoint v = target-start;
	
	float blanknum = v.length()/moveSpeed;// + movePointsPadding;
	
	for(int j = 0; j<blanknum; j++) {
		
		float t = Quint::easeInOut((float)j, 0.0f, 1.0f, blanknum);
		
		ofPoint c = (v* t) + start;
		points.emplace_back(c, (laserOnWhileMoving && j%2==0) ? ofColor(255,0,0) : ofColor(0));
		
	}
	
}
void Projector :: addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint){
	
	ofPoint target = targetpoint;
	ofPoint start = currentPosition;
	
	ofPoint v = target-start;
	
	float blanknum = (v.length()/moveSpeed)/speedMultiplier;// + movePointsPadding;
	
	for(int j = 0; j<blanknum; j++) {
		
		float t = Quint::easeInOut((float)j, 0.0f, 1.0f, blanknum);
		
		ofPoint c = (v* t) + start;
		addPoint(c, (laserOnWhileMoving && j%2==0) ? ofColor(200,0,0) : ofColor(0));
		
	}
	
}

void Projector :: addPoint(ofPoint p, ofFloatColor c, float pointIntensity, bool useCalibration) {
	
	
	addPoint(ofxLaser::Point(p, c, pointIntensity, useCalibration));
	
}
void Projector :: addPoints(vector<ofxLaser::Point>&points, bool reversed) {
	if(!reversed) {
		for(size_t i = 0; i<points.size();i++) {
			addPoint(points[i]);
		}
	} else {
		for(size_t i=points.size()-1;i>=0; i--) {
			addPoint(points[i]);
		}
	}
}

void Projector :: addPoint(ofxLaser::Point p) {
	
	p+=(ofPoint)outputOffset;
	
	laserPoints.push_back(p);
	
	previewPathMesh.addVertex(ofPoint(p.x, p.y));

}



void  Projector :: processPoints(float masterIntensity, bool offsetColours) {
			
	// Some lasers change colour too early/late, so the colourChangeOffset system
	// mitigates against that by shifting the colours for the points.
	// Some optimisation makes this slightly hard to read but we iterate through
	// the points forwards or backwards depending on which direction we have to shift.
	// To avoid creating a whole new vector, we're storing the overlap in a buffer of
	// colours.
	// I'm sure there must be some slicker C++ way of doing this...
	
	if(offsetColours) {
	// the offset value is in time, so we convert it to a number of points.
		// this way we can change the PPS and this should still work
		int colourChangeIndexOffset = (float)pps/10000.0f*colourChangeOffset ;
		
		// if >0 then we change the colour later, ie we shift the colours forward

		if(colourChangeIndexOffset>0) {
			
			// add some points to the end of the vector to allow for the offset
			laserPoints.resize(laserPoints.size()+colourChangeIndexOffset);
			for(size_t i = laserPoints.size()-1; i>=0; i--) {
				Point& p = laserPoints[i];
				
				// if we're one of the new points at the end copy the position
				// from the last point
				if(i>=laserPoints.size()-colourChangeIndexOffset) {
					// copy the last positino in the original array
					
					p = laserPoints[laserPoints.size()-1-colourChangeIndexOffset];
					//cout << (laserPoints.size()-1-colourChangeOffset) << " " << p << endl;
					
				}
				//now shift the colour from an earlier point
				if(i>=colourChangeIndexOffset){
					p.copyColourFromPoint(laserPoints[i-colourChangeIndexOffset]);
				} else {
					p.setColour(0,0,0);
				}
				
			}
			
		} else {
			
			// if we're <0 then we shift the colours backward
			
			// change negative to positive
			colourChangeIndexOffset*=-1;
			
			laserPoints.resize(laserPoints.size()+colourChangeIndexOffset);
			
			for(size_t i = laserPoints.size()-1; i>=0; i--) {
				Point& p = laserPoints[i];
				
				ofColor c(0,0,0);
				if(i<laserPoints.size()-colourChangeIndexOffset) c = p.getColour();
				if(i>=colourChangeIndexOffset) {
					p = laserPoints[i-colourChangeIndexOffset];
				} else {
					p= laserPoints[colourChangeIndexOffset];
				}
				p.setColour(c.r, c.g, c.b);
			
			}

		}
	}

	
	for(size_t i = 0; i<laserPoints.size(); i++) {
		
		ofxLaser::Point &p = laserPoints[i];
		

		//p.y+=outputOffset;
//		ofColor maskPixelColour;
//		if(useMaskBitmap) {
//			maskPixelColour = maskBitmap.getColor(p.x/appWidth* (float)maskBitmap.getWidth(), p.y/appHeight * (float)maskBitmap.getHeight());
//			
//		}
		
		
		if(flipY) p.y= 800-p.y;
		if(flipX) p.x= 800-p.x;
		if(abs(rotation)>0.5) {
			p.x-=400;
			p.y-=400;
//			glm::vec3 v(p.x, p.y, 0);
//			glm::vec3 axis(0.0f,0.0f,1.0f);
//			v = glm::rotate(v,1.0f, axis);
			auto vec = glm::vec3(p.x,p.y,0.0f);
			float angle = ofDegToRad(rotation); // since glm version 0.9.6, rotations are in radians, not in degrees
			
			//cout << vec << endl;
			auto rotatedVec = glm::rotate(vec, angle, glm::vec3(0.0f, 0.0f, 1.0f));
			p.x=rotatedVec.x+400;
			p.y=rotatedVec.y+400;

		}
		
		// bounds check
		if(p.x<0) p.x = p.r = p.g = p.b = 0;
		else if(p.x>800) {
			p.x = 800;
			p.r = p.g = p.b = 0;
		}
		if(p.y<0) p.y = p.r = p.g = p.b = 0;
		else if(p.y>800) {
			p.y = 800;
			p.r = p.g = p.b = 0;
		}
		
//		if(showPostTransformPreview){
//			ofPoint previewpoint(p.x, p.y);
//			previewPathMesh.addVertex(previewpoint);
//		}
		
//		ofFloatColor c(p.r / 255.0f, p.g / 255.0f, p.b / 255.0f);
//		if(useMaskBitmap) {
//			c.r*=((float)maskPixelColour.r/255.0);
//			c.g*=((float)maskPixelColour.g/255.0);
//			c.b*=((float)maskPixelColour.b/255.0);
//		}
		
		if(p.useCalibration) {
			p.r = calculateCalibratedBrightness(p.r, intensity*masterIntensity, red100, red75, red50, red25, red0);
			p.g = calculateCalibratedBrightness(p.g, intensity*masterIntensity, green100, green75, green50, green25, green0);
			p.b = calculateCalibratedBrightness(p.b, intensity*masterIntensity, blue100, blue75, blue50, blue25, blue0);
		}
		
		if(!armed) {
			p.r = 0;
			p.g = 0;
			p.b = 0;
		}
	//	ildaPoints.push_back(ofxIlda::Point(p, c, pmin, pmax));
		
	}
	
}


float Projector::calculateCalibratedBrightness(float value, float intensity, float level100, float level75, float level50, float level25, float level0){
	value/=255.0f;
	value *=intensity;
	if(value<0.001) {
		return 0;
	} else if(value<0.25) {
		return ofMap(value, 0, 0.25, level0, level25) *255;
	} else if(value<0.5) {
		return ofMap(value, 0.25, 0.5,level25, level50) *255;
	} else if(value<0.75) {
		return ofMap(value, 0.5, 0.75,level50, level75) *255;
	} else {
		return ofMap(value, 0.75, 1,level75, level100) *255;
	}
	
}


void Projector::saveSettings(){
	gui->saveToFile(label+".json");
	// not sure this is needed cos we save if we edit...
//	for(size_t i = 0; i<zoneWarps.size(); i++) {
//		zoneWarps[i]->saveSettings();
//	}
	
}


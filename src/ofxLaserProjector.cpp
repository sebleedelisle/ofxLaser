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

Projector::Projector(string projectorlabel, DacBase& laserdac) : dac(laserdac){
	
	
	laserHomePosition = ofPoint(400,400);
	label = projectorlabel;
	//maskRectangle.set(0,0,800,800);
	
	renderProfiles.emplace(OFXLASER_PROFILE_FAST, ofToString("Fast"));
	renderProfiles.emplace(OFXLASER_PROFILE_DEFAULT, ofToString("Default"));
	renderProfiles.emplace(OFXLASER_PROFILE_DETAIL, ofToString("High quality"));
	
	numTestPatterns = 8;
    guiIsVisible = true; 
	
};

void Projector :: initGui(bool showAdvanced) {
	
	gui = new ofxPanel();
	
	gui->setup(label, label+".xml");
	gui->setName(label);
	
	gui->add(armed.set("ARMED", false));
	gui->add(intensity.set("Intensity", 0.1,0,1));
	gui->add(testPattern.set("Test Pattern", 0,0,numTestPatterns));
	
	ofParameterGroup projectorparams;
	projectorparams.setName("Projector settings");
	projectorparams.add(pps.set("Points per second", 30000,3000,80000));
	pps.addListener(this, &Projector::ppsChanged);
	
	
    if(showAdvanced) {
        projectorparams.add(speedMultiplier.set("Speed Multiplier", 1,0.01,2));
		projectorparams.add(targetFramerate.set("Target framerate", 25, 1, 100));
		projectorparams.add(syncToTargetFramerate.set("Sync to Target framerate", false));
		
    }
    else speedMultiplier = 1;
    
	projectorparams.add(colourChangeOffset.set("Colour change offset", 0,-3,3));
	projectorparams.add(laserOnWhileMoving.set("Laser on while moving", false));
	projectorparams.add(moveSpeed.set("Move Speed", 5,0.1,15));
	projectorparams.add(shapePreBlank.set("Blank points before", 1,0,8));
	projectorparams.add(shapePreOn.set("On points before", 1,0,8));
	projectorparams.add(shapePostOn.set("On points after", 1,0,8));
	projectorparams.add(shapePostBlank.set("Blank points after", 1,0,8));
	
	projectorparams.add(flipX.set("Flip X", false));
	projectorparams.add(flipY.set("Flip Y",false));
	projectorparams.add(outputOffset.set("Output position offset", ofVec2f(0,0), ofVec2f(-20,-20),ofVec2f(20,20)));
	
	
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
	for(int i = 0; i<zones.size(); i++) {
		zonemutegroup.add(zonesMuted[i].set(zones[i]->label, false));
	}
	gui->add(zonemutegroup);
    
    ofParameterGroup zonesologroup;
    zonesologroup.setName("Solo Zones");
    zonesSoloed.resize(zones.size());
    for(int i = 0; i<zones.size(); i++) {
        zonesologroup.add(zonesSoloed[i].set(zones[i]->label, false));
    }
    
    gui->add(zonesologroup);
	
	
    
	for(int i = 0; i<zones.size(); i++) {
		
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
		zonewarpgroup.add(zoneWarps[i]->params);
		gui->add(zonewarpgroup);
	
	}
	
	minimiseGui();
	
	gui->loadFromFile(label+".xml");
	armed = false;
	testPattern = 0;
    for(int i = 0; i<zonesSoloed.size(); i++ ) zonesSoloed[i] = false;
    
    initListeners();
    


	
}



void Projector :: minimiseGui() {
	gui->minimizeAll();
	ofxGuiGroup* g;
	//	g = dynamic_cast <ofxGuiGroup *>(gui->getControl(projectorlabel));
	//	if(g) g->maximize();
	g = dynamic_cast <ofxGuiGroup *>(gui->getControl("Projector settings"));
	if(g) g->maximize();
    g = dynamic_cast <ofxGuiGroup *>(g->getControl("Output position offset"));
    if(g) g->minimize();
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
	dac.setPointsPerSecond(pps);
}


void Projector::addZone(Zone* zone, float srcwidth, float srcheight) {

	if(std::find(zones.begin(), zones.end(), zone) != zones.end()) {
		ofLog(OF_LOG_ERROR, "Projector::addZone(...) - Projector already contains zone");
		return;
	}
	
	zones.push_back(zone);
	
	zoneWarps.push_back(new ZoneTransform(zone->label.substr(4,4), "WarpProjector"+label+"Zone"+ofToString(zones.size()-1)));
	ZoneTransform& warp = *zoneWarps.back();
    
    zonesMuted.resize(zones.size());
    zonesSoloed.resize(zones.size());
    zonesEnabled.resize(zones.size());

	warp.init(zone->rect); 
//	warp.setSrc(zone->rect);
//	ofRectangle destRect = zone->rect;
//	destRect.scale(800/srcwidth, 800/srcheight);
//	destRect.x*=800/srcwidth;
//	destRect.y*=800/srcheight;
//	warp.setDst(destRect);
//	warp.scale = 1;
//	warp.offset.set(0,0);
	
	//warp.updateHomography();
	
	zoneMasks.emplace_back(zone->rect);
	
	leftEdges.push_back(0);
	rightEdges.push_back(0);
	topEdges.push_back(0);
	bottomEdges.push_back(0);
	
	//warp.loadSettings();
}

void Projector::zoneMaskChanged(ofAbstractParameter& e) {
	updateZoneMasks();
}

void Projector::updateZoneMasks() {
	
	for(int i = 0; i<zoneMasks.size(); i++) {
		
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


void Projector::drawUI(bool fullscreen) {
	
	ofPushStyle();
	ofNoFill();
    
    float warpscale = fullscreen ? (float)ofGetHeight()/800.0f : 1;
    ofPushMatrix();
    ofScale(warpscale,warpscale);
	ofDrawRectangle(0,0,800,800);
	ofPoint p;
    
    ofPoint mousepoint (ofGetMouseX(), ofGetMouseY());
	
	//ofDisableBlendMode();
	
//	for(int i = 0; i<zoneWarps.size(); i++) {
//		
//		if(zonesEnabled[i]) continue;
//		
//		ofSetColor(zonesEnabled[i]? 255: 50);
//		QuadWarp& warp = *zoneWarps[i];
//		warp.draw();
//		
//		
//	}
	
    
	for(int i = 0; i<zoneWarps.size(); i++) {
		
		
		ZoneTransform& warp = *zoneWarps[i];
		
		warp.setVisible(zonesEnabled[i]);
        
        
                         
		if(!zonesEnabled[i]) continue;
		ofRectangle& mask = zoneMasks[i];
		
		ofPushMatrix();
		ofTranslate(ofPoint(outputOffset)*warpscale);
		ofFill();
		ofSetColor(0,0,255,40);

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
		
		ofNoFill();
		ofSetColor(0,0,255,200);
		
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
		
		
		ofPopMatrix();

	}
    
    ofPopMatrix();
    
	ofNoFill();
	ofSetColor(255);
	
	for(int i = 0; i<zoneWarps.size(); i++) {
		if(!zonesEnabled[i]) continue;
		ZoneTransform& warp = *zoneWarps[i];
		//warp.offset = ofPoint(outputOffset)*warpscale;
        warp.scale = warpscale;
		warp.draw();
	}
	
	
	ofNoFill();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofPushMatrix();
    ofScale(warpscale, warpscale);
	ofSetColor(100);
    ofSetLineWidth(0.5);
    
	previewPathMesh.setMode(OF_PRIMITIVE_POINTS);
	previewPathMesh.draw();
	
	for(int i = 0; i<previewPathMesh.getNumVertices();i++) {
		previewPathMesh.addColor(ofColor(ofMap(i,0,previewPathMesh.getNumVertices(), 255,0),0,255));
	}
	
	previewPathMesh.setMode(OF_PRIMITIVE_LINE_STRIP);
	previewPathMesh.draw();
	
	if(previewPathMesh.getVertices().size()>0) {
		
		float time = previewPathMesh.getVertices().size()/50.0f;
		int pointindex =ofMap(fmod(ofGetElapsedTimef(),time),0,time,0,previewPathMesh.getVertices().size());
		ofPoint p = previewPathMesh.getVertices().at(pointindex);
		ofSetColor(255);
		ofDrawCircle(p, 6);
		ofFill();
		Point& lp =laserPoints[pointindex];
		ofSetColor(lp.getColor()*255);
		ofDrawCircle(p, 6);
	}
	
    ofPopMatrix();
	ofDisableBlendMode();
	ofPopStyle();
	
	
	
}

void Projector :: hideGui() {
	
	for(int i = 0; i<zoneWarps.size(); i++) {
		
		
		ZoneTransform& warp = *zoneWarps[i];
		
		warp.setVisible(false);
		
	}
	
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

	
    bool soloMode = false;
    for(int i = 0; i<zonesSoloed.size(); i++) {
        if(zonesSoloed[i]) {
            soloMode = true;
            break;
        }
    }
    if(soloMode) {
        for(int i = 0; i<zonesEnabled.size(); i++ ) {
            zonesEnabled[i] = zonesSoloed[i];
        }
        
    } else {
        for(int i = 0; i<zonesEnabled.size(); i++ ) {
            zonesEnabled[i] = !zonesMuted[i];
        }
    }
    
    
	laserPoints.clear();
	previewPathMesh.clear();
	for(int i = 0; i<zoneWarps.size(); i++) {
		zoneWarps[i]->update();
	}
	
	// if any of the source rectangles have changed then update all the warps
	if(updateZones) {
		for(int i = 0; i<zoneWarps.size(); i++) {
			ZoneTransform& warp = *zoneWarps[i];
			warp.setSrc(zones[i]->rect);
			warp.updateHomography();
			updateZoneMasks();
		}
	}

}


void Projector::sendRawPoints(const vector<ofxLaser::Point>& points, int zonenum ){
    
     //ofLog(OF_LOG_NOTICE, "ofxLaser::Projector::sendRawPoints(...) point count : "+ofToString(points.size()));
    
    Zone& zone = *zones.at(zonenum);
    ofRectangle& maskRectangle = zoneMasks.at(zonenum);
    ZoneTransform& warp = *zoneWarps.at(zonenum);
    bool offScreen = true;
	
    
    vector<Point>segmentpoints;
    
    //iterate through the points
    for(int k = 0; k<points.size(); k++) {
        
        Point p = points[k];
        //previewPathMesh.addVertex(p);
        //ofColor c = p.getColor();
        //previewPathMesh.addColor(c);
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
    
	
	
    processPoints();
    dac.sendPoints(laserPoints);
    
   
}


                        

void Projector::send(ofPixels* pixels) {

	vector<SegmentPoints> allzonesegmentpoints;
	vector<SegmentPoints*> sortedsegmentpoints;
	vector<SegmentPoints> zonesegmentpoints;
	vector<Point> shapepoints;
	deque<Shape*> zoneshapes;
	
	// go through each zone
	for(int i = 0; i<zones.size(); i++) {
		
		if(!zonesEnabled[i]) continue;
		Zone& zone = *zones[i];
		ZoneTransform& warp = *zoneWarps[i];
		ofRectangle& maskRectangle = zoneMasks[i];
		zoneshapes = zone.shapes;
		
		// add testpattern points for this zone...
		// get array of segmentpoints and append them
		deque<Shape*> testPatternShapes = getTestPatternShapesForZone(i);
		
		zoneshapes.insert(zoneshapes.end(), testPatternShapes.begin(), testPatternShapes.end());
		
		zonesegmentpoints.clear(); 
		
		// got through each shape in the zone
		
		for(int j = 0; j<zoneshapes.size(); j++)  {
			
			// get the points
			Shape& shape = *(zoneshapes[j]);
			
			RenderProfile& renderProfile = getRenderProfile(shape.profileLabel);
			
			shapepoints.clear();
			shape.appendPointsToVector(shapepoints, renderProfile, speedMultiplier);
			
			bool offScreen = true;
			
			SegmentPoints segmentpoints;
			
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
							zonesegmentpoints.push_back(segmentpoints); // should copy

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
				zonesegmentpoints.push_back(segmentpoints);
			}
			
		} // end zoneshapes
		
		
		// go through all the points and warp them into projector space
		for(int j = 0; j<zonesegmentpoints.size(); j++) {
			SegmentPoints& segmentpoints = zonesegmentpoints[j];
			for(int k= 0; k<segmentpoints.size(); k++) {

                // Possibly a good time to check against the mask image?
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
		allzonesegmentpoints.insert(allzonesegmentpoints.end(), zonesegmentpoints.begin(), zonesegmentpoints.end());
		
		// delete all the test pattern shapes
		for(int j = 0; j<testPatternShapes.size(); j++) {
			delete testPatternShapes[j];
		}
		testPatternShapes.clear();
		
	} // end zones
	
	// sort the point objects
	if(allzonesegmentpoints.size()>0) {
		bool reversed = false;
		int currentIndex = 0;
		float shortestDistance = INFINITY;
		int nextDotIndex = -1;

		
		do {
			
			SegmentPoints& shape1 = allzonesegmentpoints[currentIndex];
			
			shape1.tested = true;
			sortedsegmentpoints.push_back(&shape1);
			shape1.reversed = reversed;
			
			shortestDistance = INFINITY;
			nextDotIndex = -1;
			
			
			for(int j = 0; j<allzonesegmentpoints.size(); j++) {
				
				SegmentPoints& shape2 = allzonesegmentpoints[j];
				if((&shape1==&shape2) || (shape2.tested)) continue;
				
				shape2.reversed = false;
				
				if(shape1.getEnd().squareDistance(shape2.getStart()) < shortestDistance) {
					shortestDistance = shape1.getEnd().squareDistance(shape2.getStart());
					nextDotIndex = j;
					reversed = false;
				}
				
				if((shape2.reversable) && (shape1.getEnd().squareDistance(shape2.getEnd()) < shortestDistance)) {
					shortestDistance = shape1.getEnd().squareDistance(shape2.getEnd());
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
		
		for(int j = 0; j<sortedsegmentpoints.size(); j++) {
			SegmentPoints& segmentpoints = *sortedsegmentpoints[j];
			if(segmentpoints.size()==0) continue;
			
			if(currentPosition.distance(segmentpoints.getStart())>2){
				addPointsForMoveTo(currentPosition, segmentpoints.getStart());
			
				for(int k = 0; k<shapePreBlank; k++) {
					addPoint((ofPoint)segmentpoints.getStart(), ofColor(0));
				}
				for(int k = 0;k<shapePreOn;k++) {
					addPoint(segmentpoints.getStart());
				}
			}
			if(segmentpoints.reversed) {
				for(int k=segmentpoints.size()-1;k>=0; k--) {
					addPoint((Point)segmentpoints[k]);
				}
			} else {
				addPoints(segmentpoints);

			}
			
			currentPosition = segmentpoints.getEnd();
			
			if(j<sortedsegmentpoints.size()-1) {
				SegmentPoints& nextsegment = *sortedsegmentpoints[j+1];
				if(currentPosition.distance(nextsegment.getStart())>2){
					for(int k = 0;k<shapePostOn;k++) {
						addPoint(segmentpoints.getEnd());
					}
					for(int k = 0; k<shapePostBlank; k++) {
						addPoint((ofPoint)segmentpoints.getEnd(), ofColor(0));
					}
				}
			}
			
			
		}
		addPointsForMoveTo(currentPosition, laserHomePosition);
		
	}
	
	
	if (syncToTargetFramerate) {
		targetFramerate = round(targetFramerate * 100) / 100.0f;
		float targetNumPoints = (float)pps / targetFramerate;
		// TODO make this a proper system!
		if (ofGetKeyPressed(OF_KEY_LEFT)) targetNumPoints -= 10;
		if (ofGetKeyPressed(OF_KEY_RIGHT)) targetNumPoints += 10;
		while (laserPoints.size() < targetNumPoints) {
			addPoint(laserHomePosition, ofColor::black);
		}
	}
	
	
	processPoints();
	dac.sendFrame(laserPoints);
    numPoints = laserPoints.size();
	
	if(sortedsegmentpoints.size()>0) laserHomePosition += (sortedsegmentpoints.front()->getStart()-laserHomePosition)*0.02;
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
		shapes.push_back(new Line(rect.getTopLeft(),rect.getTopLeft().getInterpolated(rect.getBottomLeft(), 1.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		shapes.push_back(new Line(rect.getBottomLeft(),rect.getTopLeft().getInterpolated(rect.getBottomLeft(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		shapes.push_back(new Line(rect.getTopRight().getInterpolated(rect.getBottomRight(), 1.0f/3.0f),rect.getTopRight().getInterpolated(rect.getBottomRight(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		
	} else if(testPattern==4) {
		
		ofRectangle& rect = zone.rect;
		
		ofPoint v = rect.getBottomRight() - rect.getTopLeft()-ofPoint(0.2,0.2);
		
		for(float x =0 ; x<=1.1; x+=0.3333333333) {
			shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1+ v.x*x, rect.getTop()+0.1),ofPoint(rect.getLeft()+0.1 + v.x*x, rect.getBottom()-0.1), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT ));
			
		}
		
		shapes.push_back(new Line(rect.getTopLeft(),rect.getTopLeft().getInterpolated(rect.getTopRight(), 1.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		shapes.push_back(new Line(rect.getTopRight(),rect.getTopLeft().getInterpolated(rect.getTopRight(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		shapes.push_back(new Line(rect.getBottomLeft().getInterpolated(rect.getBottomRight(), 1.0f/3.0f),rect.getBottomLeft().getInterpolated(rect.getBottomRight(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		
		
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
		points.emplace_back(c, (laserOnWhileMoving && j%2==0) ? ofColor(200,0,0) : ofColor(0));
		
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
void Projector :: addPoints(vector<ofxLaser::Point>&points) {
	for(int i = 0; i<points.size();i++) {
		addPoint(points[i]);
	}
}

void Projector :: addPoint(ofxLaser::Point p) {
	
	p+=(ofPoint)outputOffset;
	
	laserPoints.push_back(p);
	
	previewPathMesh.addVertex(ofPoint(p.x, p.y));

}



void  Projector :: processPoints() {
			
	// Some lasers change colour too early/late, so the colourChangeOffset system
	// mitigates against that by shifting the colours for the points.
	// Some optimisation makes this slightly hard to read but we iterate through
	// the points forwards or backwards depending on which direction we have to shift.
	// To avoid creating a whole new vector, we're storing the overlap in a buffer of
	// colours.
	// I'm sure there must be some slicker C++ way of doing this...
	
	int colourChangeIndexOffset = (float)pps/10000.0f*colourChangeOffset ;
	

	if(colourChangeIndexOffset>0) {
		vector<ofColor> colourBuffer(colourChangeIndexOffset);
		
		for(int i = 0; i<laserPoints.size(); i++) {
			
			Point& p = laserPoints[i];
			// todo : check colour range is 0 to 255
			if(i<colourChangeIndexOffset) colourBuffer[i].set(p.r, p.g, p.b);
			
			int colourPointIndex = i+colourChangeIndexOffset;
			ofColor c;
			if(colourPointIndex>=laserPoints.size()) {
				c = colourBuffer[colourPointIndex%laserPoints.size()];
			
			} else {
				Point& colourPoint = laserPoints[colourPointIndex];
				c.set(colourPoint.r, colourPoint.g, colourPoint.b);
			}
			p.r = c.r;
			p.g = c.g;
			p.b = c.b;

		}
		
	} else {
		// change negative to positive
		colourChangeIndexOffset*=-1;
		vector<ofColor> colourBuffer(colourChangeIndexOffset);
		int bufferStartIndex = laserPoints.size()-colourChangeIndexOffset;
		
		for(int i = laserPoints.size()-1; i>=0; i--) {
			
			Point& p = laserPoints[i];
			// todo : check colour range is 0 to 255
			if(i>=bufferStartIndex) {
			//	ofLog(OF_LOG_NOTICE, "storing colour "+ofToString(i)+" in " + ofToString( i-bufferStartIndex));
				colourBuffer[i-bufferStartIndex].set(p.r, p.g, p.b);
				
			}
			int colourPointIndex = i-colourChangeIndexOffset;
			ofColor c;
			if(colourPointIndex<0) {
				c = colourBuffer[i];
			//	ofLog(OF_LOG_NOTICE, "getting colour for "+ofToString(i)+" from " + ofToString( i));
				
			} else {
				Point& colourPoint = laserPoints[colourPointIndex];
				c.set(colourPoint.r, colourPoint.g, colourPoint.b);
			}
			p.r = c.r;
			p.g = c.g;
			p.b = c.b;
			
		}
	}
	

	
	for(int i = 0; i<laserPoints.size(); i++) {
		
		ofxLaser::Point &p = laserPoints[i];
		

		//p.y+=outputOffset;
//		ofColor maskPixelColour;
//		if(useMaskBitmap) {
//			maskPixelColour = maskBitmap.getColor(p.x/appWidth* (float)maskBitmap.getWidth(), p.y/appHeight * (float)maskBitmap.getHeight());
//			
//		}
		
		
		if(flipY) p.y= 800-p.y;
		if(flipX) p.x= 800-p.x;
		
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
			p.r = calculateCalibratedBrightness(p.r, intensity, red100, red75, red50, red25, red0);
			p.g = calculateCalibratedBrightness(p.g, intensity, green100, green75, green50, green25, green0);
			p.b = calculateCalibratedBrightness(p.b, intensity, blue100, blue75, blue50, blue25, blue0);
		}
		
		//p.r = std::__1::max(p.r, std::__1::max(p.g, p.b));
		
		if(!armed) {
			p.r = 0;
			p.g = 0;
			p.b = 0;
		}
	//	ildaPoints.push_back(ofxIlda::Point(p, c, pmin, pmax));
		
	}
//	int minPoints = 200;
//	while(ildaPoints.size()<minPoints) {
//		ofxLaser::Circle blank(laserHomePosition + ofPoint(0,10), 10, ofFloatColor(0), 2,2,0);
//		shapepoints.clear();
//
//		blank.appendPointsToVector(shapepoints);
//		addPoints(shapepoints);
//
//
//	}
	
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
	gui->saveToFile(label+".xml");
	for(int i = 0; i<zoneWarps.size(); i++) {
		zoneWarps[i]->saveSettings();
	}
	
}


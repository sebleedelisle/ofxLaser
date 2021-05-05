//
//  ofxLaserProjector.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserProjector.h"

using namespace ofxLaser;

Projector::Projector(int _index) {
    projectorIndex = _index;
    dac = &emptyDac;
    
	laserHomePosition = ofPoint(400,400);
	//id = projectorlabel;
	 
 	
	numTestPatterns = 9;
 	
	guiInitialised = false;
    maskManager.init(800,800);
   
    
	
};

Projector::~Projector() {
    
    // NOTE that the manager saves the projector settings when it closes 
	ofLog(OF_LOG_NOTICE, "ofxLaser::Projector destructor called");
	pps.removeListener(this, &Projector::ppsChanged);
	armed.removeListener(this, &ofxLaser::Projector::setDacArmed);
    ofRemoveListener(params.parameterChangedE(), this, &Projector::paramsChanged);
   
	if(dac!=nullptr) dac->close();
	//delete gui;
}

void Projector::setDac(DacBase* newdac){
    if(dac!=newdac) {
        dac = newdac;
        newdac->setPointsPerSecond(pps);
        dacId = dac->getId();
        armed = false; // automatically calls setArmed because of listener on parameter
    }
    
}
DacBase* Projector::getDac(){
    return dac;
    
}

bool Projector::hasDac() {
    return (dac != &emptyDac);
    
}
bool Projector::removeDac(){
	if (dac != &emptyDac) {
		dac = &emptyDac;
		dacId = "";
		return true;
	}
	else {
		return false;
	}
}



void Projector::setDefaultHandleSize(float size) {
	
	defaultHandleSize = size;
	//for(ZoneTransform* zonetrans : zoneTransforms) {
	//	zonetrans->setHandleSize(defaultHandleSize);
	//}
	
}

void Projector :: init() {

    // TODO is this used for anything other than display?
	params.setName(ofToString(projectorIndex));
	
//    params.add(armed.set("ARMED", false));
    params.add(intensity.set("Brightness", 1,0,1));
//    params.add(testPattern.set("Test Pattern", 0,0,numTestPatterns));
    armed.set("ARMED", false);
    testPattern.set("Test Pattern", 0,0,numTestPatterns);
    
    params.add(dacId.set("dacId", ""));
    
    hideContentDuringTestPattern.set("Test pattern only", true);
	ofParameterGroup projectorparams;
	projectorparams.setName("Projector settings");
	
    projectorparams.add(speedMultiplier.set("Speed", 1,0.12,2));
    
	 
	projectorparams.add(colourChangeShift.set("Colour shift", 2,0,6));
		
	projectorparams.add(flipX.set("Flip Horizontal", false));
	projectorparams.add(flipY.set("Flip Vertical",false));
	projectorparams.add(outputOffset.set("Output position offset", glm::vec2(0,0), glm::vec2(-20,-20),glm::vec2(20,20)));
	projectorparams.add(rotation.set("Output rotation",0,-90,90));

	
	ofParameterGroup& advanced = advancedParams;
    advanced.setName("Advanced");
    projectorparams.add(pps.set("Points per second", 30000,1000,80000));
    advanced.add(laserOnWhileMoving.set("Laser on while moving", false));
	advanced.add(smoothHomePosition.set("Smooth home position", true));
	advanced.add(sortShapes.set("Optimise shape draw order", true));
	advanced.add(targetFramerate.set("Target framerate", 25, 23, 120));
	advanced.add(syncToTargetFramerate.set("Sync to Target framerate", false));
	advanced.add(syncShift.set("Sync shift", 0, -50, 50));

	projectorparams.add(advanced);
	
	params.add(projectorparams);
    params.add(scannerSettings.params);
	
	ofParameterGroup renderparams;
	renderparams.setName("Render profiles");
	
	   
	
	
	params.add(colourSettings.params);

     
     
    armed.addListener(this, &ofxLaser::Projector::setDacArmed);
    pps.addListener(this, &Projector::ppsChanged);
  

    //loadSettings();
   
    dac->setPointsPerSecond(pps);
	// error checking on blank shift for older config files
	if(colourChangeShift<0) colourChangeShift = 0;

//	for(size_t i = 0; i<zoneTransforms.size(); i++) {
//		zoneTransforms[i]->initGuiListeners();
//		zoneTransforms[i]->loadSettings();
//	}
	
	armed = false;
	testPattern = 0;
    //for(size_t i = 0; i<zonesSoloed.size(); i++ ) zonesSoloed[i] = false;
    
    ofAddListener(params.parameterChangedE(), this, &Projector::paramsChanged);
   
	guiInitialised = true;

    //masks.resize(5); 
	
}





void Projector ::setDacArmed(bool& _armed){
    dac->setArmed(_armed);
    
}

bool Projector ::toggleArmed() {
    armed = !armed;
    return armed;
}

void Projector:: ppsChanged(int& e){
	ofLog(OF_LOG_NOTICE, "ppsChanged"+ofToString(pps));
	pps=round(pps/100)*100;
	if(pps<=100) pps =100;
	dac->setPointsPerSecond(pps);
}


void Projector::addZone(Zone* zone, float srcwidth, float srcheight) {

	if(hasZone(zone)) {
		ofLog(OF_LOG_ERROR, "Projector::addZone(...) - Projector already contains zone");
		return;
	}
    
    ProjectorZone* projectorZone = new ProjectorZone(*zone);
    projectorZones.push_back(projectorZone);
    
    // initialise zoneTransform
    projectorZone->zoneTransform.init(zone->rect);
    //    ofRectangle destRect = zone->rect;
    //    destRect.scale(400/srcwidth, 400/srcheight);
    //    destRect.x*=400/srcwidth;
    //    destRect.x+=200;
    //    destRect.y*=400/srcheight;
    //    zoneTransform.setDst(destRect);
    projectorZone->zoneMask = zone->rect;
    
    std::sort(projectorZones.begin(), projectorZones.end(), [](const ProjectorZone* a, const ProjectorZone* b) -> bool
    {
        
        return (a->getZoneIndex()<b->getZoneIndex()); // a.mProperty > b.mProperty;
    });
     
}

bool Projector :: hasZone(Zone* zone){
    for(ProjectorZone* projectorZone : projectorZones) {
        if(zone == &projectorZone->zone) return true;
    }
    return false;
    
}
bool Projector :: removeZone(Zone* zone){

    ProjectorZone* projectorZone = getProjectorZoneForZone(zone);
    if(projectorZone==nullptr) return false;
    
    vector<ProjectorZone*>::iterator it = std::find(projectorZones.begin(), projectorZones.end(), projectorZone);

    // TODO Check cleanup
    projectorZones.erase(it);
    delete projectorZone;
    
    return true;
    
    
    
}


void Projector::updateZoneMasks() {
	
    for(ProjectorZone* projectorZone : projectorZones) {
        
        projectorZone->updateZoneMask();
    }
    
    /*
	for(size_t i = 0; i<zoneMasks.size(); i++) {
		
		ofRectangle& zoneMask = zoneMasks[i];
		Zone& zone = *projectorZones[i];
		
		ofParameter<float>& leftEdge = leftEdges[i];
		ofParameter<float>& rightEdge = rightEdges[i];
		ofParameter<float>& topEdge = topEdges[i];
		ofParameter<float>& bottomEdge = bottomEdges[i];
		zoneMask.setX(zone.rect.getLeft()+(leftEdge*zone.rect.getWidth()));
		zoneMask.setY(zone.rect.getTop()+(topEdge*zone.rect.getHeight()));
		zoneMask.setWidth(zone.rect.getWidth()*(1-leftEdge-rightEdge));
		zoneMask.setHeight(zone.rect.getHeight()*(1-topEdge-bottomEdge));
	}*/
}


string Projector::getDacLabel() {
    if(dac!=&emptyDac) {
        return dac->getId();
    } else {
        return "No laser controller assigned";
    }
}

int Projector::getDacConnectedState() {
    
    if(dac!=nullptr) {
        return dac->getStatus();
    } else {
        return OFXLASER_DACSTATUS_ERROR;
    }
}

void Projector::drawTransformUI(float x, float y, float w, float h) {
	
	ofPushStyle();
	ofNoFill();

    float scale = w/800.0f;
    ofPoint offset = ofPoint(x,y) + (ofPoint(outputOffset)*scale);
    for(ProjectorZone* projectorZone : projectorZones) {
        //if(!projectorZone->getEnabled()) continue;
        projectorZone->setScale(scale);
        projectorZone->setOffset(offset);
        projectorZone->draw();
        
        
    }
    maskManager.setOffsetAndScale(offset,scale);
    maskManager.draw();
   
    ofPopStyle();
}   

void Projector::drawTransformAndPath(ofRectangle rect) {
    ofRectangle bounds;
    vector<glm::vec3> perimeterpoints;
    bool firsttime = true;
    for(ProjectorZone* zone : projectorZones) {
        ZoneTransform& zonetransform = zone->zoneTransform;
       
        zonetransform.getPerimeterPoints(perimeterpoints);
        if(firsttime) {
            bounds.setPosition(*perimeterpoints.begin());
            firsttime = false;
        }
        for(glm::vec3& p:perimeterpoints) {
            bounds.growToInclude(p);
        }
        
        
    }
    //drawTransformUI(rect.x, rect.y, rect.width, rect.height);
    
    ofPushMatrix();
    ofTranslate(rect.x, rect.y);
    ofScale(rect.width/800, rect.height/800);
    ofPushStyle();
    ofNoFill();
    ofSetColor(50,50,200);
   
    float scale = 800.0f/bounds.width;
    if(800/bounds.height<scale) scale = 800/bounds.height;
    ofScale(scale, scale);
    ofTranslate(-bounds.x, -bounds.y); //getTopLeft());
    drawLaserPath(ofRectangle(0,0,800,800), false);
   
   // ofDrawRectangle(bounds);
    for(ProjectorZone* zone : projectorZones) {

        zone->zoneTransform.getPerimeterPoints(perimeterpoints);
        
        ofBeginShape();
        for(glm::vec3& p:perimeterpoints) {
            ofVertex(p);
        }
        ofEndShape();
        
    }
    ofPopStyle();
    ofPopMatrix();
    
}


void Projector :: drawLaserPath(ofRectangle rect, bool drawDots) {
	drawLaserPath(rect.x, rect.y, rect.width, rect.height, drawDots);
}
void Projector :: drawLaserPath(float x, float y, float w, float h, bool drawDots) {
	ofPushStyle();
	
	ofSetColor(255);

	ofDrawBitmapString(ofToString(projectorIndex+1), x+w-20, y+20);
	ofSetColor(100);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofPushMatrix();
	ofTranslate(x,y);
	ofScale(w/800, h/800);

	ofTranslate(outputOffset);

	ofPoint p;

  	ofNoFill();
	//ofSetColor(255);
    ofSetColor(MIN(255 * w / 800.0f, 255));
	ofSetLineWidth(0.5);
	
	previewPathMesh.setMode(OF_PRIMITIVE_POINTS);
	if(drawDots) previewPathMesh.draw();
	
    // draw the coloured line in the background
	for(size_t i = 0; i<previewPathMesh.getNumVertices();i++) {
		previewPathMesh.addColor(ofColor::fromHsb(ofMap(i,0,previewPathMesh.getNumVertices(), 227, 128),255,255));
	}

	
	
	ofSetLineWidth(2 * w / 800.0f);
	previewPathMesh.setMode(OF_PRIMITIVE_LINE_STRIP);
	previewPathMesh.draw();
	
	// draws the animated laser path
	
	if(previewPathMesh.getNumVertices()>0) {
		
        // 100 points per second
		float time = previewPathMesh.getNumVertices()/100.0f;
		int pointindex =ofMap(fmod(ofGetElapsedTimef(),time),0,time,0,previewPathMesh.getNumVertices());
        
        Point& lp =laserPoints[pointindex];
        ofPoint p = previewPathMesh.getVertex(pointindex);
        ofColor c = lp.getColour()*255;
        
        if(c==ofColor::black) {
            ofSetColor(200);
            ofDrawCircle(p, 3);
        } else {
            ofFill();
            c.setBrightness(255);
            ofSetColor(c);
            ofDrawCircle(p, 4);
            ofNoFill();
            ofSetLineWidth(2); 
            c.setBrightness(128);
            ofSetColor(c);
            ofDrawCircle(p, 6);
        }
	}
	

	ofDisableBlendMode();
	ofPopStyle();
	
	
	ofPopMatrix();
	
}

void Projector :: disableTransformGui() {
	
    for(ProjectorZone* projectorZone : projectorZones) {
        projectorZone->setEnabled(false);
    }
	
	
}
void Projector :: enableTransformGui() {
    for(ProjectorZone* projectorZone : projectorZones) {
        if(projectorZone->getVisible()) projectorZone->setEnabled(true);
    }
	
}




void Projector::update(bool updateZones) {
	
    bool soloMode = false;
    bool needsSave = false;
    for(ProjectorZone* projectorZone : projectorZones) {
        
        if(projectorZone->soloed) {
            soloMode = true;
            break;
        }
    }
    
    if(soloMode) {
        for(ProjectorZone* projectorZone : projectorZones) {
            projectorZone->setVisible(projectorZone->soloed);
        }
        
    } else {
        for(ProjectorZone* projectorZone : projectorZones) {
            projectorZone->setVisible(!projectorZone->muted);
        }
    }
    
    
    // if any of the source rectangles have changed then update all the warps
    // (shouldn't need anything saving)
    if(updateZones) {
        for(ProjectorZone* projectorZone : projectorZones) {
            ZoneTransform& warp = projectorZone->zoneTransform;
            warp.setSrc(projectorZone->zone.rect);
            warp.updateHomography();
            updateZoneMasks();
        }
    }
    
    needsSave = maskManager.update() | needsSave;
    
    
	laserPoints.clear();
	previewPathMesh.clear();
    bool projectorZoneChanged = false;
    for(ProjectorZone* projectorZone : projectorZones) {
        projectorZoneChanged |= projectorZone->update();
	}
	
    needsSave |= projectorZoneChanged;
    float framerate = getFrameRate();
	smoothedFrameRate += (framerate - smoothedFrameRate)*0.2;
    frameTimeHistory[frameTimeHistoryOffset] = 1/framerate;
    frameTimeHistoryOffset++;
    if(frameTimeHistoryOffset>=frameTimeHistorySize) frameTimeHistoryOffset = 0;
    
    if(needsSave) saveSettings();
 
}


void Projector::sendRawPoints(const vector<ofxLaser::Point>& points, Zone* zone, float masterIntensity ){
    
     //ofLog(OF_LOG_NOTICE, "ofxLaser::Projector::sendRawPoints(...) point count : "+ofToString(points.size()));
    
    ProjectorZone& projectorZone = *getProjectorZoneForZone(zone);
    ofRectangle& maskRectangle = projectorZone.zoneMask;
    ZoneTransform& warp = projectorZone.zoneTransform;
    bool offScreen = true;
    
    vector<Point>segmentpoints;
    
    //iterate through the points
    for(size_t k = 0; k<points.size(); k++) {
        
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

    for(size_t k= 0; k<segmentpoints.size(); k++) {
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
	
	vector<PointsForShape> allzoneshapepoints;

	// TODO add speed multiplier to getPointsForMove function
	getAllShapePoints(&allzoneshapepoints, pixels, speedMultiplier);
	
	vector<PointsForShape*> sortedshapepoints;
	
	
	// sort the point objects
	if(allzoneshapepoints.size()>0) {
		bool reversed = false;
		int currentIndex = 0;
		float shortestDistance = INFINITY;
		int nextDotIndex = -1;

		if(sortShapes) {
			do {
				
				PointsForShape& shapePoints1 = allzoneshapepoints[currentIndex];
				
				shapePoints1.tested = true;
				sortedshapepoints.push_back(&shapePoints1);
				shapePoints1.reversed = reversed;
				
				shortestDistance = INFINITY;
				nextDotIndex = -1;
				
				
				for(size_t j = 0; j<allzoneshapepoints.size(); j++) {
					
					PointsForShape& shapePoints2 = allzoneshapepoints[j];
					if((&shapePoints1==&shapePoints2) || (shapePoints2.tested)) continue;
					
					shapePoints2.reversed = false;
					
					if(shapePoints1.getEnd().squareDistance(shapePoints2.getStart()) < shortestDistance) {
						shortestDistance = shapePoints1.getEnd().squareDistance(shapePoints2.getStart());
						nextDotIndex = (int)j;
						reversed = false;
					}
					
					if((shapePoints2.reversable) && (shapePoints1.getEnd().squareDistance(shapePoints2.getEnd()) < shortestDistance)) {
						shortestDistance = shapePoints1.getEnd().squareDistance(shapePoints2.getEnd());
						nextDotIndex = (int)j;
						reversed = true;
					}
					
					
				}
				
				currentIndex = nextDotIndex;
				
				
				
			} while (currentIndex>-1);
		} else {
			for(size_t j = 0; j<allzoneshapepoints.size(); j++) {
				sortedshapepoints.push_back(&allzoneshapepoints[j]);
			}
			
		}
		

		// go through the point objects
		// add move between each one
		// add points to the laser
		
		ofPoint currentPosition = laserHomePosition; // MUST be in projector space
		
		for(size_t j = 0; j<sortedshapepoints.size(); j++) {
			PointsForShape& shapepoints = *sortedshapepoints[j];
			if(shapepoints.size()==0) continue;
			
			if(currentPosition.distance(shapepoints.getStart())>2){
				addPointsForMoveTo(currentPosition, shapepoints.getStart());
			
				for(int k = 0; k<scannerSettings.shapePreBlank; k++) {
					addPoint((ofPoint)shapepoints.getStart(), ofColor(0));
				}
				for(int k = 0;k<scannerSettings.shapePreOn;k++) {
					addPoint(shapepoints.getStart());
				}
			}
			
			addPoints(shapepoints, shapepoints.reversed);

			currentPosition = shapepoints.getEnd();
			
			PointsForShape* nextshapepoints = nullptr;
			
			if(j<sortedshapepoints.size()-1) {
				nextshapepoints = sortedshapepoints[j+1];
			}
			if((nextshapepoints==nullptr) || (currentPosition.distance(nextshapepoints->getStart())>2)){
				for(int k = 0;k<scannerSettings.shapePostOn;k++) {
					addPoint(shapepoints.getEnd());
				}
				for(int k = 0; k<scannerSettings.shapePostBlank; k++) {
					addPoint((ofPoint)shapepoints.getEnd(), ofColor(0));
				}
			}
		
			
			
		}
		if(smoothHomePosition) addPointsForMoveTo(currentPosition, laserHomePosition);
		
	}
	
	if (laserPoints.size() == 0) {
		laserPoints.push_back(Point(laserHomePosition, ofColor(0)));
	}
	int targetNumPoints;
    
	// TODO add system to speed up if too much stuff to draw
	if (syncToTargetFramerate) {
		
		targetNumPoints = round((float)pps / targetFramerate);
		
		if(syncShift!=0) {
			targetNumPoints+=syncShift;
			if(!ofGetMousePressed()) syncShift = 0;
		}
		
		while (laserPoints.size() < targetNumPoints) {
			addPoint(laserHomePosition, ofColor::black);
		}
	}
	
	processPoints(masterIntensity);
	
	if(syncToTargetFramerate && (laserPoints.size()!=targetNumPoints)) {
	
		ofLogError("syncToTargetFramerate failed! " + ofToString(targetNumPoints)+ " " + ofToString(laserPoints.size()));
	}
	
	dac->sendFrame(laserPoints);
    numPoints = (int)laserPoints.size();
	
	if(sortedshapepoints.size()>0) {
		if(smoothHomePosition) {
			laserHomePosition += (sortedshapepoints.front()->getStart()-laserHomePosition)*0.05;
		} else {
			laserHomePosition = sortedshapepoints.back()->getEnd();
		}
	}
}


void Projector ::getAllShapePoints(vector<PointsForShape>* shapepointscontainer, ofPixels*pixels, float speedmultiplier){
	
	vector<PointsForShape>& allzoneshapepoints = *shapepointscontainer;
	
	// temp vectors for storing the shapes in
	vector<PointsForShape> zonePointsForShapes;
	vector<Point> shapePointBuffer;
	
	// go through each zone
	//for(int i = 0; i<(int)projectorZones.size(); i++) {
    for(ProjectorZone* projectorZone : projectorZones) {
        //ProjectorZone* projectorZone = projectorZones[i];
		if(!projectorZone->getVisible()) continue;
        
		Zone& zone = projectorZone->zone;
        ZoneTransform& warp = projectorZone->zoneTransform;
		ofRectangle& maskRectangle = projectorZone->zoneMask;
        
        // doesn't make a copy, just a pointer to the original shapes in the zone
        // CHECK - is this OK ?
		deque<Shape*>* zoneshapes = &zone.shapes;
		
        // get test pattern shapes - we have to do this even if
        // we don't have a test pattern, so that the code at the end
        // of this function can delete the shapes.
        deque<Shape*> testPatternShapes = getTestPatternShapesForZone(*projectorZone);
        
        // define this here so we don't lose scope
        deque<Shape*> zoneShapesWithTestPatternShapes;
        
        if(testPattern>0) {
            // copy zone shapes into it
            if(!hideContentDuringTestPattern) zoneShapesWithTestPatternShapes = zone.shapes;
            
            // add testpattern points for this zone...
            zoneShapesWithTestPatternShapes.insert(zoneShapesWithTestPatternShapes.end(), testPatternShapes.begin(), testPatternShapes.end());
            zoneshapes = &zoneShapesWithTestPatternShapes;
        }
        
        
        // so this is either going to be the test pattern shapes or
        // a reference to the zone shapes
        deque<Shape*>& shapesInZone = *zoneshapes;
        
        // reuse the last vector of shapepoints
		zonePointsForShapes.clear();
		
		// go through each shape in the zone
		
		for(size_t j = 0; j<shapesInZone.size(); j++)  {
			
			// get the points
			Shape& shape = *(shapesInZone[j]);
			
			RenderProfile& renderProfile = getRenderProfile(shape.profileLabel);
			
            // calculate the points for the shape and put them
            // in the temporary point storage buffer.
			shapePointBuffer.clear();
			shape.appendPointsToVector(shapePointBuffer, renderProfile, speedmultiplier);
			
            // now we need to go through the shape and see if any of it is
            // off the edge of the input zone. If it is we split the
            // shape up into separate segments.
            
			bool offScreen = true;
			PointsForShape segmentPoints;
            segmentPoints.reversable = shape.reversable;
            
			//iterate through the points
			for(int k = 0; k<shapePointBuffer.size(); k++) {
				
				Point& p = shapePointBuffer[k];
				
				// check each point against the mask
				// are we outside the edge mask?
                // NB can't use inside because I want points on the edge
				
                // if we are outside the mask area
				if(p.x<maskRectangle.getLeft() ||
				   p.x>maskRectangle.getRight() ||
				   p.y<maskRectangle.getTop() ||
				   p.y>maskRectangle.getBottom())  {
					
                    // and we're not already offscreen
                    if(!offScreen) {
						offScreen = true;
						// if we already have points then add an end point
                        // for the shape that is on the edge of the mask
						if(k>0) {
                            
							Point lastpoint = p;
							
							// TODO better point on edge rather than just clamp
							lastpoint.x = ofClamp(lastpoint.x, maskRectangle.getLeft(), maskRectangle.getRight());
							lastpoint.y = ofClamp(lastpoint.y, maskRectangle.getTop(), maskRectangle.getBottom());
							segmentPoints.push_back(lastpoint);
							
							// add this bunch to the collection for this zone
							zonePointsForShapes.push_back(segmentPoints); // should copy
							
							//clear the vector and start again
							segmentPoints.clear();
							
						}
					}
                    // otherwise if we are already off screen we don't need
                    // to do anything except ignore this point
				
                // else if we are inside the mask rectangle
				} else {
                    
					// and we're currently off screen
					if(offScreen) {
						// clear the points - do we need to if we
                        // cleared them already at the end of the last one?
						segmentPoints.clear();
						offScreen = false;
                        // if we have points already
						if(k>0) {
                            
                            // then figure out the position on the edge
                            // of the mask
							Point lastpoint = shapePointBuffer[k-1];
							
							// TODO better point on edge rather than just clamp
							lastpoint.x = ofClamp(lastpoint.x, maskRectangle.getLeft(), maskRectangle.getRight());
							lastpoint.y = ofClamp(lastpoint.y, maskRectangle.getTop(), maskRectangle.getBottom());
                            
                            // and add it to the beginning of the new segment
                            segmentPoints.push_back(lastpoint);
						}
					}
                    
                    // either way, add this next point because we are on screen
					segmentPoints.push_back(p);
				}
				
			
				
			}
			// add the final segment points to zone points
			if(segmentPoints.size()>0) {
				zonePointsForShapes.push_back(segmentPoints);
			}
			
		} // end zoneshapes
		
		
		// go through all the points and warp them into projector space
		for(size_t j = 0; j<zonePointsForShapes.size(); j++) {
			PointsForShape& segmentpoints = zonePointsForShapes[j];
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
                Point& p = segmentpoints[k];
                p = warp.getWarpedPoint(p);
                
                // check if it's in any of the masks!
                for(QuadMask* mask : maskManager.quads){
                    if(mask->hitTest(p)) {
                        p.multiplyColour(ofMap(mask->maskLevel,100,0,0,1));
                    }
                }
                
			}
		}
		
		
		// add all the segments for the zone into the big container for all the segs
		allzoneshapepoints.insert(allzoneshapepoints.end(), zonePointsForShapes.begin(), zonePointsForShapes.end());
		
		// delete all the test pattern shapes
		for(size_t j = 0; j<testPatternShapes.size(); j++) {
			delete testPatternShapes[j];
		}

		testPatternShapes.clear();
		
	} // end zones
	
	
	
	
}



RenderProfile& Projector::getRenderProfile(string profilelabel) {
	
		if(scannerSettings.renderProfiles.count(profilelabel) == 0) {
			// if we don't have a profile with that name then
			// something has seriously gone wrong
			profilelabel = OFXLASER_PROFILE_DEFAULT;
		}
		return scannerSettings.renderProfiles.at(profilelabel);
	
}

deque<Shape*> Projector ::getTestPatternShapesForZone(ProjectorZone& projectorZone) {
	
	deque<Shape*> shapes;
    new Line(ofPoint(0,0), ofPoint(100,0),ofColor::red, OFXLASER_PROFILE_FAST);
	Zone& zone = projectorZone.zone;

	ofRectangle& maskRectangle = projectorZone.zoneMask;

	if(testPattern==1) {

		ofRectangle& rect = maskRectangle;

		ofColor col = ofColor(0,255,0);
		shapes.push_back(new Line(rect.getTopLeft(), rect.getTopRight(), col, OFXLASER_PROFILE_FAST));
		shapes.push_back(new Line(rect.getTopRight(), rect.getBottomRight(), col, OFXLASER_PROFILE_FAST));
		shapes.push_back(new Line(rect.getBottomRight(), rect.getBottomLeft(), col, OFXLASER_PROFILE_FAST));
		shapes.push_back(new Line(rect.getBottomLeft(), rect.getTopLeft(), col, OFXLASER_PROFILE_FAST));
		shapes.push_back(new Line(rect.getTopLeft(), rect.getBottomRight(), col, OFXLASER_PROFILE_FAST));
		shapes.push_back(new Line(rect.getTopRight(), rect.getBottomLeft(), col, OFXLASER_PROFILE_FAST));


	} else if(testPattern==2) {

		ofRectangle& rect = zone.rect;

		ofPoint v = rect.getBottomRight() - rect.getTopLeft()-ofPoint(0.2,0.2);
		for(float y = 0; y<=1.1; y+=0.333333333) {

			shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1, rect.getTop()+0.1+v.y*y),ofPoint(rect.getRight()-0.1, rect.getTop()+0.1+v.y*y), ofColor(255), OFXLASER_PROFILE_FAST));
		}

		for(float x =0 ; x<=1.1; x+=0.3333333333) {


			shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1+ v.x*x, rect.getTop()+0.1),ofPoint(rect.getLeft()+0.1 + v.x*x, rect.getBottom()-0.1), ofColor(255,0,0), OFXLASER_PROFILE_FAST ));

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
					c.r *= colourSettings.red100;
					c.g *= colourSettings.green100;
					c.b *= colourSettings.blue100;
					break;
				case 1 :
					c.r *= colourSettings.red75;
					c.g *= colourSettings.green75;
					c.b *= colourSettings.blue75;
					break;
				case 2 :
					c.r *= colourSettings.red50;
					c.g *= colourSettings.green50;
					c.b *= colourSettings.blue50;
					break;
				case 3 :
					c.r *= colourSettings.red25;
					c.g *= colourSettings.green25;
					c.b *= colourSettings.blue25;
					break;
				case 4 :
					c.r *= colourSettings.red0;
					c.g *= colourSettings.green0;
					c.b *= colourSettings.blue0;
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


void Projector :: addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint){

	ofPoint target = targetpoint;
	ofPoint start = currentPosition;

	ofPoint v = target-start;

	float blanknum = (v.length()/scannerSettings.moveSpeed)/speedMultiplier;// + movePointsPadding;

	for(int j = 0; j<blanknum; j++) {

		float t = Quint::easeInOut((float)j, 0.0f, 1.0f, blanknum);

		ofPoint c = (v* t) + start;
		addPoint(c, (laserOnWhileMoving && j%2==0) ? ofColor(200,0,0) : ofColor(0));

	}

}

void Projector :: addPoint(ofPoint p, ofFloatColor c, bool useCalibration) {
	
	
	addPoint(ofxLaser::Point(p, c, useCalibration));
	
}
void Projector :: addPoints(vector<ofxLaser::Point>&points, bool reversed) {
	if(!reversed) {
		for(size_t i = 0; i<points.size();i++) {
			addPoint(points[i]);
		}
	} else {
		for(int i=(int)points.size()-1;i>=0; i--) {
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
			
	// Lasers usually change colour sooner than the mirrors can move to the next
	// position, so the colourChangeOffset system
	// mitigates against that by shifting the colours for the points.
	// Some optimisation makes this slightly hard to read but we iterate through
	// the points backwards, and store the .
	// To avoid creating a whole new vector, we're storing the overlap in a buffer of
	// colours.
	// I'm sure there must be some slicker C++ way of doing this...
	
	frameCounter++;
	
	if(offsetColours) {
        // the offset value is in time, so we convert it to a number of points.
		// this way we can change the PPS and this should still work
        // TODO do we need to take into account the speed multiplier?
		int colourChangeIndexOffset = (float)pps/10000.0f*colourChangeShift ;
		
		// we switch the front and rear buffers every frame, so we copy the
		// rear points to the front
		vector<Point>& frontBuffer = (frameCounter%2==0) ? sparePoints : sparePoints2;
		vector<Point>& rearBuffer = (frameCounter%2==0) ? sparePoints2 : sparePoints;

		// we change the colour later, ie we shift the colours forward (later)
		
		// resize the spare points to match the number we need
		if(rearBuffer.size()!= colourChangeIndexOffset) {
			// should be blank point but might wanna give it a
			// default value with the current position
			rearBuffer.resize(colourChangeIndexOffset);
			// maybe do some other stuff as well for safety
		}
		if(frontBuffer.size()!= colourChangeIndexOffset) {
			frontBuffer.resize(colourChangeIndexOffset);
		}
		
		for(int i = (int)(laserPoints.size()+rearBuffer.size())-1; i>=0; i--) {
			
			Point& p = (i<laserPoints.size()) ? laserPoints[i] : rearBuffer[i-laserPoints.size()];

			//now shift the colour from an earlier point
			if(i>=colourChangeIndexOffset){
				p.copyColourFromPoint(laserPoints[i-colourChangeIndexOffset]);
			} else {
				p.copyColourFromPoint(frontBuffer[i-colourChangeIndexOffset+frontBuffer.size()]);
			}
		}
	}

	
	for(size_t i = 0; i<laserPoints.size(); i++) {
		
		ofxLaser::Point &p = laserPoints[i];
    
		
		if(flipY) p.y= 800-p.y;
		if(flipX) p.x= 800-p.x;
		if(rotation!=0) {
			p.x-=400;
			p.y-=400;

			glm::vec3 vec = glm::vec3(p.x,p.y,0);
			float angle = ofDegToRad(rotation);
			
			glm::vec2 rotatedVec = glm::rotate(vec, angle, glm::vec3(0.0f, 0.0f, 1.0f));
			p.x=rotatedVec.x+400;
			p.y=rotatedVec.y+400;

		}
		
		// bounds check
        if(p.x<0) {
            p.x = p.r = p.g = p.b = 0;
        } else if(p.x>800) {
			p.x = 800;
			p.r = p.g = p.b = 0;
		}
        if(p.y<0) {
            p.y = p.r = p.g = p.b = 0;
        } else if(p.y>800) {
			p.y = 800;
			p.r = p.g = p.b = 0;
		}
		
		if(p.useCalibration) {
            colourSettings.processColour(p, intensity*masterIntensity);
        } else {
            
            
        }
		
		if(!armed) {
			p.r = 0;
			p.g = 0;
			p.b = 0;
		}
	
		
	}
	
}





bool Projector::loadSettings(vector<Zone*>& zones){
    ofJson json = ofLoadJson("projectors/projector" + ofToString(projectorIndex)+".json");
    ofDeserialize(json, params);
    

    bool success = maskManager.deserialize(json);
    
    //cout << json.dump(3) << endl;
    
    //vector<int>projectorzonenums = json["projectorzones"];
    ofJson zoneNumJson = json["projectorzones"];
    //cout << zoneNumJson.dump(3) << endl;
    
    // if the json node isn't found then this should do nothing
    for(auto jsonitem : zoneNumJson) {
        cout << (int) jsonitem << endl;
        int zoneNum = (int)jsonitem;
        ProjectorZone* projectorZone = new ProjectorZone(*zones[zoneNum]);
        projectorZones.push_back(projectorZone);
        ofJson projectorZoneJson = ofLoadJson("projectors/projector"+ ofToString(projectorIndex) +"zone" + ofToString(zoneNum) + ".json");

        success &= projectorZone->deserialize(projectorZoneJson);
        
    }

    if(json.empty() || (!success)) {
        return false;
    } else {
        return true;
    }
}


bool Projector::saveSettings(){
    // update the projector index if necessary
    params.setName(ofToString(projectorIndex));
    
    ofJson json;
    ofSerialize(json, params);
    
    //scannerSettings.serialize(json);

    vector<int>projectorzonenums;
    for(ProjectorZone* projectorZone : projectorZones) {
        projectorzonenums.push_back(projectorZone->getZoneIndex());
    }
    
    json["projectorzones"] = projectorzonenums;

    maskManager.serialize(json);
    //cout << json.dump(3) << endl;
    bool success = ofSavePrettyJson("projectors/projector"+ ofToString(projectorIndex) +".json", json);

    
    for(ProjectorZone* projectorZone : projectorZones) {
        ofJson projectorzonejson;
        projectorZone->serialize(projectorzonejson);
        
        success &= ofSavePrettyJson("projectors/projector"+ ofToString(projectorIndex) +"zone" + ofToString(projectorZone->getZoneIndex()) + ".json", projectorzonejson);
    }
    
    lastSaveTime = ofGetElapsedTimef(); 
    return success;
    
}


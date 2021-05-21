//
//  ofxLaserLaser.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserLaser.h"

using namespace ofxLaser;

Laser::Laser(int _index) {
    laserIndex = _index;
    dac = &emptyDac;
    
	laserHomePosition = ofPoint(400,400);
	
	numTestPatterns = 9;
 	
	guiInitialised = false;
    maskManager.init(800,800);
   
    
	
};

Laser::~Laser() {
    
    // NOTE that the manager saves the laser settings when it closes
	ofLog(OF_LOG_NOTICE, "ofxLaser::Laser destructor called");
	pps.removeListener(this, &Laser::ppsChanged);
	armed.removeListener(this, &ofxLaser::Laser::setDacArmed);
    ofRemoveListener(params.parameterChangedE(), this, &Laser::paramsChanged);
   
	if(dac!=nullptr) dac->close();
	//delete gui;
}

void Laser::setDac(DacBase* newdac){
    if(dac!=newdac) {
        dac = newdac;
        newdac->setPointsPerSecond(pps);
        dacId = dac->getId();
        armed = false; // automatically calls setArmed because of listener on parameter
    }
    
}
DacBase* Laser::getDac(){
    return dac;
    
}

bool Laser::hasDac() {
    return (dac != &emptyDac);
    
}
bool Laser::removeDac(){
	if (dac != &emptyDac) {
		dac = &emptyDac;
		dacId = "";
		return true;
	}
	else {
		return false;
	}
}

int Laser::getPointRate() {
    return pps;
};
float Laser::getFrameRate() {
    if(numPoints>0) return (float)pps/(float)numPoints;
    else return pps;
}

void Laser::setDefaultHandleSize(float size) {
	
	defaultHandleSize = size;
	//for(ZoneTransform* zonetrans : zoneTransforms) {
	//	zonetrans->setHandleSize(defaultHandleSize);
	//}
	
}

void Laser :: init() {

    // TODO is this used for anything other than display?
	params.setName(ofToString(laserIndex));
	
//    params.add(armed.set("ARMED", false));
    params.add(intensity.set("Brightness", 1,0,1));
//    params.add(testPattern.set("Test Pattern", 0,0,numTestPatterns));
    armed.set("ARMED", false);
    testPattern.set("Test Pattern", 0,0,numTestPatterns);
    
    params.add(dacId.set("dacId", ""));
    
    hideContentDuringTestPattern.set("Test pattern only", true);
	ofParameterGroup laserparams;
	laserparams.setName("Laser settings");
	
    laserparams.add(speedMultiplier.set("Speed", 1,0.12,2));
    
	 
	laserparams.add(colourChangeShift.set("Colour shift", 2,0,6));
		
	laserparams.add(flipX.set("Flip Horizontal", false));
	laserparams.add(flipY.set("Flip Vertical",false));
	laserparams.add(outputOffset.set("Output position offset", glm::vec2(0,0), glm::vec2(-20,-20),glm::vec2(20,20)));
	laserparams.add(rotation.set("Output rotation",0,-90,90));

	
	ofParameterGroup& advanced = advancedParams;
    advanced.setName("Advanced");
    laserparams.add(pps.set("Points per second", 30000,1000,80000));
    advanced.add(laserOnWhileMoving.set("Laser on while moving", false));
	advanced.add(smoothHomePosition.set("Smooth home position", true));
	advanced.add(sortShapes.set("Optimise shape draw order", true));
	advanced.add(targetFramerate.set("Target framerate", 25, 23, 120));
	advanced.add(syncToTargetFramerate.set("Sync to Target framerate", false));
	advanced.add(syncShift.set("Sync shift", 0, -50, 50));

	laserparams.add(advanced);
	
	params.add(laserparams);
    params.add(scannerSettings.params);
	
	ofParameterGroup renderparams;
	renderparams.setName("Render profiles");
	
	   
	
	
	params.add(colourSettings.params);

     
     
    armed.addListener(this, &ofxLaser::Laser::setDacArmed);
    pps.addListener(this, &Laser::ppsChanged);
  

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
    
    ofAddListener(params.parameterChangedE(), this, &Laser::paramsChanged);
   
	guiInitialised = true;

    //masks.resize(5); 
	
}





void Laser ::setDacArmed(bool& _armed){
    dac->setArmed(_armed);
    
}

bool Laser ::toggleArmed() {
    armed = !armed;
    return armed;
}

void Laser:: ppsChanged(int& e){
	ofLog(OF_LOG_NOTICE, "ppsChanged"+ofToString(pps));
	pps=round(pps/100)*100;
	if(pps<=100) pps =100;
	dac->setPointsPerSecond(pps);
}


void Laser::addZone(Zone* zone, float srcwidth, float srcheight) {

	if(hasZone(zone)) {
		ofLog(OF_LOG_ERROR, "Laser::addZone(...) - Laser already contains zone");
		return;
	}
    
    LaserZone* laserZone = new LaserZone(*zone);
    laserZones.push_back(laserZone);
    
    // initialise zoneTransform
    laserZone->zoneTransform.init(zone->rect);

    laserZone->zoneMask = zone->rect;
    
    // sort the zones... oh a fancy lambda check me out
    std::sort(laserZones.begin(), laserZones.end(), [](const LaserZone* a, const LaserZone* b) -> bool {
        return (a->getZoneIndex()<b->getZoneIndex());
    });
    saveSettings();
     
}

bool Laser :: hasZone(Zone* zone){
    for(LaserZone* laserZone : laserZones) {
        if(zone == &laserZone->zone) return true;
    }
    return false;
}

bool Laser :: removeZone(Zone* zone){

    LaserZone* laserZone = getLaserZoneForZone(zone);
    if(laserZone==nullptr) return false;
    
    vector<LaserZone*>::iterator it = std::find(laserZones.begin(), laserZones.end(), laserZone);

    // TODO Check cleanup
    laserZones.erase(it);
    delete laserZone;
    
    saveSettings();
    
    return true;
    
    
    
}
LaserZone* Laser::getLaserZoneForZone(Zone* zone) {
    for(LaserZone* laserZone : laserZones) {
        if(&laserZone->zone == zone) return laserZone;
    }
    return nullptr;

}

void Laser::updateZoneMasks() {
	
    for(LaserZone* laserZone : laserZones) {
        
        laserZone->updateZoneMask();
    }
}

string Laser :: getLabel() {
    return "Laser " + ofToString(laserIndex+1);
}

string Laser::getDacLabel() {
    if(dac!=&emptyDac) {
        return dac->getId();
    } else {
        return "No laser controller assigned";
    }
}

int Laser::getDacConnectedState() {
    
    if(dac!=nullptr) {
        return dac->getStatus();
    } else {
        return OFXLASER_DACSTATUS_ERROR;
    }
}

void Laser::drawTransformUI(float x, float y, float w, float h) {
	
	ofPushStyle();
	ofNoFill();

    float scale = w/800.0f;
    ofPoint offset = ofPoint(x,y) + (ofPoint(outputOffset)*scale);
    for(LaserZone* laserZone : laserZones) {
        //if(!laserZone->getEnabled()) continue;
        laserZone->setScale(scale);
        laserZone->setOffset(offset);
        laserZone->draw();
        
        
    }
    maskManager.setOffsetAndScale(offset,scale);
    maskManager.draw();
   
    ofPopStyle();
}   

void Laser::drawTransformAndPath(ofRectangle rect) {
    ofRectangle bounds;
    vector<glm::vec3> perimeterpoints;
    bool firsttime = true;
    for(LaserZone* zone : laserZones) {
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
    ofSetColor(255);
    ofDrawBitmapString(ofToString(laserIndex+1), rect.getRight()-20, rect.getTop()+20);

    ofPushMatrix();
    ofTranslate(rect.x, rect.y);
    float rectscale = rect.width/800;
    ofScale(rect.width/800, rect.height/800);
    ofPushStyle();
    ofNoFill();
    ofSetColor(50,50,200);
   
    float scale = 800.0f/bounds.width;
    if(800/bounds.height<scale) scale = 800/bounds.height;
    if(scale<1.1) {
        scale = 1;
        bounds.set(0,0,800,800);
    } else {
        // draw output scale
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(700,700);
        ofScale(0.1,0.1);
        ofFill();
        ofSetColor(0);
        ofDrawRectangle(0,0,800,800);
        ofNoFill();
        ofSetColor(50,50,200);
        ofDrawRectangle(0,0,800,800);
        for(LaserZone* zone : laserZones) {

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
    ofScale(scale, scale);
    ofTranslate(-bounds.x, -bounds.y); //getTopLeft());
    drawLaserPath(ofRectangle(0,0,800,800), false, 4/(scale*rectscale));
   
   // ofDrawRectangle(bounds);
    for(LaserZone* zone : laserZones) {

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


void Laser :: drawLaserPath(ofRectangle rect, bool drawDots, float radius) {
	drawLaserPath(rect.x, rect.y, rect.width, rect.height, drawDots, radius);
}
void Laser :: drawLaserPath(float x, float y, float w, float h, bool drawDots, float radius) {
	ofPushStyle();
	
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
            ofDrawCircle(p, radius);
        } else {
            ofFill();
            c.setBrightness(255);
            ofSetColor(c);
            ofDrawCircle(p, radius);
            ofNoFill();
            ofSetLineWidth(2); 
            c.setBrightness(128);
            ofSetColor(c);
            ofDrawCircle(p, radius*1.5);
        }
	}
	

	ofDisableBlendMode();
	ofPopStyle();
	
	
	ofPopMatrix();
	
}

void Laser :: disableTransformGui() {
	
    for(LaserZone* laserZone : laserZones) {
        laserZone->setEnabled(false);
    }
	
	
}
void Laser :: enableTransformGui() {
    for(LaserZone* laserZone : laserZones) {
        if(laserZone->getVisible()) laserZone->setEnabled(true);
    }
	
}




void Laser::update(bool updateZones) {
	
    bool soloMode = false;
    bool needsSave = false;
    for(LaserZone* laserZone : laserZones) {
        
        if(laserZone->soloed) {
            soloMode = true;
            break;
        }
    }
    
    if(soloMode) {
        for(LaserZone* laserZone : laserZones) {
            laserZone->setVisible(laserZone->soloed);
        }
        
    } else {
        for(LaserZone* laserZone : laserZones) {
            laserZone->setVisible(!laserZone->muted);
        }
    }
    
    
    // if any of the source rectangles have changed then update all the warps
    // (shouldn't need anything saving)
    if(updateZones) {
        for(LaserZone* laserZone : laserZones) {
            ZoneTransform& warp = laserZone->zoneTransform;
            warp.setSrc(laserZone->zone.rect);
            warp.updateHomography();
            updateZoneMasks();
        }
    }
    
    needsSave = maskManager.update() | needsSave;
    
    
	laserPoints.clear();
	previewPathMesh.clear();
    bool laserZoneChanged = false;
    for(LaserZone* laserZone : laserZones) {
        laserZoneChanged |= laserZone->update();
	}
	
    needsSave |= laserZoneChanged;
    float framerate = getFrameRate();
	smoothedFrameRate += (framerate - smoothedFrameRate)*0.2;
    frameTimeHistory[frameTimeHistoryOffset] = 1/framerate;
    frameTimeHistoryOffset++;
    if(frameTimeHistoryOffset>=frameTimeHistorySize) frameTimeHistoryOffset = 0;
    
    if(needsSave) saveSettings();
 
}


void Laser::sendRawPoints(const vector<ofxLaser::Point>& points, Zone* zone, float masterIntensity ){
    
    LaserZone& laserZone = *getLaserZoneForZone(zone);
    ofRectangle& maskRectangle = laserZone.zoneMask;
    ZoneTransform& warp = laserZone.zoneTransform;
    bool offScreen = true;
    
    vector<Point>segmentpoints;
    
    //iterate through the points
    for(size_t k = 0; k<points.size(); k++) {
        
        Point p = points[k];
        
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


    
    // go through all the points and warp them into output space

    for(size_t k= 0; k<segmentpoints.size(); k++) {
        addPoint(warp.getWarpedPoint(segmentpoints[k]));
    }
    
    
    
    processPoints(masterIntensity, false);
    dac->sendPoints(laserPoints);
    
   
}


                        

void Laser::send(ofPixels* pixels, float masterIntensity) {


	if(!guiInitialised) {
		ofLog(OF_LOG_ERROR, "Error, ofxLaser::laser not initialised yet. (Probably missing a ofxLaser::Manager.initGui() call...");
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
		
		ofPoint currentPosition = laserHomePosition; // MUST be in output space
		
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


void Laser ::getAllShapePoints(vector<PointsForShape>* shapepointscontainer, ofPixels*pixels, float speedmultiplier){
	
	vector<PointsForShape>& allzoneshapepoints = *shapepointscontainer;
	
	// temp vectors for storing the shapes in
	vector<PointsForShape> zonePointsForShapes;
	vector<Point> shapePointBuffer;
	
	// go through each zone
	//for(int i = 0; i<(int)laserZones.size(); i++) {
    for(LaserZone* laserZone : laserZones) {
      
        if(!laserZone->getVisible()) continue;
        
		Zone& zone = laserZone->zone;
        ZoneTransform& warp = laserZone->zoneTransform;
		ofRectangle& maskRectangle = laserZone->zoneMask;
        
        // doesn't make a copy, just a pointer to the original shapes in the zone
        // CHECK - is this OK ?
		deque<Shape*>* zoneshapes = &zone.shapes;
		
        // get test pattern shapes - we have to do this even if
        // we don't have a test pattern, so that the code at the end
        // of this function can delete the shapes.
        deque<Shape*> testPatternShapes = getTestPatternShapesForZone(*laserZone);
        
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
		
		
		// go through all the points and warp them into output space
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



RenderProfile& Laser::getRenderProfile(string profilelabel) {
	
		if(scannerSettings.renderProfiles.count(profilelabel) == 0) {
			// if we don't have a profile with that name then
			// something has seriously gone wrong
			profilelabel = OFXLASER_PROFILE_DEFAULT;
		}
		return scannerSettings.renderProfiles.at(profilelabel);
	
}

deque<Shape*> Laser ::getTestPatternShapesForZone(LaserZone& laserZone) {
	
	deque<Shape*> shapes;
    new Line(ofPoint(0,0), ofPoint(100,0),ofColor::red, OFXLASER_PROFILE_FAST);
	Zone& zone = laserZone.zone;

	ofRectangle& maskRectangle = laserZone.zoneMask;

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


void Laser :: addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint){

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

void Laser :: addPoint(ofPoint p, ofFloatColor c, bool useCalibration) {
	
	
	addPoint(ofxLaser::Point(p, c, useCalibration));
	
}
void Laser :: addPoints(vector<ofxLaser::Point>&points, bool reversed) {
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

void Laser :: addPoint(ofxLaser::Point p) {
	
	p+=(ofPoint)outputOffset;
	
	laserPoints.push_back(p);
	
	previewPathMesh.addVertex(ofPoint(p.x, p.y));

}



void  Laser :: processPoints(float masterIntensity, bool offsetColours) {
			
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


void Laser::paramsChanged(ofAbstractParameter& e){
    saveSettings();
}


bool Laser::loadSettings(vector<Zone*>& zones){

    ofJson json = ofLoadJson(savePath + "laser"+ ofToString(laserIndex)+".json");
    ofDeserialize(json, params);
    

    bool success = maskManager.deserialize(json);
    
    ofJson zoneNumJson = json["laserzones"];
    
    // if the json node isn't found then this should do nothing
    for(auto jsonitem : zoneNumJson) {
        cout << (int) jsonitem << endl;
        int zoneNum = (int)jsonitem;
        LaserZone* laserZone = new LaserZone(*zones[zoneNum]);
        laserZones.push_back(laserZone);
        ofJson laserZoneJson = ofLoadJson(savePath + "laser"+ ofToString(laserIndex) +"zone" + ofToString(zoneNum) + ".json");

        success &= laserZone->deserialize(laserZoneJson);
        
    }

    if(json.empty() || (!success)) {
        return false;
    } else {
        return true;
    }
}


bool Laser::saveSettings(){
    // update the laser index if necessary
    params.setName(ofToString(laserIndex));
    
    ofJson json;
    ofSerialize(json, params);
    
    //scannerSettings.serialize(json);

    vector<int>laserzonenums;
    for(LaserZone* laserZone : laserZones) {
        laserzonenums.push_back(laserZone->getZoneIndex());
    }
    
    json["laserzones"] = laserzonenums;

    maskManager.serialize(json);
    //cout << json.dump(3) << endl;
    bool success = ofSavePrettyJson(savePath + "laser"+ ofToString(laserIndex) +".json", json);

    
    for(LaserZone* laserZone : laserZones) {
        ofJson laserzonejson;
        laserZone->serialize(laserzonejson);
        
        success &= ofSavePrettyJson(savePath + "laser"+ ofToString(laserIndex) +"zone" + ofToString(laserZone->getZoneIndex()) + ".json", laserzonejson);
    }
    
    lastSaveTime = ofGetElapsedTimef(); 
    return success;
    
}

bool Laser :: getSaveStatus(){
    return (ofGetElapsedTimef()-lastSaveTime<1);
}

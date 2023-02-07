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
	
	numTestPatterns = 10;
 	
	guiInitialised = false;
    maskManager.init(800,800);
   
    pauseStateRecorded = false;
    
    lastSaveTime = 0;
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
        newdac->setColourShift(colourChangeShift); 
        newdac->maxLatencyMS = maxLatencyMS;
        dacLabel = dac->getId();
       // dacAlias = dac->getAlias();
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
		dacLabel = "";
        //dacAlias = "";
		return true;
	}
	else {
		return false;
	}
}

int Laser::getPointRate() {
    return pps;
}

float Laser::getFrameRate() {
    if(numPoints>1) return (float)pps/(float)numPoints;
    else return INFINITY;
}

void Laser :: init() {

    // TODO is this used for anything other than display?
	params.setName(ofToString(laserIndex));
	
//    params.add(armed.set("ARMED", false));
    params.add(intensity.set("Brightness", 1,0,1));
//    params.add(testPattern.set("Test Pattern", 0,0,numTestPatterns));
    armed.set("ARM", false);
    testPattern.set("Test Pattern", 0,0,numTestPatterns);
    
    paused.set("Paused", false);
    
    params.add(dacLabel.set("dacId", ""));
    //params.add(dacAlias.set("dacAlias", ""));
    
    params.add(useAlternate.set("Use alternate zones", false));
    params.add(muteOnAlternate.set("Mute instead of alternates", false));

    hideContentDuringTestPattern.set("Test pattern only", true);
	ofParameterGroup laserparams;
	laserparams.setName("Laser settings");
	
    laserparams.add(speedMultiplier.set("Speed", 1,0.12,2));
    
    laserparams.add(colourChangeShift.set("Colour shift", 2,0,12));
    
    //laserparams.add(maxLatencyMS.set("Frame latency", 100,5,300));
    maxLatencyMS = 150;
    
	laserparams.add(flipX.set("Flip Horizontal", false));
    laserparams.add(flipY.set("Flip Vertical",false));
    laserparams.add(mountOrientation.set("Orientation",0,0,3));
    laserparams.add(outputOffset.set("Output position offset", glm::vec2(0,0), glm::vec2(-20,-20),glm::vec2(20,20)));
	laserparams.add(rotation.set("Output rotation",0,-90,90));

	ofParameterGroup& advanced = advancedParams;
    advanced.setName("Advanced");
    laserparams.add(pps.set("Points per second", 30000,1000,80000));
    advanced.add(laserOnWhileMoving.set("Laser on while moving", false));
	advanced.add(smoothHomePosition.set("Smooth home position", true));
    advanced.add(sortShapes.set("Optimise shape draw order", true));
    advanced.add(newShapeSortMethod.set("Experimental shape sorting", true));
    //advanced.add(alwaysClockwise.set("Always clockwise sorting", true));
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
    colourChangeShift.addListener(this, &Laser::colourShiftChanged);

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
    //ofLog(OF_LOG_NOTICE, "ppsChanged"+ofToString(pps));
    pps=round(pps/100)*100;
    if(pps<=100) pps =100;
    dac->setPointsPerSecond(pps);
}
void Laser:: colourShiftChanged(float& e){
    //ofLog(OF_LOG_NOTICE, "ppsChanged"+ofToString(pps));
    //pps=round(pps/100)*100;
    //if(pps<=100) pps =100;
    dac->setColourShift(e);
}


void Laser::addZone(InputZone* inputzone, bool isAlternate) {

	if(hasZone(inputzone) && !isAlternate) {
		ofLog(OF_LOG_ERROR, "Laser::addZone(...) - Laser already contains zone");
		return;
	}
    if(hasAltZone(inputzone) && isAlternate) {
        ofLog(OF_LOG_ERROR, "Laser::addZone(...) - Laser already contains alt zone");
        return;
    }
    if((!hasZone(inputzone)) && isAlternate) {
        ofLog(OF_LOG_ERROR, "Laser :: addZone(...) can only add alt if laser has zone already");
    }
    
    OutputZone* outputzone = new OutputZone(*inputzone);

   //outputzone->setGrid(snapToGrid, gridSize);
    outputZones.push_back(outputzone);
    
    ofJson laserZoneJson;
    string filename = savePath + "laser"+ ofToString(laserIndex) +"zone" + ofToString(inputzone->getIndex()) + (isAlternate?"alt.json" : ".json");
                           
    if(ofFile(filename).exists()) {
        laserZoneJson = ofLoadJson(filename);
    }

    
    if(!laserZoneJson.empty()) {
        outputzone->deserialize(laserZoneJson);
    } else {
        // initialise zoneTransform
        outputzone->init(inputzone->getRect());
        //outputzone->zoneMask = inputzone->rect;
    }
    outputzone->setIsAlternate(isAlternate);
    // sort the zones... oh a fancy lambda check me out
    std::sort(outputZones.begin(), outputZones.end(), [](const OutputZone* a, const OutputZone* b) -> bool {
        return (a->getZoneIndex()<b->getZoneIndex());
    });
    saveSettings();
     
}


void Laser::addAltZone(InputZone* zone) {
    addZone(zone, true);
}



bool Laser :: hasZone(InputZone* zone){
    
    for(OutputZone* laserZone : outputZones) {
        if((!laserZone->getIsAlternate()) && (zone == &laserZone->zone)) return true;
    }
    return false;
}

bool Laser :: hasAltZone(InputZone* zone){
    
    for(OutputZone* laserZone : outputZones) {
        if((laserZone->getIsAlternate()) && (zone == &laserZone->zone)) return true;
    }
    return false;
}
bool Laser :: hasAltZone(int zoneIndex){
    
    for(OutputZone* laserZone : outputZones) {
        if((laserZone->getIsAlternate()) && (zoneIndex == laserZone->getZoneIndex())) return true;
    }
    return false;
}

bool Laser :: hasAnyAltZones() {
    
    for(OutputZone* laserZone : outputZones) {
        if(laserZone->getIsAlternate()) return true;
    }
    return false;
}


bool Laser :: removeZone(InputZone* inputZone){

    OutputZone* outputZone = getLaserZoneForZone(inputZone);
    
    
    return removeZone(outputZone);
    
}

bool Laser :: removeZone(OutputZone* outputZone){

    if(outputZone==nullptr) return false;
    
    vector<OutputZone*>::iterator it = std::find(outputZones.begin(), outputZones.end(), outputZone);

    outputZones.erase(it);
    delete outputZone;
    
    
    saveSettings();
    
    return true;
    
}


bool Laser :: removeAltZone(InputZone* zone){
    return removeAltZone(getLaserAltZoneForZone(zone));

}

bool Laser :: removeAltZone(OutputZone* outputZone){

    vector<OutputZone*>::iterator it = std::find(outputZones.begin(), outputZones.end(), outputZone);
    if(it!=outputZones.end()) {
        outputZones.erase(it);
        delete outputZone;
    
        saveSettings();
    
        return true;
    } else {
        return false;
    }
    
}

bool Laser :: removeAltZone(int zoneIndex){

    for(OutputZone* outputZone : outputZones) {
        if(outputZone->getIsAlternate() && outputZone->getZoneIndex()==zoneIndex) {
            return removeAltZone(outputZone);
        }
        
    }
    return false;
    
}



OutputZone* Laser::getLaserZoneForZone(InputZone* zone) {
    for(OutputZone* laserZone : outputZones) {
        if((!laserZone->getIsAlternate()) && (&laserZone->zone == zone)) return laserZone;
    }
    return nullptr;
}

OutputZone* Laser::getLaserAltZoneForZone(InputZone* zone) {
    for(OutputZone* laserZone : outputZones) {
        if((laserZone->getIsAlternate()) && (&laserZone->zone == zone)) return laserZone;
    }
    return nullptr;
}

OutputZone* Laser::getLaserAltZoneForZone(int zoneIndex){
    for(OutputZone* laserZone : outputZones) {
        if((laserZone->getIsAlternate()) && (laserZone->getZoneIndex() == zoneIndex)) return laserZone;
    }
    return nullptr;
}

vector<OutputZone*> Laser::getActiveZones(){
    bool soloActive = areAnyZonesSoloed();
    vector<OutputZone*> activeZones;
    for(OutputZone* laserZone : outputZones) {
        
        if((!useAlternate) && laserZone->getIsAlternate()) continue;
        if(useAlternate && (!laserZone->getIsAlternate())) continue;
        
        if(laserZone->getIsAlternate()) continue;
        if(soloActive && laserZone->soloed) {
            activeZones.push_back(laserZone);
        } else if(!laserZone->muted) {
            activeZones.push_back(laserZone);
        }
    }
    return activeZones;
}


void Laser::clearOutputZones() {
    
    for(OutputZone* zone : outputZones) {
        delete zone;
    }
    outputZones.clear();
}

bool Laser::areAnyZonesSoloed() {
    for(OutputZone* laserZone : outputZones) {
        if(laserZone->soloed) {
            return true;
        }
    }
    
    return false;
}

bool Laser ::muteZone(int zonenum) {
    bool changed = false;
    for(OutputZone* laserZone : outputZones) {
        if(laserZone->getZoneIndex() == zonenum) {
            if(!laserZone->muted) {
                laserZone->muted = true;
                changed = true;
            } else {
                //return false;
            }
        }
    }
    return changed;
}
bool Laser ::unMuteZone(int zonenum){
    bool changed = false;
    for(OutputZone* laserZone : outputZones) {
        if(laserZone->getZoneIndex() == zonenum) {
            if(laserZone->muted) {
                laserZone->muted = false;
                changed = true;
            } else {
                //return false;
            } 
        }
    }
    return changed;
}

bool Laser ::soloZone(int zonenum) {
    bool changed = false;
    for(OutputZone* laserZone : outputZones) {
        if(laserZone->getZoneIndex() == zonenum) {
            if(!laserZone->soloed) {
                laserZone->soloed = true;
                changed = true;
                
            } else {
                
            }
        }
    }
    return changed;
}
bool Laser ::unSoloZone(int zonenum){
    bool changed = false; 
    for(OutputZone* laserZone : outputZones) {
        if(laserZone->getZoneIndex() == zonenum) {
            if(laserZone->soloed) {
                laserZone->soloed = false;
                changed = true; // return true;
            } else {
               // return false;
            }
        }
    }
    return changed;
}

string Laser :: getLabel() {
    return "Laser " + ofToString(laserIndex+1);
}

string Laser::getDacLabel() {
    if(dac!=&emptyDac) {
        return dac->getId();
    } else {
        return "";
    }
}


int Laser::getDacConnectedState() {
    
    if((dac!=nullptr)&&(dac!=&emptyDac)) {
        return dac->getStatus();
    } else {
        return OFXLASER_DACSTATUS_NO_DAC;
    }
}


void Laser::update(bool updateZones) {
//
//    if(previewDragging) {
//        previewOffset = glm::vec2(ofGetMouseX(), ofGetMouseY())-dragStartPoint;
//    }
//
    
   // bool soloMode = areAnyZonesSoloed();
    bool needsSave = false;
    
    
  
    
    
    // if any of the source rectangles have changed then update all the warps
    // (shouldn't need anything saving)
    if(updateZones) {
       
        for(OutputZone* laserZone : outputZones) {
            //ZoneTransform& warp = laserZone->zoneTransform;
            //laserZone->init(laserZone->zone.rect);
            //laserZone->updateHomography();
            
        }
        
        //updateZoneMasks();
    }
    

    needsSave = maskManager.update() | needsSave;
    
    bool laserZoneChanged = false;
    for(OutputZone* laserZone : outputZones) {
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


void Laser::sendRawPoints(const vector<ofxLaser::Point>& points, InputZone* zone, float masterIntensity ){
    
    clearPoints();
    
    OutputZone* laserZone = getLaserZoneForZone(zone);
    if(laserZone==nullptr) {
        ofLogError("Laser::sendRawPoints(...), zone "+zone->zoneLabel + " not added to laser ");
        return;
        
    }
    ofRectangle maskRectangle = laserZone->zone.getRect();
    //ZoneTransform& warp = laserZone->zoneTransform;
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
        addPoint(laserZone->getWarpedPoint(segmentpoints[k]));
    }
    
    
    
    processPoints(masterIntensity, false);
    dac->sendPoints(laserPoints);
    
   
}


void Laser :: clearPoints() {
    laserPoints.clear();
    previewPathMesh.clear();
    previewPathColoured.clear();
    
    
}

void Laser::send(float masterIntensity, ofPixels* pixelmask) {


	if(!guiInitialised) {
		ofLog(OF_LOG_ERROR, "Error, ofxLaser::laser not initialised yet. (Probably missing a ofxLaser::Manager.initGui() call...");
		return;
	}
    
    if(!dac->isReadyForFrame(maxLatencyMS)) {
        // register skipped frame
        return;
    }
    
    // PAUSE FUNCTION
    if(paused) {
        if(!pauseStateRecorded) {
            // record all zone shapes;
            pauseStateRecorded = true;
            
            for(OutputZone* laserZone : outputZones) {
                if(laserZone->getIsAlternate()) continue; // to ensure we don't get two sets of shapes
                InputZone& zone = laserZone->zone;
                deque<Shape*>& shapes = pauseShapesByZone[&zone];
                for(Shape* shape : zone.shapes) {
                    shapes.push_back(shape->clone());
                }
            }
            
        }
    } else if(pauseStateRecorded) {
        pauseStateRecorded = false;
        // delete all zone shapes
        for (auto const& element : pauseShapesByZone) {
            for(Shape* shape : element.second) {
                delete shape;
            }
        }
        pauseShapesByZone.clear();
        
    }
    
    clearPoints();
    
	vector<PointsForShape> allzoneshapepoints;

	// TODO add speed multiplier to getPointsForMove function
	getAllShapePoints(&allzoneshapepoints, pixelmask, speedMultiplier);
	
	vector<PointsForShape*> sortedshapes;
	
	// sort the point objects
	if(allzoneshapepoints.size()>0) {
		bool reversed = false;
		float shortestDistance = INFINITY;

        PointsForShape* currentShape = nullptr;
        PointsForShape* nextShape = nullptr;
        ofPoint position = laserHomePosition;

		if(sortShapes) {
            
            float moveDistanceForUnSortedShapes = getMoveDistanceForShapes(allzoneshapepoints);
            
			do {
                
                if(currentShape!=nullptr) {
                    // get the shape object at the current index
                    PointsForShape& shape1 = *currentShape; // allzoneshapes[currentIndex];
				
                    // set its tested flag to say we've checked it
                    shape1.tested = true;
                    // add it to the list
                    sortedshapes.push_back(&shape1);
                    // set its reversed flag - this is set during the
                    // previous iterative process to find the next shape
                    shape1.reversed = reversed;
                    
                    position = shape1.getEnd();
                    
                    // set the distance to infinity
                    shortestDistance = INFINITY;
                    // reset the next shape in case we don't find any
                    nextShape = nullptr;
                }
                    
				
                // go through all the shapes
				for(size_t i = 0; i<allzoneshapepoints.size(); i++) {
					
                    // get the shape at j
					PointsForShape& shape2 = allzoneshapepoints[i];
					// if it's the same shape as this one or we've already checked it skip this one
                    if((currentShape==&shape2) || (shape2.tested)) continue;
					
                    // check non-reversed first
					shape2.reversed = false;
					
                    // if the distance between our first shape and the second shape is
                    // the shortest we've found...
					if(position.squareDistance(shape2.getStart()) < shortestDistance) {
                        // set the new shortest distance...
						shortestDistance = position.squareDistance(shape2.getStart());
                        // set this as the next shape to check
						nextShape = &shape2;
                        // set reversed to be false (this is set the next time around
						reversed = false;
					}
					
                    // now do the same thing but with the next shape reversed
					if((shape2.reversable) && (position.squareDistance(shape2.getEnd()) < shortestDistance)) {
						shortestDistance = position.squareDistance(shape2.getEnd());
                        nextShape = &shape2;
						reversed = true;
					}
					
				}
                currentShape = nextShape;
				
			} while (currentShape!=nullptr);
            
            
            if(newShapeSortMethod) {
                
                //cout << " NEW SHAPE SORT START -------------- " <<sortedshapes.size()<<  endl;
             
                // reset the tested flags
                for (PointsForShape* shape : sortedshapes) shape->tested = false;
             
                // start at the end
                int currentIndex = sortedshapes.size()-1;
                
                while(currentIndex>1) { // don't think we need to do this process for 0 and 1
                
                    PointsForShape& shape = *sortedshapes[currentIndex];
                    if(shape.tested) {
                        currentIndex--;
                        continue;
                    }
                    // position to move this shape to
                    int targetIndex = currentIndex;
                    
                    // get the distance between this and its two neighbours
                    PointsForShape& neighbourAfter = *sortedshapes[(currentIndex+1) % sortedshapes.size()];
                    PointsForShape& neighbourBefore = *sortedshapes[currentIndex-1]; // should always be >0
                  
                    float distanceToBeat = neighbourAfter.getStart().distance(shape.getEnd()) + shape.getStart().distance(neighbourBefore.getEnd()) - neighbourBefore.getEnd().distance(neighbourAfter.getStart());
                    distanceToBeat *= 0.95; // so close calls do nothing
                    
                    // now iterate back to the first shape
                    for(int i = currentIndex-1; i>0; i--) { // don't think we need to go all the way back to 0
                        // check the distance if we were to insert the shape between i and i-1
                        int shapeIndexBefore = (i==0) ? sortedshapes.size()-1 : i-1 ;
                        int shapeIndexAfter = i;
                        
                        PointsForShape& shapeBefore = *sortedshapes[shapeIndexBefore];
                        PointsForShape& shapeAfter = *sortedshapes[shapeIndexAfter];
                        
                        float distanceToCompare = shapeBefore.getEnd().distance(shape.getStart()) + shape.getEnd().distance(shapeAfter.getStart()) -                            shapeBefore.getEnd().distance(shapeAfter.getStart());
                        
                       // if((shape.getStart()!=shape.getEnd()) && (shapeBefore.getEnd().squareDistance(shapeAfter.getStart()) < 1)) continue; // if the shapes are connected don't insert a new one here unless it starts and ends at the same place
                       
                        if(distanceToCompare<distanceToBeat) {
                            // set target position of this shape to be i
                            targetIndex = i;
                            distanceToBeat = distanceToCompare; 
                        }
                    
                    }
                    
                    shape.tested = true;
                    // if the target position != currentIndex then move it there
                    if(targetIndex!=currentIndex) {
                        sortedshapes.erase(sortedshapes.begin() + currentIndex);
                        //if(targetIndex == 0 ) targetIndex = sortedshapes.size();
                        sortedshapes.insert(sortedshapes.begin() +targetIndex, &shape);
                        //ofLogNotice("moving shape at ") << currentIndex << " to " << targetIndex;
                    } else {
                        // else subtract 1 from the currentIndex
                        currentIndex--;
                    }
                }
                
                //cout << "----------------------------------- " << endl;

                
                
            }
            
          //  if(alwaysClockwise) {
                
                // TODO this algorithm doesn't seem to work right now :/
//
//                // CHECK HANDEDNESS
//                float sum = 0;
//                ofPoint p1 = sortedshapes[0]->getStart();
//                ofPoint p2 = p1;
//                for(size_t i = 0; i< sortedshapes.size(); i++) {
//
//                    PointsForShape& shape = *sortedshapes[i];
//                    p1 = p2;
//                    p2 = shape.getStart();
//
//                    float value = (p2.x-p1.x) * (p2.y+p1.y);
//                    sum+=value;
//
//                    if(shape.getStart()!=shape.getEnd()) {
//                        p1 = p2;
//                        p2 = shape.getEnd();
//                        value = (p2.x-p1.x) * (p2.y+p1.y);
//                        sum+=value;
//                    }
//
//
//                }
//
//                //cout << sum << ((sum>0) ? "RIGHT" : "LEFT") << endl;
//
//                if(sum<0) {
//                    reverse(sortedshapes.begin(),sortedshapes.end());
//                    for (PointsForShape* shape : sortedshapes) shape->reversed = !shape->reversed;
//
//                }
          //  }
            float moveDistanceForSortedShapes = getMoveDistanceForShapes(sortedshapes);
            // if the sorted shapes don't save much then don't bother sorting them!
            if(moveDistanceForSortedShapes/moveDistanceForUnSortedShapes > 0.9) {
                sortedshapes.clear();
                for(size_t j = 0; j<allzoneshapepoints.size(); j++) {
                    allzoneshapepoints[j].reversed = false;
                    sortedshapes.push_back(&allzoneshapepoints[j]);
                }
                
            }
            
		} else {
			for(size_t j = 0; j<allzoneshapepoints.size(); j++) {
				sortedshapes.push_back(&allzoneshapepoints[j]);
			}
			
		}
		

		// go through the point objects
		// add move between each one
		// add points to the laser
		
		ofPoint currentPosition = laserHomePosition; // MUST be in output space
		
		for(size_t j = 0; j<sortedshapes.size(); j++) {
			PointsForShape& shapepoints = *sortedshapes[j];
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
			
			if(j<sortedshapes.size()-1) {
				nextshapepoints = sortedshapes[j+1];
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
        // if we have a really fast frame, let's duplicate it and reverse it
        // (this helps for things like a single line where we maybe don't want to
        // jump back to the beginning if we can draw the line again reversed)
        if (sortShapes && (((float)laserPoints.size() / (float)pps < 0.01))) {
            //if(sortShapes && ((pps/ laserPoints.size()) >100)) {
            int numpoints = laserPoints.size();
            for(int i = numpoints-1; i>=0; i--) {
                addPoint(laserPoints[i]);
                
            }
            currentPosition = laserPoints.back();
        }
        
		//if(smoothHomePosition)
        addPointsForMoveTo(currentPosition, laserHomePosition);
		
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
	
	processPoints(masterIntensity, !dac->colourShiftImplemented); // if the colour shift isn't implemented at the DAC level, do it here
	
	if(syncToTargetFramerate && (laserPoints.size()!=targetNumPoints)) {
	
		ofLogError("syncToTargetFramerate failed! " + ofToString(targetNumPoints)+ " " + ofToString(laserPoints.size()));
	}
	
	dac->sendFrame(laserPoints);
    numPoints = (int)laserPoints.size();
	
	if(sortedshapes.size()>0) {
		if(smoothHomePosition) {
			laserHomePosition += (sortedshapes.front()->getStart()-laserHomePosition)*0.05;
		} else {
			laserHomePosition = sortedshapes.front()->getStart();
		}
	}
}

float Laser ::getMoveDistanceForShapes(vector<PointsForShape>& shapes){
    float distance = 0;
    ofPoint position = laserHomePosition;
    for(PointsForShape shape : shapes) {
        distance+= shape.getStart().distance(position);
        position = shape.getEnd();
    }
    return distance;
    
}


float Laser ::getMoveDistanceForShapes(vector<PointsForShape*>& shapes){
    float distance = 0;
    ofPoint position = laserHomePosition;
    for(PointsForShape* shape : shapes) {
        distance+= shape->getStart().distance(position);
        position = shape->getEnd();
    }
    return distance;
    
}

bool Laser ::isLaserZoneActive(OutputZone* outputZone) {
    // mute / solo functionality
    if(areAnyZonesSoloed()) {
        return outputZone->soloed;
    } else {
        return !outputZone->muted;
    }
}
void Laser ::getAllShapePoints(vector<PointsForShape>* shapepointscontainer, ofPixels*pixels, float speedmultiplier){
	
	vector<PointsForShape>& allzoneshapepoints = *shapepointscontainer;
	
	// temp vectors for storing the shapes in
	vector<PointsForShape> zonePointsForShapes;
	vector<Point> shapePointBuffer;
	
	// go through each zone
	//for(int i = 0; i<(int)laserZones.size(); i++) {
    for(OutputZone* laserZone : outputZones) {
      
       if(!isLaserZoneActive(laserZone)) continue;
        
        // if we're not using the alternate zones and this is an alternate zone then skip it
        if((!useAlternate) && (laserZone->getIsAlternate())) continue;
        
        // if we are using alternate zones, this is not an alternate zone, and we have an alternate zone, skip it!
        if((useAlternate) &&
           ((muteOnAlternate) ||
            ((!laserZone->getIsAlternate()) && (hasAltZone(&laserZone->zone))))) continue;
        
		InputZone& zone = laserZone->zone;
        //ZoneTransform& warp = laserZone->zoneTransform;
		ofRectangle maskRectangle = laserZone->zone.getRect();
        
        
        // doesn't make a copy, just a pointer to the original shapes in the zone
        deque<Shape*>* zoneshapes;
        if(paused) {
            // get pausedshapes
            zoneshapes = &pauseShapesByZone[&zone];
        } else {
            zoneshapes = &zone.shapes;
        }
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
                p = laserZone->getWarpedPoint(p);
                
                // check if it's in any of the masks!
                for(QuadMask* mask : maskManager.quads){
                    if(mask->hitTest(p.x, p.y)) {
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

deque<Shape*> Laser ::getTestPatternShapesForZone(OutputZone& laserZone) {
	
	deque<Shape*> shapes;
    if(testPattern==0) return shapes;
   
	InputZone& zone = laserZone.zone;

	ofRectangle maskRectangle = laserZone.zone.getRect();

	if(testPattern==1) {

		ofRectangle& rect = maskRectangle;

		ofColor col = ofColor(0,255,0);
		shapes.push_back(new Line(rect.getTopLeft(), rect.getTopRight(), col, OFXLASER_PROFILE_FAST));
		shapes.push_back(new Line(rect.getTopRight(), rect.getBottomRight(), col, OFXLASER_PROFILE_FAST));
		shapes.push_back(new Line(rect.getBottomRight(), rect.getBottomLeft(), col, OFXLASER_PROFILE_FAST));
		shapes.push_back(new Line(rect.getBottomLeft(), rect.getTopLeft(), col, OFXLASER_PROFILE_FAST));
		shapes.push_back(new Line(rect.getTopLeft(), rect.getBottomRight(), col, OFXLASER_PROFILE_FAST));
        shapes.push_back(new Line(rect.getTopRight(), rect.getBottomLeft(), col, OFXLASER_PROFILE_FAST));
        
        float cornersize = rect.getWidth()*0.2;
        float spacer = rect.getWidth()*0.05;
        
        if(cornersize > rect.getHeight()*0.5) {
            cornersize = rect.getHeight()*0.5;
            spacer = cornersize*0.25;
        } 
        
        shapes.push_back(new Line(rect.getTopLeft()+glm::vec3(spacer,spacer, 0), rect.getTopLeft()+glm::vec3(cornersize,spacer,0), ofColor::white, OFXLASER_PROFILE_FAST));
        shapes.push_back(new Line(rect.getTopLeft()+glm::vec3(spacer,spacer, 0), rect.getTopLeft()+glm::vec3(spacer,cornersize,0), ofColor::white, OFXLASER_PROFILE_FAST));
      //shapes.push_back(new Circle(rect.getTopLeft()+glm::vec3(cornersize,cornersize, 0), cornersize*0.8, ofColor::white, OFXLASER_PROFILE_FAST));


	} else if(testPattern==2) {

		ofRectangle rect = zone.getRect();

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

		ofRectangle rect = zone.getRect();

		ofPoint v = rect.getBottomRight() - rect.getTopLeft()-ofPoint(0.2,0.2);

		for(float y = 0; y<=1.1; y+=0.333333333) {

			shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1, rect.getTop()+0.1+v.y*y),ofPoint(rect.getRight()-0.1, rect.getTop()+0.1+v.y*y), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
		}
		shapes.push_back(new Line(rect.getTopLeft(),  glm::mix( rect.getTopLeft(), rect.getBottomLeft(), 1.0f/3.0f ), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));

		shapes.push_back(new Line(rect.getBottomLeft(), glm::mix(rect.getTopLeft(), rect.getBottomLeft(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));

		shapes.push_back(new Line( glm::mix(rect.getTopRight(), rect.getBottomRight(), 1.0f/3.0f), mix(rect.getTopRight(), rect.getBottomRight(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));


	} else if(testPattern==4) {

		ofRectangle rect = zone.getRect();

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

	} else if(testPattern ==10) {
        ofRectangle rect = maskRectangle;

        float progress = (float)(ofGetElapsedTimeMillis()%2000) /1000.0f;
        
        float x = (progress<=1) ? (ofMap(progress, 0, 1, rect.getLeft(), rect.getRight())) : (ofMap(progress, 2, 1, rect.getLeft(), rect.getRight()));
        
        shapes.push_back(new Line( ofPoint(x,rect.getTop()), ofPoint(x, rect.getBottom()), ofColor::white, OFXLASER_PROFILE_DEFAULT));

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
    
	laserPoints.push_back(p);
	
	previewPathMesh.addVertex(ofPoint(p.x, p.y));
    previewPathColoured.addVertex(ofPoint(p.x, p.y));
    previewPathColoured.addColor(p.getColour());
    
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
    
        // fine adjustments
        p+=(ofPoint)outputOffset;
        
        if(rotation!=0) {
            p.x-=400;
            p.y-=400;

            glm::vec3 vec = glm::vec3(p.x,p.y,0);
            float angle = ofDegToRad(rotation);
            
            glm::vec2 rotatedVec = glm::rotate(vec, angle, glm::vec3(0.0f, 0.0f, 1.0f));
            p.x=rotatedVec.x+400;
            p.y=rotatedVec.y+400;

        }
        
		if(flipY) p.y= 800-p.y;
		if(flipX) p.x= 800-p.x;
		
        if(mountOrientation == 1) {
            float y = 800-p.x;
            p.x = p.y;
            p.y = y;
        } else if (mountOrientation ==2) {
            p.x = 800-p.x;
            p.y = 800-p.y;
        } else if (mountOrientation ==3) {
            float y = 800-p.x;
            p.x = p.y;
            p.y = y;
            
            p.x = 800-p.x;
            p.y = 800-p.y;
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
		
//		if(!armed) {
//			p.r = 0;
//			p.g = 0;
//			p.b = 0;
//            p.x = laserHomePosition.x;
//            p.y = laserHomePosition.y;
//		}
	
		
	}
	
}


void Laser::paramsChanged(ofAbstractParameter& e){
    if(ignoreParamChange) return;
    else saveSettings();
}


bool Laser::loadSettings(vector<InputZone*>& zones){
    ignoreParamChange = true;
    ofJson json = ofLoadJson(savePath + "laser"+ ofToString(laserIndex)+".json");
    ofDeserialize(json, params);
    //ofDeserialize(json, visual3DParams);
    bool success = maskManager.deserialize(json);
    
    clearOutputZones(); 
    
    ofJson zoneNumJson = json["laserzones"];
    
    // if the json node isn't found then this should do nothing
    for(auto jsonitem : zoneNumJson) {
        //cout << "Laser::loadSettings " << (int) jsonitem << endl;
        int zoneNum = (int)jsonitem;
        
        // if a zone exists with this index then add a LaserZone for it
        if(zones.size()>zoneNum) {
            OutputZone* laserZone = new OutputZone(*zones[zoneNum]);
            outputZones.push_back(laserZone);
            
            string filename ="laser"+ ofToString(laserIndex) +"zone" + ofToString(zoneNum) + ".json";
            ofJson laserZoneJson = ofLoadJson(savePath + filename);
            if(laserZone->deserialize(laserZoneJson)) {
                success&=true;
                laserZonesLastSavedMap[filename] = laserZoneJson.dump();
                
            }
        }
    }
    
    ofJson altZoneNumJson = json["laseraltzones"];
    
    // if the json node isn't found then this should do nothing
    for(auto jsonitem : altZoneNumJson) {
        //cout << "Laser::loadSettings " << (int) jsonitem << endl;
        int zoneNum = (int)jsonitem;
        
        // if a zone exists with this index then add a LaserZone for it
        if(zones.size()>zoneNum) {
            OutputZone* laserZone = new OutputZone(*zones[zoneNum]);
            laserZone->setIsAlternate(true);
            string filename = "laser"+ ofToString(laserIndex) +"zone" + ofToString(zoneNum) + "alt.json";
            outputZones.push_back(laserZone);
            ofJson laserZoneJson = ofLoadJson(savePath + filename);

            //success &= laserZone->deserialize(laserZoneJson);
            if(laserZone->deserialize(laserZoneJson)) {
                success&=true;
                laserZonesLastSavedMap[filename] = laserZoneJson.dump();
                
            }
        }
    }
    
    paused = false;
    
    ignoreParamChange = false;
    
    if(json.empty() || (!success)) {
        return false;
    } else {
        return true;
    }
    
}


bool Laser::saveSettings(){
    // update the laser index if necessary
    string name =ofToString(laserIndex);
    if(params.getName()!= name) params.setName(name);
    
    ofJson json;
    ofSerialize(json, params);

    vector<int>laserzonenums;
    vector<int>laseraltzonenums;
    for(OutputZone* laserZone : outputZones) {
        if(laserZone->getIsAlternate()) {
            laseraltzonenums.push_back(laserZone->getZoneIndex());
        } else {
            laserzonenums.push_back(laserZone->getZoneIndex());
        }
        
    }
    json["laserzones"] = laserzonenums;
    json["laseraltzones"] = laseraltzonenums;
    

    maskManager.serialize(json);
    // Save the laser settings
    bool success = ofSavePrettyJson(savePath + "laser"+ ofToString(laserIndex) +".json", json);

    for(OutputZone* laserZone : outputZones) {
        ofJson laserzonejson;
        laserZone->serialize(laserzonejson);
        
        string filename = "laser"+ ofToString(laserIndex) +"zone" + ofToString(laserZone->getZoneIndex()) + (laserZone->getIsAlternate()?"alt.json" : ".json");
        
        bool needssave = true;
        if(laserZonesLastSavedMap.find(filename)!=laserZonesLastSavedMap.end()) {
            if(laserZonesLastSavedMap[filename]==laserzonejson.dump()) {
                needssave = false;
            }
            
        }
        if(needssave && ofSavePrettyJson(savePath + filename, laserzonejson)) {
            success &=true;
            laserZonesLastSavedMap[filename]=laserzonejson.dump();
        }
        
    }
    
    
    
    lastSaveTime = ofGetElapsedTimef();
    return success;
    
}

bool Laser :: getSaveStatus(){
    return (ofGetElapsedTimef()-lastSaveTime<1);
}


vector<OutputZone*> Laser ::getSortedOutputZones() {
    vector<OutputZone*> sortedzones;
    for(OutputZone* zone : outputZones) {
        if(!zone->getIsAlternate()) sortedzones.push_back(zone);
    }
    sort(sortedzones.begin(), sortedzones.end(),
        [](const OutputZone* a, const OutputZone* b) -> bool {
        return a->getZoneIndex() < b->getZoneIndex();
    });
    return sortedzones;
    
}

vector<OutputZone*> Laser ::getSortedOutputAltZones() {
    vector<OutputZone*> sortedzones;
    for(OutputZone* zone : outputZones) {
        if(zone->getIsAlternate()) sortedzones.push_back(zone);
    }
    sort(sortedzones.begin(), sortedzones.end(),
        [](const OutputZone* a, const OutputZone* b) -> bool {
        return a->getZoneIndex() < b->getZoneIndex();
    });
    return sortedzones;
    
}

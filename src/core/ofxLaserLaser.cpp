//
//  ofxLaserLaser.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserLaser.h"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace ofxLaser;

Laser::Laser(int _index) {
    laserIndex = _index;
    emptyDac = std::make_shared<DacEmpty>();
    dac = emptyDac;
    
    laserHomePosition = ofPoint(400,400);
    testPatternActive = false;
    testPattern = 1;
    testPatternGlobalActive = false;
    testPatternGlobal = 1;
    
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
    dac = nullptr;
    
    ofLogNotice("Laser::~Laser()"); 
}

void Laser::setDac(std::shared_ptr<DacBase>  newdac){
    
    if(dac!=newdac) {
        
        dac = newdac;
        newdac->setPointsPerSecond(pps);
        pps = newdac->getPointsPerSecond();
        newdac->setColourShift(colourChangeShift);
        newdac->maxLatencyMS = maxLatencyMS;
        dacLabel = dac->getFullId();

        armed = false; // automatically calls setArmed because of listener on parameter
    }
}

std::shared_ptr<DacBase> Laser::getDac(){
    return dac;
}

bool Laser::hasDac() {
    return dac.get() != emptyDac.get();
}

void Laser::setDacDiagnostics(bool state) {
    std::shared_ptr<DacNetworkBaseThreaded> dac = std::dynamic_pointer_cast<DacNetworkBaseThreaded> (getDac());
    if(dac!=nullptr) {
        dac->setDiagnosticsRecording(state); 
    }
    
}


bool Laser::removeDac(){
    
    if(dac) {
        dac = emptyDac;
        dacLabel = "";
        return true;
    } else {
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
    //testPattern.set("Test Pattern", 0,0,numTestPatterns);
    
    paused.set("Paused", false);
    
    params.add(dacLabel.set("dacId", ""));
    //params.add(dacAlias.set("dacAlias", ""));
    
    params.add(useAlternate.set("Use alternate zones", false));
    params.add(muteOnAlternate.set("Mute instead of alternates", false));
    
    hideContentDuringTestPattern.set("Test pattern only", true);
    ofParameterGroup laserparams;
    laserparams.setName("Laser settings");
    
    laserparams.add(speed.set("Speed", 1,0.12,2));
    
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
    advanced.add(coherentShapeSort.set("Use coherent shape sorting", false));
    advanced.add(coherentSortMemoryStrength.set("Coherent sort memory", 0.35f, 0.0f, 1.0f));
    //advanced.add(alwaysClockwise.set("Always clockwise sorting", true));
    advanced.add(targetFramerate.set("Target framerate", 25, 23, 120));
    advanced.add(syncToTargetFramerate.set("Sync to Target framerate", false));
    advanced.add(syncShift.set("Sync shift", 0, -50, 50));
    // Preview mesh generation is useful in UI mode, but it is significant work in
    // high-point scenes. Keeping this toggle in advanced settings lets us disable
    // that cost without affecting real laser output.
    advanced.add(buildPreviewPathMeshes.set("Build preview path mesh", true));
    //advanced.add(disableSpeedCompensation.set("Disable speed compensation",false));
    
    laserparams.add(advanced);
    
    params.add(laserparams);
    params.add(scannerSettings.params);
    
    ofParameterGroup renderparams;
    renderparams.setName("Render profiles");
    
    params.add(colourSettings.params);
    
    armed.addListener(this, &ofxLaser::Laser::setDacArmed);
    pps.addListener(this, &Laser::ppsChanged);
    colourChangeShift.addListener(this, &Laser::colourShiftChanged);
    
 
    dac->setPointsPerSecond(pps);
    // error checking on blank shift for older config files
    if(colourChangeShift<0) colourChangeShift = 0;
    
    
    armed = false;
    
    
    ofAddListener(params.parameterChangedE(), this, &Laser::paramsChanged);
    
    guiInitialised = true;
    
    
}

void Laser :: reset() {
    init();

    
}
void Laser :: setGlobalTestPattern(bool active, int pattern) {
    testPatternGlobal = pattern;
    testPatternGlobalActive = active;
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

    pps = dac->getPointsPerSecond(); // in case the dac can't do it
    
}
void Laser:: colourShiftChanged(float& e){
    //ofLog(OF_LOG_NOTICE, "ppsChanged"+ofToString(pps));
    //pps=round(pps/100)*100;
    //if(pps<=100) pps =100;
    dac->setColourShift(e);
}


void Laser::addZone(ZoneId zoneId, bool isAlternate) {
    
    if(hasZone(zoneId) && !isAlternate) {
        ofLog(OF_LOG_ERROR, "Laser::addZone(...) - Laser already contains zone");
        return;
    }
    if(hasAltZone(zoneId) && isAlternate) {
        ofLog(OF_LOG_ERROR, "Laser::addZone(...) - Laser already contains alt zone");
        return;
    }
    if((!hasZone(zoneId)) && isAlternate) {
        ofLog(OF_LOG_ERROR, "Laser :: addZone(...) can only add alt if laser has zone already");
    }
    
    std::shared_ptr<OutputZone> outputzone = std::make_shared<OutputZone>(zoneId);
        
    ofJson laserZoneJson;
    string filename = savePath + "laser"+ ofToString(laserIndex) +"zone" + outputzone->getZoneId().getUid() + (isAlternate?"alt.json" : ".json");
    // not sure if we still need this - does it get called by loadSettings ? I don't think it does.
    // Comment out for now
//    if(ofFile(filename).exists()) {
//        laserZoneJson = ofLoadJson(filename);
//    }
    if(!laserZoneJson.empty()) {
        outputzone->deserialize(laserZoneJson);
    } else {
        // this should be done above I think ?
        // initialise zoneTransform
        //outputzone->init(sourceRect);
        //outputzone->zoneMask = inputzone->rect;
        // if it's a canvas zone then set perspective to true by default
        if(zoneId.getType() == ZoneId :: CANVAS) {
            outputzone->zoneTransformQuad.useHomography = true;
        }
    }
    outputzone->setIsAlternate(isAlternate);

    outputZones.push_back(outputzone);
    
    // sort the zones... oh a fancy lambda check me out
    std::sort(outputZones.begin(), outputZones.end(), [](const std::shared_ptr<OutputZone>& a, const std::shared_ptr<OutputZone>& b) -> bool {
        return (a->getZoneId().getUid()<b->getZoneId().getUid());
    });
    
    saveSettings();
    
}


//void Laser::addAltZone(int zoneIndex) {
//    addZone(zone, true);
//}

void Laser::addAltZone(ZoneId zoneId){
    if(!hasAltZone(zoneId)) addZone(zoneId, true);
}

bool Laser :: hasZone(ZoneId zoneId){
   
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
       
        if((!laserZone->getIsAlternate()) && (zoneId == laserZone->getZoneId())) return true;
    }
    return false;
}


bool Laser :: hasAltZone(ZoneId zoneId){
    
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        if((laserZone->getIsAlternate()) && (zoneId == laserZone->getZoneId())) return true;
    }
    return false;
}

bool Laser :: hasAnyAltZones() {
    
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        if(laserZone->getIsAlternate()) return true;
    }
    return false;
}

//
bool Laser :: removeZone(ZoneId zoneId){
    
    std::shared_ptr<OutputZone> outputZone = getLaserZoneForZoneId(zoneId);
    
    return removeZone(outputZone);
    
}

bool Laser :: removeZone(std::shared_ptr<OutputZone>& outputZone){
    
    if(outputZone==nullptr) return false;
    deleteSettingsFileForZone(outputZone);
    
//    vector<OutputZone*>::iterator it = std::find(outputZones.begin(), outputZones.end(), outputZone);
//    
//    outputZones.erase(it);
//    delete outputZone;
  
    SebUtils::removeElementFromVector(outputZones, outputZone); 
    saveSettings();
    
    return true;
    
}


//bool Laser :: removeAltZone(InputZone* zone){
//    return removeAltZone(getLaserAltZoneForZone(zone));
//
//}

bool Laser :: removeAltZone(std::shared_ptr<OutputZone>& outputZone){
    
    if(outputZone==nullptr) return false;
    
    deleteSettingsFileForZone(outputZone);
    
    if(SebUtils::removeElementFromVector(outputZones, outputZone) ) {
        saveSettings();
        return true;
    } else {
        return false;
    }
//    vector<OutputZone*>::iterator it = std::find(outputZones.begin(), outputZones.end(), outputZone);
//    if(it!=outputZones.end()) {
//        outputZones.erase(it);
//        delete outputZone;
//        
//        saveSettings();
//        
//        return true;
//    } else {
//        return false;
//    }
    
}

bool Laser :: removeAltZone(ZoneId zoneId){
    
    for(std::shared_ptr<OutputZone>& outputZone : outputZones) {
        if(outputZone->getIsAlternate() && outputZone->getZoneId()==zoneId) {
            return removeAltZone(outputZone);
        }
        
    }
    return false;
    
}



std::shared_ptr<OutputZone> Laser::getLaserZoneForZoneId(ZoneId zoneId) {
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        if((!laserZone->getIsAlternate()) && (laserZone->getZoneId() == zoneId)) return laserZone;
    }
    return nullptr;
}


std::shared_ptr<OutputZone> Laser::getLaserAltZoneForZoneId(ZoneId zoneId){
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        if((laserZone->getIsAlternate()) && (laserZone->getZoneId() == zoneId)) return laserZone;
    }
    return nullptr;
}


const int Laser::findZoneContentIndexForId(ZoneId zoneId, const vector<ZoneContent>& zonesContent ) {
    for(int i = 0; i<zonesContent.size(); i++) {
        const ZoneContent& zoneContent = zonesContent[i];
        ZoneId zoneContentId = zoneContent.zoneId;
        if(zoneContentId == zoneId) return i;
    }
    // else return ?
    return -1;
    
}





bool Laser::hasZoneContentForId(ZoneId zoneId,const vector<ZoneContent>& zonesContent ){
    
    for(const ZoneContent& zoneContent: zonesContent) {
        ZoneId zoneContentId = zoneContent.zoneId;
        if(zoneContentId == zoneId) return true;
    }
    return false;
}

vector<std::shared_ptr<OutputZone>> Laser::getActiveZones(){
    bool soloActive = areAnyZonesSoloed();
    vector<std::shared_ptr<OutputZone>> activeZones;
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        
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


bool Laser::updateZones(map<ZoneId, ZoneId>& changedZones){
    
    bool changed = false;
    for(std::shared_ptr<OutputZone>& outputZone : outputZones) {
        //for (auto const& [key, val] : symbolTable)
        const ZoneId oldid = outputZone->getZoneId();
        ZoneId newid;
        for (const auto& kv : changedZones) {
        //for(auto const& [key, val] : changedZones) {
            if(kv.first == oldid) {
                newid = kv.second;
                outputZone->setZoneId(newid);
                changed = true;
                break;
            }
        }
    }
    
    if(changed) {
        saveSettings();
        return true;
    } else {
        return false;
    }
    
}

bool Laser::updateZoneLabels(vector<std::shared_ptr<ObjectWithZoneId>>& zoneids){
    
   // ofLogNotice("Laser::updateZoneLabels");
    bool changed = false;
    
    for(std::shared_ptr<OutputZone>& outputZone : outputZones) {
        //for (auto const& [key, val] : symbolTable)
        ZoneId id = outputZone->getZoneId();
        
        ofLogNotice(id.getUid()) << " " << id.getLabel()<< " " ;
       
        for (std::shared_ptr<ObjectWithZoneId>& objectWithZoneId : zoneids) {
            ZoneId& updatedzoneid  =objectWithZoneId->zoneId;
            if(id.getUid()==updatedzoneid.getUid()) {
                if(id.getLabel()!=updatedzoneid.getLabel()) {
                    ofLogNotice("    changed to: ") << updatedzoneid.getLabel();
                    changed = true;
                    outputZone->setZoneId(updatedzoneid);
                }
                
            }
        }
    }
    return changed;
}


void Laser::clearOutputZones() {
//    
//    for(OutputZone* zone : outputZones) {
//        delete zone;
//    }
    outputZones.clear();
}

bool Laser::areAnyZonesSoloed() {
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        if(laserZone->soloed) {
            return true;
        }
    }
    
    return false;
}

bool Laser ::muteZone(ZoneId zoneId) {
    bool changed = false;
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        if(laserZone->getZoneId() == zoneId) {
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
bool Laser ::unMuteZone(ZoneId zoneId){
    bool changed = false;
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        if(laserZone->getZoneId() == zoneId) {
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

bool Laser ::soloZone(ZoneId zoneId) {
    bool changed = false;
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        if(laserZone->getZoneId() == zoneId) {
            if(!laserZone->soloed) {
                laserZone->soloed = true;
                changed = true;
                
            } else {
                
            }
        }
    }
    return changed;
}
bool Laser ::unSoloZone(ZoneId zoneId){
    bool changed = false; 
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        if(laserZone->getZoneId() == zoneId) {
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
    if(dac) {
        return dac->getFullId();
    } else {
        return "";
    }
}


int Laser::getDacConnectedState() {
    
    if(dac) {
        return dac->getStatus();
    } else {
        return OFXLASER_DACSTATUS_NO_DAC;
    }
}


void Laser::update() {
    
    // bool soloMode = areAnyZonesSoloed();
    bool needsSave = false;
    
    // if any of the source rectangles have changed then update all the warps
    // (shouldn't need anything saving)
    //    if(updateZones) {
    //       
    //        for(OutputZone* laserZone : outputZones) {
    //            //ZoneTransform& warp = laserZone->zoneTransform;
    //            //laserZone->init(laserZone->zone.rect);
    //            //laserZone->updateHomography();
    //            
    //        }
    //        
    //        //updateZoneMasks();
    //    }
    
    needsSave = maskManager.update() | needsSave;
    
    bool laserZoneChanged = false;
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
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


void Laser::sendRawPoints(const vector<ofxLaser::Point>& points, ZoneId* zoneId, float masterIntensity ){
    // TODO FIX THIS
    clearPoints();
    //    
    //    OutputZone* laserZone = getLaserZoneForZoneIndex(targetZoneIndex);
    //    if(laserZone==nullptr) {
    //        ofLogError("Laser::sendRawPoints(...), zone "+ ofToString(targetZoneIndex+1) + " not added to laser ");
    //        return;
    //        
    //    }
    //    ofRectangle maskRectangle = laserZone->getSourceRect();
    //
    //    bool offScreen = true;
    //    
    //    vector<Point>segmentpoints;
    //    
    //    //iterate through the points
    //    for(size_t k = 0; k<points.size(); k++) {
    //        
    //        Point p = points[k];
    //        
    //        // are we outside the mask? NB can't use inside because I want points on the edge
    //        //
    //        
    //        if(p.x<maskRectangle.getLeft() ||
    //           p.x>maskRectangle.getRight() ||
    //           p.y<maskRectangle.getTop() ||
    //           p.y>maskRectangle.getBottom())  {
    //            
    //            if(!offScreen) {
    //                offScreen = true;
    //                // if we already have points then add an inbetween point
    //                if(k>0) {
    //                    Point lastpoint = p;
    //                    
    //                    // TODO better point on edge rather than just clamp
    //                    lastpoint.x = ofClamp(lastpoint.x, maskRectangle.getLeft(), maskRectangle.getRight());
    //                    lastpoint.y = ofClamp(lastpoint.y, maskRectangle.getTop(), maskRectangle.getBottom());
    //                    segmentpoints.push_back(lastpoint);
    //                    
    //                    
    //                }
    //            }
    //            
    //        } else {
    //            // we're on screen!
    //            if(offScreen) {
    //
    //                offScreen = false;
    //                if(k>0) {
    //                    Point lastpoint = points[k-1];
    //                    
    //                    // TODO better point on edge rather than just clamp
    //                    lastpoint.x = ofClamp(lastpoint.x, maskRectangle.getLeft(), maskRectangle.getRight());
    //                    lastpoint.y = ofClamp(lastpoint.y, maskRectangle.getTop(), maskRectangle.getBottom());
    //                    
    //                    segmentpoints.push_back(lastpoint);
    //                }
    //            }
    //            segmentpoints.push_back(p);
    //        }
    //        
    //        // create a point object for it
    //        
    //    } // end shapepoints
    //    // add the segment points to the points for the zone
    //
    //
    //    
    //    // go through all the points and warp them into output space
    //
    //    for(size_t k= 0; k<segmentpoints.size(); k++) {
    //        addPoint(laserZone->getWarpedPoint(segmentpoints[k]));
    //    }
    //    
    //    
    //    
    //    processPoints(masterIntensity, false);
    //    dac->sendPoints(laserPoints);
    //    
    
}


void Laser :: clearPoints() {
    laserPoints.clear();
    previewPathMesh.clear();
    previewPathColoured.clear();
}

void Laser::sortShapePointsLegacy(vector<PointsForShape>& allShapePoints, vector<PointsForShape*>& sortedShapes) {
    sortedShapes.clear();
    if(allShapePoints.empty()) {
        return;
    }

    // Legacy sorter:
    // 1) Build a route greedily by always taking the nearest unvisited shape endpoint.
    // 2) Optionally run a second insertion-style pass (`newShapeSortMethod`) to reduce travel.
    // 3) If total travel improvement is too small, keep original order to avoid pointless churn.
    for(PointsForShape& shape : allShapePoints) {
        shape.tested = false;
        shape.reversed = false;
    }

    bool reversed = false;
    float shortestDistance = INFINITY;
    PointsForShape* currentShape = nullptr;
    PointsForShape* nextShape = nullptr;
    ofPoint position = laserHomePosition;

    const float moveDistanceForUnSortedShapes = getMoveDistanceForShapes(allShapePoints);

    do {
        if(currentShape != nullptr) {
            PointsForShape& shape1 = *currentShape;
            shape1.tested = true;
            sortedShapes.push_back(&shape1);
            shape1.reversed = reversed;
            position = shape1.getEnd();
            shortestDistance = INFINITY;
            nextShape = nullptr;
        }

        // Evaluate every unvisited shape and pick whichever start/end endpoint is closest.
        // We use squared distance here (no sqrt) because only relative ordering matters.
        for(size_t i = 0; i < allShapePoints.size(); i++) {
            PointsForShape& shape2 = allShapePoints[i];
            if((currentShape == &shape2) || shape2.tested) {
                continue;
            }

            shape2.reversed = false;
            const float distanceToStart = position.squareDistance(shape2.getStart());
            if(distanceToStart < shortestDistance) {
                shortestDistance = distanceToStart;
                nextShape = &shape2;
                reversed = false;
            }

            if(shape2.reversable) {
                const float distanceToEnd = position.squareDistance(shape2.getEnd());
                if(distanceToEnd < shortestDistance) {
                    shortestDistance = distanceToEnd;
                    nextShape = &shape2;
                    reversed = true;
                }
            }
        }
        currentShape = nextShape;
    } while(currentShape != nullptr);

    if(newShapeSortMethod) {
        // Experimental second pass:
        // for each element (backwards), test whether moving it earlier lowers local bridge cost.
        for(PointsForShape* shape : sortedShapes) {
            shape->tested = false;
        }

        int currentIndex = static_cast<int>(sortedShapes.size()) - 1;
        while(currentIndex > 1) {
            PointsForShape& shape = *sortedShapes[currentIndex];
            if(shape.tested) {
                currentIndex--;
                continue;
            }

            int targetIndex = currentIndex;
            PointsForShape& neighbourAfter = *sortedShapes[(currentIndex + 1) % sortedShapes.size()];
            PointsForShape& neighbourBefore = *sortedShapes[currentIndex - 1];

            // Local bridge-delta cost: "if shape stays here", how much connector travel does it add?
            float distanceToBeat =
                neighbourAfter.getStart().distance(shape.getEnd()) +
                shape.getStart().distance(neighbourBefore.getEnd()) -
                neighbourBefore.getEnd().distance(neighbourAfter.getStart());
            // Require a minimum improvement margin to avoid tiny oscillatory edits.
            distanceToBeat *= 0.95f;

            for(int i = currentIndex - 1; i > 0; i--) {
                const int shapeIndexBefore = (i == 0) ? static_cast<int>(sortedShapes.size()) - 1 : i - 1;
                const int shapeIndexAfter = i;
                PointsForShape& shapeBefore = *sortedShapes[shapeIndexBefore];
                PointsForShape& shapeAfter = *sortedShapes[shapeIndexAfter];

                const float distanceToCompare =
                    shapeBefore.getEnd().distance(shape.getStart()) +
                    shape.getEnd().distance(shapeAfter.getStart()) -
                    shapeBefore.getEnd().distance(shapeAfter.getStart());

                if(distanceToCompare < distanceToBeat) {
                    targetIndex = i;
                    distanceToBeat = distanceToCompare;
                }
            }

            shape.tested = true;
            if(targetIndex != currentIndex) {
                sortedShapes.erase(sortedShapes.begin() + currentIndex);
                sortedShapes.insert(sortedShapes.begin() + targetIndex, &shape);
            } else {
                currentIndex--;
            }
        }
    }

    if((moveDistanceForUnSortedShapes > 0) && (!sortedShapes.empty())) {
        // Keep original order if sorting barely helps:
        // 0.9 means the sorted route must save >10% travel to be accepted.
        const float moveDistanceForSortedShapes = getMoveDistanceForShapes(sortedShapes);
        if((moveDistanceForSortedShapes / moveDistanceForUnSortedShapes) > 0.9f) {
            sortedShapes.clear();
            for(size_t j = 0; j < allShapePoints.size(); j++) {
                allShapePoints[j].reversed = false;
                sortedShapes.push_back(&allShapePoints[j]);
            }
        }
    }
}

void Laser::pruneCoherentSortMemory(const std::unordered_set<std::string>& activeZoneUids) {
    for(auto it = coherentSortMemoryByZoneUid.begin(); it != coherentSortMemoryByZoneUid.end();) {
        if(activeZoneUids.find(it->first) == activeZoneUids.end()) {
            it = coherentSortMemoryByZoneUid.erase(it);
        } else {
            ++it;
        }
    }
}

void Laser::sortShapePointsCoherent(vector<PointsForShape>& allShapePoints, vector<PointsForShape*>& sortedShapes) {
    sortedShapes.clear();
    if(allShapePoints.empty()) {
        return;
    }

    struct ShapeDescriptor {
        PointsForShape* shape = nullptr;
        glm::vec2 start;
        glm::vec2 end;
        glm::vec2 center;
        float length = 0.0f;
        bool reversable = false;
        int prevOrder = -1;
        bool preferredReversed = false;
        bool selected = false;
    };

    struct MatchCandidate {
        int currentIndex = -1;
        int prevIndex = -1;
        float score = INFINITY;
        bool preferredReversed = false;
    };

    // Coherent sorter parameters.
    // - matchGate: max geometry mismatch allowed when linking current shape to prior-frame entry.
    // - matchWeightCenter / matchWeightLength: how much center drift and length drift matter in matching.
    // - orderPenalty / orientationPenalty / unmatchedPenalty: coherence costs added in route scoring.
    // - lowMatchRatioThreshold: if too few shapes match prior frame, disable memory for quick adaptation.
    constexpr float matchGate = 220.0f;
    constexpr float matchWeightCenter = 0.35f;
    constexpr float matchWeightLength = 0.20f;
    constexpr float orderPenalty = 40.0f;
    constexpr float orientationPenalty = 18.0f;
    constexpr float unmatchedPenalty = 22.0f;
    constexpr float lowMatchRatioThreshold = 0.45f;

    for(PointsForShape& shape : allShapePoints) {
        shape.reversed = false;
        shape.tested = false;
    }

    // Memory is maintained per output-zone UID (per laser instance).
    std::unordered_map<std::string, vector<PointsForShape*>> shapesByZoneUid;
    shapesByZoneUid.reserve(allShapePoints.size());
    vector<std::string> zoneOrder;
    zoneOrder.reserve(allShapePoints.size());

    for(PointsForShape& shape : allShapePoints) {
        const std::string zoneUid = shape.zoneUid.empty() ? std::string("__global__") : shape.zoneUid;
        auto found = shapesByZoneUid.find(zoneUid);
        if(found == shapesByZoneUid.end()) {
            zoneOrder.push_back(zoneUid);
            found = shapesByZoneUid.insert({zoneUid, {}}).first;
        }
        found->second.push_back(&shape);
    }

    std::unordered_set<std::string> activeZoneUids;
    activeZoneUids.reserve(zoneOrder.size());
    for(const std::string& zoneUid : zoneOrder) {
        activeZoneUids.insert(zoneUid);
    }
    pruneCoherentSortMemory(activeZoneUids);

    ofPoint currentPosition = laserHomePosition;
    // User slider: 0 => pure travel greedy, 1 => full configured coherence penalties.
    const float memoryStrength = ofClamp(coherentSortMemoryStrength.get(), 0.0f, 1.0f);

    for(const std::string& zoneUid : zoneOrder) {
        vector<PointsForShape*>& zoneShapes = shapesByZoneUid[zoneUid];
        if(zoneShapes.empty()) {
            continue;
        }

        vector<ShapeDescriptor> descriptors;
        descriptors.reserve(zoneShapes.size());
        for(PointsForShape* shape : zoneShapes) {
            if((shape == nullptr) || shape->empty()) {
                continue;
            }

            ShapeDescriptor descriptor;
            descriptor.shape = shape;
            const Point& first = shape->front();
            const Point& last = shape->back();
            descriptor.start = glm::vec2(first.x, first.y);
            descriptor.end = glm::vec2(last.x, last.y);
            descriptor.center = (descriptor.start + descriptor.end) * 0.5f;
            descriptor.length = glm::distance(descriptor.start, descriptor.end);
            descriptor.reversable = shape->reversable;
            descriptors.push_back(descriptor);
        }

        if(descriptors.empty()) {
            coherentSortMemoryByZoneUid[zoneUid].route.clear();
            continue;
        }

        size_t matchedCount = 0;
        float effectiveMemory = memoryStrength;
        const auto previousMemory = coherentSortMemoryByZoneUid.find(zoneUid);
        if((previousMemory != coherentSortMemoryByZoneUid.end()) && (!previousMemory->second.route.empty())) {
            const vector<CoherentShapeMemoryEntry>& previousRoute = previousMemory->second.route;
            vector<MatchCandidate> candidates;
            candidates.reserve(descriptors.size() * previousRoute.size());

            // Step 1: Match current shapes to prior route entries by geometry only (no IDs).
            // We compare endpoint alignment, center movement, and length change.
            for(size_t i = 0; i < descriptors.size(); i++) {
                const ShapeDescriptor& current = descriptors[i];
                for(size_t j = 0; j < previousRoute.size(); j++) {
                    const CoherentShapeMemoryEntry& prev = previousRoute[j];

                    const float directEndpointCost =
                        glm::distance(current.start, prev.start) + glm::distance(current.end, prev.end);
                    float reversedEndpointCost = std::numeric_limits<float>::max();
                    if(current.reversable) {
                        reversedEndpointCost =
                            glm::distance(current.start, prev.end) + glm::distance(current.end, prev.start);
                    }

                    const bool preferReversed = reversedEndpointCost < directEndpointCost;
                    const float endpointCost = preferReversed ? reversedEndpointCost : directEndpointCost;
                    const float centerCost = glm::distance(current.center, prev.center);
                    const float lengthCost = std::abs(current.length - prev.length);
                    const float score =
                        endpointCost + (centerCost * matchWeightCenter) + (lengthCost * matchWeightLength);
                    // Reject weak/ambiguous matches so bad history links don't inject jitter.
                    if(score > matchGate) {
                        continue;
                    }

                    MatchCandidate candidate;
                    candidate.currentIndex = static_cast<int>(i);
                    candidate.prevIndex = static_cast<int>(j);
                    candidate.score = score;
                    candidate.preferredReversed = preferReversed;
                    candidates.push_back(candidate);
                }
            }

            std::sort(candidates.begin(), candidates.end(), [](const MatchCandidate& a, const MatchCandidate& b) {
                return a.score < b.score;
            });

            vector<bool> currentTaken(descriptors.size(), false);
            vector<bool> prevTaken(previousRoute.size(), false);
            for(const MatchCandidate& candidate : candidates) {
                if(currentTaken[candidate.currentIndex] || prevTaken[candidate.prevIndex]) {
                    continue;
                }
                currentTaken[candidate.currentIndex] = true;
                prevTaken[candidate.prevIndex] = true;
                descriptors[candidate.currentIndex].prevOrder = candidate.prevIndex;
                descriptors[candidate.currentIndex].preferredReversed = candidate.preferredReversed;
                matchedCount++;
            }

            const float matchRatio = static_cast<float>(matchedCount) / static_cast<float>(descriptors.size());
            if(matchRatio < lowMatchRatioThreshold) {
                // Scene changed significantly: prioritise fast adaptation over memory.
                effectiveMemory = 0.0f;
            }
        }

        vector<PointsForShape*> sortedZoneShapes;
        sortedZoneShapes.reserve(descriptors.size());
        int lastPrevOrder = -1;

        for(size_t placed = 0; placed < descriptors.size(); placed++) {
            int bestIndex = -1;
            bool bestReversed = false;
            float bestScore = std::numeric_limits<float>::max();
            float bestTravel = std::numeric_limits<float>::max();

            // Step 2: Build route greedily with coherence-aware scoring:
            // score = travelDistance + memoryStrength * coherencePenalty
            // When memoryStrength is 0, this becomes pure distance greedy.
            for(size_t i = 0; i < descriptors.size(); i++) {
                ShapeDescriptor& candidate = descriptors[i];
                if(candidate.selected) {
                    continue;
                }

                auto evaluateOrientation = [&](bool candidateReversed) {
                    if(candidateReversed && !candidate.reversable) {
                        return;
                    }

                    const glm::vec2 candidateStart = candidateReversed ? candidate.end : candidate.start;
                    const float travelDistance = glm::distance(glm::vec2(currentPosition.x, currentPosition.y), candidateStart);

                    float coherencePenalty = 0.0f;
                    if(effectiveMemory > 0.0f) {
                        if((lastPrevOrder >= 0) && (candidate.prevOrder >= 0)) {
                            const int expectedPrevOrder = lastPrevOrder + 1;
                            // Penalise jumps away from prior-frame order continuity.
                            coherencePenalty += std::abs(candidate.prevOrder - expectedPrevOrder) * orderPenalty;
                        } else if((lastPrevOrder >= 0) && (candidate.prevOrder < 0)) {
                            // Penalise injecting unmatched shapes in the middle of a matched sequence.
                            coherencePenalty += unmatchedPenalty;
                        }

                        if((candidate.prevOrder >= 0) && (candidateReversed != candidate.preferredReversed)) {
                            // Penalise direction flips relative to the previous-frame correspondence.
                            coherencePenalty += orientationPenalty;
                        }
                    }

                    const float totalScore = travelDistance + (effectiveMemory * coherencePenalty);
                    if((totalScore < bestScore) || ((totalScore == bestScore) && (travelDistance < bestTravel))) {
                        bestScore = totalScore;
                        bestTravel = travelDistance;
                        bestIndex = static_cast<int>(i);
                        bestReversed = candidateReversed;
                    }
                };

                evaluateOrientation(false);
                evaluateOrientation(true);
            }

            if(bestIndex < 0) {
                break;
            }

            ShapeDescriptor& selected = descriptors[bestIndex];
            selected.selected = true;
            selected.shape->reversed = bestReversed;
            selected.shape->tested = true;
            sortedZoneShapes.push_back(selected.shape);

            currentPosition = selected.shape->getEnd();
            if(selected.prevOrder >= 0) {
                lastPrevOrder = selected.prevOrder;
            }
        }

        // Step 3: Store this zone's final route as next frame's memory seed.
        CoherentZoneMemory& zoneMemory = coherentSortMemoryByZoneUid[zoneUid];
        zoneMemory.route.clear();
        zoneMemory.route.reserve(sortedZoneShapes.size());
        for(PointsForShape* sortedShape : sortedZoneShapes) {
            if((sortedShape == nullptr) || sortedShape->empty()) {
                continue;
            }
            CoherentShapeMemoryEntry memoryEntry;
            const Point& start = sortedShape->getStart();
            const Point& end = sortedShape->getEnd();
            memoryEntry.start = glm::vec2(start.x, start.y);
            memoryEntry.end = glm::vec2(end.x, end.y);
            memoryEntry.center = (memoryEntry.start + memoryEntry.end) * 0.5f;
            memoryEntry.length = glm::distance(memoryEntry.start, memoryEntry.end);
            memoryEntry.reversed = sortedShape->reversed;
            memoryEntry.reversable = sortedShape->reversable;
            zoneMemory.route.push_back(memoryEntry);
        }

        sortedShapes.insert(sortedShapes.end(), sortedZoneShapes.begin(), sortedZoneShapes.end());
    }
}


void Laser::send(const vector<ZoneContent>& zonesContent, float masterIntensity, ofPixels* pixelmask) {
    
    if(!guiInitialised) {
        ofLog(OF_LOG_ERROR, "Error, ofxLaser::laser not initialised yet. (Probably missing a ofxLaser::Manager.initGui() call...");
        return;
    }
    
    if(!dac->isReadyForFrame(maxLatencyMS)) {
        // register skipped frame
        return;
    }

    // Build a one-frame lookup table for zone content by UID.
    // The old path repeatedly performed linear scans for every output zone.
    std::unordered_map<std::string, const ZoneContent*> zoneContentLookup;
    zoneContentLookup.reserve(zonesContent.size());
    for(const ZoneContent& zoneContent : zonesContent) {
        zoneContentLookup[zoneContent.zoneId.getUid()] = &zoneContent;
    }

    auto getZoneContent = [&zoneContentLookup](const ZoneId& zoneId) -> const ZoneContent* {
        const auto lookup = zoneContentLookup.find(zoneId.getUid());
        if(lookup == zoneContentLookup.end()) {
            return nullptr;
        }
        return lookup->second;
    };
    
    // Keep per-output-zone source rectangles in sync with current content layout.
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        const ZoneContent* zoneContent = getZoneContent(laserZone->getZoneId());
        if(zoneContent != nullptr) {
            laserZone->setSourceRect(zoneContent->sourceRectangle);
        }
    }
    
    // PAUSE FUNCTION
    if(paused) {
        if(!pauseStateRecorded) {
            // record all zone shapes;
            pauseStateRecorded = true;
            
            for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
                
                
                if(laserZone->getIsAlternate()) continue; // to ensure we don't get two sets of shapes
                const ZoneContent* zoneContent = getZoneContent(laserZone->getZoneId());
                if(zoneContent != nullptr) {
                    const vector<std::shared_ptr<Shape>>& zoneShapes = zoneContent->shapes;
                    vector<std::shared_ptr<Shape>>& shapes = pauseShapesByZoneUid[laserZone->getZoneId().getUid()];
                    for(std::shared_ptr<Shape> shape : zoneShapes) {
                        shapes.push_back(shape->clone());
                    }
                }
            }
            
        }
    } else if(pauseStateRecorded) {
        pauseStateRecorded = false;
        // delete all zone shapes
        pauseShapesByZoneUid.clear();
        
    }
    
    clearPoints();
    
    vector<PointsForShape> allzoneshapepoints;
    
    // TODO add speed multiplier to getPointsForMove function
    getAllShapePoints(zonesContent, &allzoneshapepoints, pixelmask, getSpeedMultiplier(), &zoneContentLookup);
    
    vector<PointsForShape*> sortedshapes;
    
    // sort the point objects
    if(allzoneshapepoints.size()>0) {
        if(sortShapes) {
            // Coherent mode keeps per-zone route memory across frames.
            // Legacy mode is stateless and only minimises this-frame travel.
            if(coherentShapeSort) {
                sortShapePointsCoherent(allzoneshapepoints, sortedshapes);
            } else {
                coherentSortMemoryByZoneUid.clear();
                sortShapePointsLegacy(allzoneshapepoints, sortedshapes);
            }
        } else {
            for(size_t j = 0; j<allzoneshapepoints.size(); j++) {
                allzoneshapepoints[j].reversed = false;
                sortedshapes.push_back(&allzoneshapepoints[j]);
            }
            coherentSortMemoryByZoneUid.clear();
        }
        
        
        // go through the point objects
        // add move between each one
        // add points to the laser
        
        ofPoint currentPosition = laserHomePosition; // MUST be in output space
        
        for(size_t j = 0; j<sortedshapes.size(); j++) {
            PointsForShape& shapepoints = *sortedshapes[j];
            if(shapepoints.size()==0) continue;
            
            if(laserOnWhileMoving) { 
                laserMoveCol = ofColor::green;
                laserMoveCol.lerp( ofColor::red, (float)j/(float)sortedshapes.size());

            }

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
        // NOTE - DISABLED - lasers rarely going in exactly the same place in reverse so we're getting a double line
        // if we have a really fast frame, let's duplicate it and reverse it
        // (this helps for things like a single line where we maybe don't want to
        // jump back to the beginning if we can draw the line again reversed)
//        if (sortShapes && (((float)laserPoints.size() / (float)pps < 0.01))) {
//            //if(sortShapes && ((pps/ laserPoints.size()) >100)) {
//            int numpoints = laserPoints.size();
//            for(int i = numpoints-1; i>=0; i--) {
//                addPoint(laserPoints[i]);
//                
//            }
//            currentPosition = laserPoints.back();
//        }
        
        addPointsForMoveTo(currentPosition, laserHomePosition);
        
    }
    
    if (laserPoints.size() == 0) {
        laserPoints.push_back(Point(laserHomePosition, ofColor(0)));
    }
    
    
    int targetNumPoints = 0;
    
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
        
        //ofLogError("syncToTargetFramerate failed! " + ofToString(targetNumPoints)+ " " + ofToString(laserPoints.size()));
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
    // Iterate by const-reference to avoid copying each shape's full point stream.
    for(const PointsForShape& shape : shapes) {
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

bool Laser ::isLaserZoneActive(std::shared_ptr<OutputZone>& outputZone) {
    // mute / solo functionality
    if(areAnyZonesSoloed()) {
        return outputZone->soloed;
    } else {
        return !outputZone->muted;
    }
}

void Laser ::getAllShapePoints(
    const vector<ZoneContent>& zonesContent,
    vector<PointsForShape>* shapepointscontainer,
    ofPixels* pixels,
    float speedmultiplier,
    const std::unordered_map<std::string, const ZoneContent*>* zoneContentLookup
) {
    
    vector<PointsForShape>& allzoneshapepoints = *shapepointscontainer;
    
    // Temporary vectors reused for every zone in this frame.
    // Keeping them outside inner loops avoids repeated allocations.
    vector<PointsForShape> zonePointsForShapes;
    vector<Point> shapePointBuffer;
    
    // go through each zone
    for(std::shared_ptr<OutputZone>& outputZone : outputZones) {
        
        if(!isLaserZoneActive(outputZone)) continue;
        
        // if we're not using the alternate zones and this is an alternate zone then skip it
        if((!useAlternate) && (outputZone->getIsAlternate())) continue;
        
        // if we are using alternate zones, this is not an alternate zone, and we have an alternate zone, skip it!
        if((useAlternate) &&
           ((muteOnAlternate) ||
            ((!outputZone->getIsAlternate()) && (hasAltZone(outputZone->getZoneId()))))) continue;
        
        // Resolve zone content through a precomputed UID->ZoneContent table when available.
        // Fallback to the legacy linear lookup so call sites remain backwards-compatible.
        const ZoneContent* zoneContentPtr = nullptr;
        if(zoneContentLookup != nullptr) {
            const auto foundZone = zoneContentLookup->find(outputZone->getZoneId().getUid());
            if(foundZone != zoneContentLookup->end()) {
                zoneContentPtr = foundZone->second;
            }
        } else {
            const int idindex = findZoneContentIndexForId(outputZone->getZoneId(), zonesContent);
            if(idindex >= 0) {
                zoneContentPtr = &zonesContent[idindex];
            }
        }
        if(zoneContentPtr == nullptr) {
            continue;
        }
        const ZoneContent& zoneContent = *zoneContentPtr;

        // Keep a pointer to the active shape list so we can avoid copying zone shapes
        // in the common non-test-pattern path.
        const vector<std::shared_ptr<ofxLaser::Shape>>* zoneShapesPtr = nullptr;
        vector<std::shared_ptr<ofxLaser::Shape>> zoneShapesStorage;
        const string zoneUid = outputZone->getZoneId().getUid();
        if(paused) {
            const auto pausedShapesIt = pauseShapesByZoneUid.find(zoneUid);
            if(pausedShapesIt != pauseShapesByZoneUid.end()) {
                zoneShapesPtr = &pausedShapesIt->second;
            } else {
                static const vector<std::shared_ptr<ofxLaser::Shape>> emptyShapes;
                zoneShapesPtr = &emptyShapes;
            }
        } else {
            zoneShapesPtr = &zoneContent.shapes;
        }
        
        ofRectangle maskRectangle = zoneContent.sourceRectangle;
        
        // get test pattern shapes - we have to do this even if
        // we don't have a test pattern, so that the code at the end
        // of this function can delete the shapes.
        vector<std::shared_ptr<Shape>> testPatternShapes;
        
        if(testPatternActive) {
            testPatternShapes = TestPatternGenerator :: getTestPatternShapes(testPattern, zoneContent.sourceRectangle);
        } else if(testPatternGlobalActive) {
            testPatternShapes = TestPatternGenerator :: getTestPatternShapes(testPatternGlobal, zoneContent.sourceRectangle);
            
        }
        
        // define this here so we don't lose scope
        vector<std::shared_ptr<Shape>> zoneShapesWithTestPatternShapes;

        if(testPatternActive || testPatternGlobalActive) {
            // Merge content + test pattern only when needed.
            if(!hideContentDuringTestPattern) {
                zoneShapesWithTestPatternShapes = *zoneShapesPtr;
            }
            zoneShapesWithTestPatternShapes.insert(zoneShapesWithTestPatternShapes.end(), testPatternShapes.begin(), testPatternShapes.end());
            zoneShapesStorage = std::move(zoneShapesWithTestPatternShapes);
            zoneShapesPtr = &zoneShapesStorage;
        }
        
        // This points at either:
        // - the original zone shapes (no copy), or
        // - a merged vector containing test-pattern overlays.
        const vector<std::shared_ptr<Shape>>& shapesInZone = *zoneShapesPtr;
        
        // reuse the last vector of shapepoints
        zonePointsForShapes.clear();
        zonePointsForShapes.reserve(shapesInZone.size());
        
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
            segmentPoints.reversable = shape.getReversable();
            segmentPoints.zoneUid = zoneUid;
            
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
                            
                            Point pointOnEdge = p;
                            
                            // TODO better point on edge rather than just clamp
                            pointOnEdge.x = ofClamp(pointOnEdge.x, maskRectangle.getLeft(), maskRectangle.getRight());
                            pointOnEdge.y = ofClamp(pointOnEdge.y, maskRectangle.getTop(), maskRectangle.getBottom());
                            
                            segmentPoints.push_back(pointOnEdge);
                            
                            // Move the finished segment into the zone container.
                            // With move-enabled PointsForShape this transfers the point buffer.
                            zonePointsForShapes.push_back(std::move(segmentPoints));

                            // Reset state for the next segment while preserving behaviour.
                            segmentPoints = PointsForShape();
                            segmentPoints.reversable = shape.getReversable();
                            segmentPoints.zoneUid = zoneUid;
                            
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
                zonePointsForShapes.push_back(std::move(segmentPoints));
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
                p = outputZone->getWarpedPoint(p);
                
                // check if it's in any of the masks!
                for(std::shared_ptr<QuadMask>& mask : maskManager.quads){
                    if(mask->hitTest(p.x, p.y)) {
                        p.multiplyColour(ofMap(mask->maskLevel,100,0,0,1));
                    }
                }
                
            }
        }
        
        // add all the segments for the zone into the big container for all the segs

        std::move(zonePointsForShapes.begin(), zonePointsForShapes.end(), std::back_inserter(allzoneshapepoints));

        // delete all the test pattern shapes
//        for(size_t j = 0; j<testPatternShapes.size(); j++) {
//            delete testPatternShapes[j];
//        }
        
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


void Laser :: addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint){
    
    ofPoint target = targetpoint;
    ofPoint start = currentPosition;
    
    ofPoint v = target-start;
    
    float blanknum = (v.length()/scannerSettings.moveSpeed)/getSpeedMultiplier();// + movePointsPadding;
    
    for(int j = 0; j<blanknum; j++) {
        
        float t = Quint::easeInOut((float)j, 0.0f, 1.0f, blanknum);
        
        ofPoint c = (v* t) + start;
        addPoint(c, (laserOnWhileMoving && j%2==0) ? laserMoveCol : ofColor(0));
        
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

    // Preview meshes are editor-only visual aids. They are not part of DAC output,
    // so we can skip this work in performance-focused runs.
    if(buildPreviewPathMeshes) {
        previewPathMesh.addVertex(ofPoint(p.x, p.y));
        previewPathColoured.addVertex(ofPoint(p.x, p.y));
        previewPathColoured.addColor(p.getColour());
    }
    
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
    if((e.getName() == sortShapes.getName()) || (e.getName() == coherentShapeSort.getName())) {
        coherentSortMemoryByZoneUid.clear();
    }
    if(ignoreParamChange) return;
    else saveSettings();
}


bool Laser::loadSettings(){
    
    ofJson json = ofLoadJson(savePath + "laser"+ ofToString(laserIndex)+".json");
    // new format
    if(!json.contains("laserzones")) {
        return deserialize(json); 
    } else {
        // old format!
        
        
        ignoreParamChange = true;
        
        ofDeserialize(json, params);
        //ofDeserialize(json, visual3DParams);
        bool success = maskManager.deserialize(json);
        
        clearOutputZones();
        
        ofJson zoneNumJson = json["laserzones"];
        
        // if the json node isn't found then this should do nothing
        for(auto jsonitem : zoneNumJson) {
            //cout << "Laser::loadSettings " << (int) jsonitem << endl;
            if(jsonitem.is_string() ) {
                string zoneUid;
                jsonitem.get_to(zoneUid); // = (std::string)jsonitem;
                ofLogNotice("Laser::loadSettings, loading zoneid ") << zoneUid;
                
                // if a zone exists with this index then add a LaserZone for it
                
                std::shared_ptr<OutputZone> laserZone = std::make_shared<OutputZone> (ZoneId());
                outputZones.push_back(laserZone);
                
                string filename ="laser"+ ofToString(laserIndex) +"zone" + zoneUid + ".json";
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
            if(jsonitem.is_string() ) {
                //cout << "Laser::loadSettings " << (int) jsonitem << endl;
                string zoneUid; // = (std::string)jsonitem;
                jsonitem.get_to(zoneUid);
                // if a zone exists with this index then add a LaserZone for it
                
                std::shared_ptr<OutputZone> laserZone = std::make_shared<OutputZone>(ZoneId());
                laserZone->setIsAlternate(true);
                string filename = "laser"+ ofToString(laserIndex) +"zone" + zoneUid + "alt.json";
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
            // replace with new format!
            deleteOldZoneFiles();
            saveSettings();
            
            return true;
        }
    }
}


bool Laser::saveSettings(){
    ofLogNotice("Laser::saveSettings() - dacLabel : ") << dacLabel;
    ofJson json;
    serialize(json);
    bool success = ofSavePrettyJson(savePath + "laser"+ ofToString(laserIndex) +".json", json);
    
    lastSaveTime = ofGetElapsedTimef();
    return success;
    
}

string Laser :: getFilenameForZone(std::shared_ptr<OutputZone>& outputZone) {
    return "laser"+ ofToString(laserIndex) +"zone" + ofToString(outputZone->getZoneId().getUid()) + (outputZone->getIsAlternate()?"alt.json" : ".json");
}

void Laser :: deleteAllSettingsFiles() {
    for(std::shared_ptr<OutputZone>& outputZone : outputZones) {
        deleteSettingsFileForZone(outputZone);
    }
    ofFile :: removeFile(savePath + "laser"+ ofToString(laserIndex) +".json");
    
}



bool Laser :: deleteSettingsFileForZone(std::shared_ptr<OutputZone>& outputZone) {
    //string filename = getFilenameForZone(outputZone);
    return ofFile :: removeFile(savePath + getFilenameForZone(outputZone));
}

void Laser :: deleteOldZoneFiles() {
    ofDirectory settingsDir(savePath);
    settingsDir.listDir();
    vector<ofFile> files = settingsDir.getFiles();
    settingsDir.close();
    for(ofFile& file : files) {
        if(file.getFileName().find("zone")!=std::string::npos) {
            cout << "removing file : " << file.getBaseName() << endl;;
            file.remove();
        }
    }
    
}



void Laser :: serialize(ofJson& json) {
    
    string name =ofToString(laserIndex);
    if(params.getName()!= name) params.setName(name);
    
    ofSerialize(json, params);
        
    maskManager.serialize(json);
    
    ofJson& zonejson = json["outputzones"];
    for(std::shared_ptr<OutputZone>& laserZone : outputZones) {
        ofJson laserzonejson;
        laserZone->serialize(laserzonejson);
        zonejson.push_back(laserzonejson);
        
    }
    
    
}

bool Laser :: deserialize(ofJson& json) {
    ignoreParamChange = true;
    
    //cout << json.dump(3) << endl;
    
    ofDeserialize(json, params);
    //ofDeserialize(json, visual3DParams);
    bool success = maskManager.deserialize(json);
    
    clearOutputZones();
    
    ofJson zonejson = json["outputzones"];
    
    // if the json node isn't found then this should do nothing
    for(auto jsonitem : zonejson) {
    
        // if a zone exists with this index then add a LaserZone for it
        
        std::shared_ptr<OutputZone> laserZone = std::make_shared<OutputZone>(ZoneId());
       

        if(laserZone->deserialize(jsonitem)) {
            success&=true;
            bool exists = false;
            for(std::shared_ptr<OutputZone>& zone : outputZones) {
                ofLogNotice("comparing zones ") << zone->getZoneId().getUid() << laserZone->getZoneId().getUid();
                if(zone->getZoneId() == laserZone->getZoneId()) {
                    if(zone->getIsAlternate() == laserZone->getIsAlternate()) {
                        exists = true;
                    }
                }
                
            }
            if(!exists) outputZones.push_back(laserZone);

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



bool Laser :: getSaveStatus(){
    return (ofGetElapsedTimef()-lastSaveTime<1);
}


vector<std::shared_ptr<OutputZone>> Laser ::getSortedOutputZones() {
    vector<std::shared_ptr<OutputZone>> sortedzones;
    for(std::shared_ptr<OutputZone>& zone : outputZones) {
        if(!zone->getIsAlternate()) sortedzones.push_back(zone);
    }
    sort(sortedzones.begin(), sortedzones.end(),
         [](const std::shared_ptr<OutputZone>& a, const std::shared_ptr<OutputZone>& b) -> bool {
        return a->getZoneId()< b->getZoneId();
    });
    return sortedzones;
    
}

vector<std::shared_ptr<OutputZone>> Laser ::getSortedOutputAltZones() {
    vector<std::shared_ptr<OutputZone>> sortedzones;
    for(std::shared_ptr<OutputZone>& zone : outputZones) {
        if(zone->getIsAlternate()) sortedzones.push_back(zone);
    }
    sort(sortedzones.begin(), sortedzones.end(),
         [](const std::shared_ptr<OutputZone>& a, const std::shared_ptr<OutputZone>& b) -> bool {
        return a->getZoneId().getUid() < b->getZoneId().getUid();
    });
    return sortedzones;
    
}


float Laser :: getSpeedMultiplier() {
    if(disableSpeedCompensation) {
        return speed.get();
    } else {
        float newmultiplier = 30000.0f/(float)pps * speed.get();
       //ofLogNotice() << newmultiplier;
        return newmultiplier;
    }
    
    
    
}

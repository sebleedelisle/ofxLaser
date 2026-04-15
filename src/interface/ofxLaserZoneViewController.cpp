//
//  ofxLaserZoneViewController.cpp
//  Created by Seb Lee-Delisle on 13/01/2023.
//
//

#include "ofxLaserZoneViewController.h"

using namespace ofxLaser;
int LaserZoneViewController :: objectCount = 0;

LaserZoneViewController :: LaserZoneViewController(){
    objectCount++;
    //ofLogNotice("LaserZoneViewController() ") << objectCount;
    laser = nullptr;
//    setSourceRect(ofRectangle(0,0,800,800));
//    setOutputRect(ofRectangle(0,0,800,800));
}
LaserZoneViewController :: LaserZoneViewController(std::shared_ptr<Laser>& newlaser){
    objectCount++;
    //ofLogNotice("LaserZoneViewController() ") << objectCount;
    laser = newlaser;
}

LaserZoneViewController ::  ~LaserZoneViewController() {

   // ofLogNotice("~LaserZoneViewController() ") ;
}


bool LaserZoneViewController :: update() {

    bool wasUpdated = ScrollableView :: update();
    
    zoomEnabled = true;
    
    boundingRect = sourceRect;
    
    for(std::shared_ptr<ZoneUIBase>& zoneUi : zoneUis) {
        
        zoneUi->setScale(scale); 
        bool zoneupdated =zoneUi->update();
        
        if(zoneupdated) {
            std::shared_ptr<OutputZone> outputZone = getOutputZoneForZoneUI(zoneUi);
            if(outputZone!=nullptr) {
                zoneUi->updateDataFromUI(outputZone);
               
            } else {
                ofLogError("missing zone ui for output zone! ");
            }
        }
        boundingRect.growToInclude(zoneUi->getBoundingBox());
        
        wasUpdated|=zoneupdated;
    }
    
    vector<std::shared_ptr<QuadMask>>& quadMasks = laser->maskManager.quads;
    for(int i = 0; i< maskUis.size(); i++) {
        std::shared_ptr<MaskUIQuad>& maskUi = maskUis[i];
        // make sure the handles are resized
        maskUi->setScale(scale);
        bool maskupdated =maskUi->update();
        
        if(maskupdated) { //  && (quadMasks.size()>i)) {
            std::shared_ptr<QuadMask>& mask = quadMasks[i];
            maskUi->updateDataFromUI(mask);
        }

        boundingRect.growToInclude(maskUi->getBoundingBox());
        wasUpdated|=maskupdated;
    }
    
    
    // 1. IF zones have changed THEN update the zone interfaces
    
    bool changed = updateZones();
    changed = updateMasks() | changed;
    if(changed) {
        resetUiElements();
    } 
    checkEdges();
    
    return wasUpdated;
}
void LaserZoneViewController :: resetUiElements() {
    uiElementsSorted.clear();
    
    for(std::shared_ptr<ZoneUIBase>& zoneUi: zoneUisSorted) {
        uiElementsSorted.push_back(zoneUi);
    }
    
    for(std::shared_ptr<MaskUIQuad>& maskUi: maskUisSorted) {
        uiElementsSorted.push_back(maskUi);
    }
    //uiElementsSorted = uiElements;
}

void LaserZoneViewController :: moveMasksToBack() {
    std::shared_ptr<MaskUIQuad> firstMask = nullptr;
    
    for(int i = 0; i<uiElementsSorted.size(); i++) {
        std::shared_ptr<MoveablePoly>& uiElement = uiElementsSorted[i];
        std::shared_ptr<MaskUIQuad> mask = std::dynamic_pointer_cast<MaskUIQuad>(uiElement);
        
        if(mask!=nullptr) {
            if(firstMask == nullptr) {
                firstMask = mask;
            } else if(firstMask == mask) {
            //then we're done
                break;
            }
            
            // otherwise move this mask to the back!
            uiElementsSorted.erase(uiElementsSorted.begin() + i);
            uiElementsSorted.push_back(mask);
            i--;
            
        }
        
        
    }

}

void LaserZoneViewController :: draw() {
    
    if(!getIsVisible()) return;
    
    ofPushStyle();
    ofFill();
    ofSetColor(0);
    ofDrawRectangle(outputRect);
    ofPopStyle();
    
    drawFrame();
    
    
    beginViewPort();
    
   
    
    drawEdges();
    drawGrid();
    
    
    drawMoveables();
    ofPushStyle();

    ofPopStyle();
    drawLaserPath();

   
    
    
    endViewPort();
    
}



bool LaserZoneViewController :: mousePressed(ofMouseEventArgs &e){
    
    return ViewWithMoveables :: mousePressed(e);
}



void LaserZoneViewController :: drawMoveables() {

    // draw all the UI elements
    ViewWithMoveables::drawMoveables();
        
        
    
}



// TODO we should probably have a function that clears everything and sets
// up this interface for a laser

bool LaserZoneViewController :: updateZones()  {

    
    bool changed = false;
    int numZones = 0;
    if(laser!=nullptr) numZones =laser->outputZones.size();
    
    for(size_t i=0; i<numZones; i++) {
        std::shared_ptr<OutputZone>& outputZone = laser->outputZones[i];
        std::shared_ptr<ZoneUIBase> zoneUi = getZoneInterfaceForOutputZone(outputZone);
        if(zoneUi==nullptr) {
            createZoneUIForOutputZone(outputZone);
            changed = true;
        } else {
            // if we do have one let's make sure it's current
            // NOTE This only works because we call it after we have
            // updated the data from the UI components
            zoneUi->updateFromData(outputZone);
        }
    }
    
    vector<std::shared_ptr<ZoneUIBase>>::iterator it = zoneUis.begin();
    while(it != zoneUis.end()) {
        
        std::shared_ptr<ZoneUIBase>& zoneUi = *it;
        
        if(getOutputZoneForZoneUI(zoneUi)==nullptr) {
            SebUtils::removeElementFromVector(zoneUisSorted, zoneUi);
            it = zoneUis.erase(it);
            changed = true;
        }
        else ++it;
    }

    return changed;

}



bool LaserZoneViewController :: updateMasks() {
    
    bool changed = false;
    
    int numMasks = 0;
    if(laser!=nullptr) numMasks = laser->maskManager.quads.size();

    // should mean that we don't need to delete any more
    if(maskUis.size()!=numMasks) {
        maskUis.resize(numMasks);
        changed = true;
    }

    for(int i = 0; i<numMasks; i++) {
        
        std::shared_ptr<MaskUIQuad>& maskUi = maskUis[i];
        
        if(!maskUi) {
            maskUi = std::make_shared<MaskUIQuad>();
        }
        std::shared_ptr<QuadMask>& mask = laser->maskManager.quads[i];

        changed|=maskUi->updateFromData(mask);
        
    }
    
    if(changed ) maskUisSorted = maskUis;
    
    return changed;

}

void LaserZoneViewController :: drawLaserPath() {
    
    if(laser==nullptr) return;
    
    
    ofMesh& previewPathMesh = laser->previewPathMesh;
    ofMesh& previewPathColoured = laser->previewPathColoured;
    
    ofPushStyle();
    
    ofSetColor(100);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    
    ofNoFill();

    if(true) { // drawDots) {
        ofSetColor(100);
        ofSetLineWidth(0.5f);
        previewPathMesh.setMode(OF_PRIMITIVE_POINTS);
        previewPathMesh.draw();
    }
    ofSetColor(25);
    ofSetLineWidth(0.5f);
    previewPathMesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    previewPathMesh.draw();
    
    ofSetColor(255);
    
    // draw as points just to make sure the dots appear
    previewPathColoured.setMode(OF_PRIMITIVE_POINTS);
    previewPathColoured.draw();
    
    ofSetLineWidth(2.0f);
    previewPathColoured.setMode(OF_PRIMITIVE_LINE_STRIP);
    previewPathColoured.draw();
    
    ofDisableBlendMode();
    ofPopStyle();
}


void LaserZoneViewController :: deselectAllButThis(std::shared_ptr<MoveablePoly>& uielement) {
    ViewWithMoveables::deselectAllButThis(uielement);
    moveMasksToBack();
}

void LaserZoneViewController :: deselectAll() {
    ViewWithMoveables::deselectAll();
    moveMasksToBack();
}


std::shared_ptr<ZoneUIBase> LaserZoneViewController ::  getZoneInterfaceForOutputZone(std::shared_ptr<OutputZone>& outputZone) {
    
    for(std::shared_ptr<ZoneUIBase>& zoneUi: zoneUis) {
        
        if(zoneUi->zoneId.getUid() == outputZone->getZoneId().getUid()) {
            
            if((std::dynamic_pointer_cast<ZoneUIQuad>(zoneUi)!=nullptr) && (outputZone->transformType==0)) {
                return zoneUi;
            } else if((std::dynamic_pointer_cast<ZoneUIQuadComplex>(zoneUi)!=nullptr) && (outputZone->transformType==2)) {
                return zoneUi;
            } else {
                //NB assumes no doubles
                return nullptr;
            }
        }
    }
    return nullptr;
    
}

std::shared_ptr<OutputZone> LaserZoneViewController ::  getOutputZoneForZoneUI(std::shared_ptr<ZoneUIBase>& zoneUi) {
    
    if(laser==nullptr) return nullptr;
    
    int zoneType;
    if(std::dynamic_pointer_cast<ZoneUIQuad>(zoneUi)) {
        zoneType = 0;
    } else if(std::dynamic_pointer_cast<ZoneUIQuadComplex>(zoneUi)) {
        zoneType = 2;
    }
    
    for(std::shared_ptr<OutputZone>& outputZone : laser->outputZones) {
        if((outputZone->getZoneId() == zoneUi->zoneId) && (outputZone->transformType == zoneType) ) {
            return outputZone;
        }
    }
    
    return nullptr;
}

std::shared_ptr<ZoneUIBase> LaserZoneViewController::getSelectedZoneUi() {
    for (std::shared_ptr<ZoneUIBase>& zoneUi : zoneUis) {
        if (zoneUi && zoneUi->getSelected()) {
            return zoneUi;
        }
    }
    return nullptr;
}

std::shared_ptr<OutputZone> LaserZoneViewController::getSelectedOutputZone() {
    std::shared_ptr<ZoneUIBase> zoneUi = getSelectedZoneUi();
    if (!zoneUi) {
        return nullptr;
    }
    return getOutputZoneForZoneUI(zoneUi);
}


bool LaserZoneViewController :: createZoneUIForOutputZone(std::shared_ptr<OutputZone>& outputZone) {
    
    std::shared_ptr<ZoneUIBase> zoneUi;
    
    if(outputZone->transformType == 0 )  {
        
        zoneUi = std::make_shared<ZoneUIQuad>();
        
        zoneUi->updateFromData(outputZone);
        zoneUi->setGrid(snapToGrid, gridSize);
        
    }  else if(outputZone->transformType == 2 )  {
        
        zoneUi =  std::make_shared<ZoneUIQuadComplex>();
        
        zoneUi->updateFromData(outputZone);
        zoneUi->setGrid(snapToGrid, gridSize);
        
    }
//    zoneUi->zoneId = outputZone->zoneId
    if(zoneUi!=nullptr) {
        zoneUis.push_back(zoneUi);
        zoneUisSorted.push_back(zoneUi);
        return true;
        
    } else {
        return false;
    }
    
    
}
int LaserZoneViewController :: getLaserIndex() {
    if(laser!=nullptr) return laser->laserIndex;
    else return -1;
    
}


void LaserZoneViewController ::setGrid(bool snaptogrid, int gridsize, bool visible) {

    ViewWithMoveables::setGrid(snaptogrid, gridsize, visible);
    for(std::shared_ptr<MaskUIQuad>& mask : maskUis) {
        mask->setGrid(false, 1);
    }

}


// ============================================================
// Signal: zones changed — rebuild zone UIs from model
// ============================================================
void LaserZoneViewController::onZonesChanged() {
    // The existing updateZones() / resetUiElements() pattern handles this.
    // When signals fire, we refresh from the laser's authoritative zone data.
    bool changed = updateZones();
    if(changed) {
        resetUiElements();
    }
}

// ============================================================
// Signal: masks changed — rebuild mask UIs from model
// ============================================================
void LaserZoneViewController::onMasksChanged() {
    bool changed = updateMasks();
    if(changed) {
        resetUiElements();
    }
}

//
//  ofxLaserCanvasViewController.cpp
//
//  Created by Seb Lee-Delisle on 11/02/2023.
//
//

#include "ofxLaserCanvasViewController.h"

using namespace ofxLaser;

bool CanvasViewController :: update() {
    
    bool wasUpdated = ViewWithMoveables::update();

    bool anyZonesChanged = zonesChangedFlag;
    zonesChangedFlag = false;
    
    return wasUpdated || anyZonesChanged;
}

std::shared_ptr<MoveablePoly> CanvasViewController::getSelectedUiElement() {
    for (std::shared_ptr<MoveablePoly>& uiElement : uiElementsSorted) {
        if (uiElement && uiElement->getSelected()) {
            return uiElement;
        }
    }
    return nullptr;
}


bool CanvasViewController :: updateZonesFromUI(std::shared_ptr<ShapeTargetCanvas> canvasTarget){
    // TODO check if changed
    for(int i = 0; i<uiElementsSorted.size(); i++) {

        MoveablePoly& poly = *uiElementsSorted[i];

        if(poly.isExternallyManaged) continue;

        std::shared_ptr<GuideImage> guideImage = getGuideImageForUiElement(canvasTarget, &poly);
        
        if(guideImage!=nullptr) {
            
            guideImage->rect = poly.getBoundingBox();
            guideImage->locked = poly.getDisabled();
            GuideImageUiQuad* guideImageUiQuad = dynamic_cast<GuideImageUiQuad*>(&poly);
            guideImage->colour = guideImageUiQuad->colour;
            
        } else {

            std::shared_ptr<InputZone> zonepointer = canvasTarget->getInputZoneForZoneIdUid(poly.getUid());
            if(zonepointer) {
                ofRectangle rect = poly.getBoundingBox();
                zonepointer->set( rect.x, rect.y, rect.width, rect.height);
                zonepointer->locked = poly.getDisabled();
            }
            // UIDs not matching a guide image or input zone (e.g. "target0")
            // are managed externally — skip them silently
        }
    }
    
    if(sourceRect!=canvasTarget->getBounds()) {
        canvasTarget->setBounds(sourceRect);
    }
    return true;
}

void CanvasViewController :: updateUIFromZones( std::shared_ptr<ShapeTargetCanvas> canvasTarget) {
    
    if(sourceRect!=canvasTarget->getBounds()) {
        setSourceRect(canvasTarget->getBounds());
        setOutputRect(canvasTarget->getBounds());
        updateGrid();
    }
    // REALLY this whole system needs to be abstracted out so that it can better handle different
    // types of interface elements but...
    
    // make interface elements for anything we're missing
    vector<std::shared_ptr<InputZone>> inputZones = canvasTarget->getInputZones();
    for(std::shared_ptr<InputZone>& inputZone : inputZones) {
        // if the input zone doesn't have a ui element, then make one
        std::shared_ptr<MoveablePoly> uiElement = getUiElementByUid(inputZone->getZoneId().getUid());
        if(uiElement==nullptr) {
            uiElement = std::make_shared<MoveablePoly>(inputZone->getZoneId().getUid());
            uiElementsSorted.push_back(uiElement);
            uiElement->setHue(220);
            uiElement->setLabel(inputZone->getZoneId().getUid()+ ":" + inputZone->getZoneId().getLabel());
        }
    }
    
    vector<std::shared_ptr<GuideImage>>& guideImages = canvasTarget->guideImages;
    
    for(int i = 0; i< guideImages.size(); i++ ) {
        
        std::shared_ptr<GuideImage>& guideImage = guideImages.at(i);
        string uid = "guide"+ofToString(i);
        std::shared_ptr<MoveablePoly> uiElement = getUiElementByUid(uid);
        
        if(uiElement==nullptr) {
            std::shared_ptr<GuideImageUiQuad> guideImageUiQuad = std::make_shared<GuideImageUiQuad>(uid);
            ofLogNotice("copying guide image");
            guideImageUiQuad->image = guideImage->image; // should copy, hopefully safer
            guideImageUiQuad->colour = guideImage->colour;
            guideImageUiQuad->setDisabled(guideImage->locked);
            
            uiElementsSorted.push_back(guideImageUiQuad);
            
        }
        
        
    }
    
    
    vector<std::shared_ptr<MoveablePoly>> elementsToDelete;
    
    // remove interface elements for things we no longer need
    // and update the other interface elemnts with their counterparts
    
    for(std::shared_ptr<MoveablePoly>& uiElement : uiElementsSorted) {

        if(uiElement->isExternallyManaged) continue;

        std::shared_ptr<GuideImageUiQuad> guideImageUiQuad = std::dynamic_pointer_cast<GuideImageUiQuad>(uiElement);
        std::shared_ptr<GuideImage> targetGuideImage;
        
        if(guideImageUiQuad!=nullptr) {
            for(int i = 0; i< guideImages.size(); i++ ) {
                
                string uid = "guide"+ofToString(i);
                
                if(guideImageUiQuad->getUid()==uid) {
                    targetGuideImage = guideImages.at(i);
                    
                }
                
                
            }
            
            if(targetGuideImage!=nullptr) {
                uiElement->setFromRect(targetGuideImage->rect);
                uiElement->setGrid(snapToGrid, gridSize);
                guideImageUiQuad->colour = targetGuideImage->colour;
                guideImageUiQuad->setDisabled(targetGuideImage->locked);
            } else {
                elementsToDelete.push_back(uiElement);
            }
            
            
        } else {
            
            
            std::shared_ptr<InputZone> targetInputZone = nullptr;
            
            for(std::shared_ptr<InputZone>& inputZone : inputZones) {
                if(inputZone->getZoneId().getUid() == uiElement->getUid()) {
                    targetInputZone = inputZone;
                    break;
                }
            }
            
            // if the uiElement doesn't have an input zone then delete it
            if(targetInputZone) {
                uiElement->setFromRect(targetInputZone->getRect());
                uiElement->setGrid(snapToGrid, gridSize);
                uiElement->setHue(220);
                uiElement->setBrightness(canvasTarget->zoneBrightness);
                uiElement->setShowLabel(!targetInputZone->locked);
                uiElement->setDisabled(targetInputZone->locked);
                
            } else {
                elementsToDelete.push_back(uiElement);
            }
            
            
        }
        
    }
    
    while(elementsToDelete.size()>0) {
        std::shared_ptr<MoveablePoly>& elementToDelete = elementsToDelete.back();
        elementsToDelete.pop_back();
        SebUtils::removeElementFromVector(uiElementsSorted, elementToDelete);
//
//        uiElementsSorted.erase(std::remove(uiElementsSorted.begin(), uiElementsSorted.end(), elementToDelete), uiElementsSorted.end());
//        delete elementToDelete;

    }

    
    
}


void CanvasViewController :: drawMoveables() {
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    // draw all the UI elements
    ViewWithMoveables::drawMoveables();
    ofDisableBlendMode();

    
}


void CanvasViewController::setOutputRect(ofRectangle rect, bool updatescaleandoffset){
    ScrollableView::setOutputRect(rect, updatescaleandoffset);
}


// ============================================================
// Signal: model changed — rebuild UI elements from LaserState
// ============================================================
void CanvasViewController::onCanvasChanged() {
    if(!laserState) return;
    // Delegate to existing method using the canvas target from LaserState
    updateUIFromZones(laserState->canvasTarget);
}

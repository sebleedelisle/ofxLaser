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


bool CanvasViewController :: updateZonesFromUI(ShapeTargetCanvas& canvasTarget){
    // TODO check if changed
    for(int i = 0; i<uiElementsSorted.size(); i++) {
        
        MoveablePoly& poly = *uiElementsSorted[i];
        
        InputZone* zonepointer = canvasTarget.getInputZoneForZoneIdUid(poly.getUid());
        if(zonepointer!=nullptr) {
            
            vector<glm::vec2*> points = poly.getPoints();
            
            // TODO better way to get MoveablePoly as rect
            float x = points[0]->x;
            float y = points[0]->y;
            float w = points[2]->x - points[0]->x;
            float h = points[2]->y - points[0]->y;

            zonepointer->set( x, y, w, h);
        }
        zonepointer->locked = poly.getDisabled();
        

    }
    
    if(sourceRect!=canvasTarget.getBounds()) {
        canvasTarget.setBounds(sourceRect);
    }
    return true;
}

void CanvasViewController :: updateUIFromZones( ShapeTargetCanvas& canvasTarget) {
    
    if(sourceRect!=canvasTarget.getBounds()) {
        setSourceRect(canvasTarget.getBounds());
        setOutputRect(canvasTarget.getBounds());
        updateGrid();
    }
    // REALLY this whole system needs to be abstracted out so that it can better handle different
    // types of interface elements but
    
    // make interface elements for anything we're missing
    vector<InputZone*> inputZones = canvasTarget.getInputZones();
    for(InputZone* inputZone : inputZones) {
        // if the input zone doesn't have a ui element, then make one
        MoveablePoly* uiElement = getUiElementByUid(inputZone->getZoneId().getUid());
        if(uiElement==nullptr) {
            uiElement = new MoveablePoly(inputZone->getZoneId().getUid());
            uiElementsSorted.push_back(uiElement);
            uiElement->setHue(220);
            uiElement->setLabel(inputZone->getZoneId().getUid()+ ":" + inputZone->getZoneId().getLabel());
        }
    }
    
    vector<MoveablePoly*> elementsToDelete;
    
    // remove interface elements for things we no longer need
    // and update the other interface elemnts with their counterparts
    
    for(MoveablePoly* uiElement : uiElementsSorted) {
       
        InputZone* targetInputZone = nullptr;
        
        for(InputZone* inputZone : inputZones) {
            if(inputZone->getZoneId().getUid() == uiElement->getUid()) {
                targetInputZone =inputZone;
                break;
            }
        }
        // if the uiElement doesn't have an input zone then delete it
        if(targetInputZone == nullptr) {
            elementsToDelete.push_back(uiElement);
        } else {
            // otherwise update it
            uiElement->setFromRect(targetInputZone->getRect());
            uiElement->setGrid(snapToGrid, gridSize);
            uiElement->setHue(220);
            uiElement->setBrightness(canvasTarget.zoneBrightness);
            uiElement->setShowLabel(!targetInputZone->locked);
            uiElement->setDisabled(targetInputZone->locked);
        }
    }
    
    while(elementsToDelete.size()>0) {
        MoveablePoly* elementToDelete = elementsToDelete.back();
        elementsToDelete.pop_back();

        uiElementsSorted.erase(std::remove(uiElementsSorted.begin(), uiElementsSorted.end(), elementToDelete), uiElementsSorted.end());
        delete elementToDelete;

    }

    
    
}


void CanvasViewController :: drawImGui() {
    vector<MoveablePoly*> uiElementsToMoveBack;
    
    for(int i = 0; i<uiElementsSorted.size(); i++) {
        ImGui::PushID(uiElementsSorted[i]->getLabel().c_str());
        //ofLogNotice(uiElements[i]->getLabel()) << " " << i;
        MoveablePoly& uiElement = *uiElementsSorted[i];
        
        
        // OutputZone* outputZone = getOutputZoneForZoneUI(zoneUi, laser->outputZones);
        string label ="CANVAS ZONE SETTINGS " + uiElement.getUid();
        //if(ImGui::BeginPopup(label.c_str())) {
        if(uiElement.getRightClickPressed()) {
            ImGui::OpenPopup(label.c_str());
            ofLogNotice("Opening pop up : ") << label;
        }
        if(ImGui::BeginPopup(label.c_str())) {
            ImGui::Text("CANVAS ZONE %s", label.c_str());
            //ImGui::Text("CANVAS ZONE %s", uiElements[i]->getLabel().c_str());
//            if(find(uiElementsSorted.begin(), uiElementsSorted.end(), &uiElement) == uiElementsSorted.begin()) {
//                UI::startDisabled();
//            }
            if(UI::Button("Move to back")) {
                uiElementsToMoveBack.push_back(&uiElement);
                ImGui::CloseCurrentPopup();
            }
           
            if(uiElement.getDisabled()) UI::secondaryColourStart();

            if(ImGui::Button(ICON_FK_LOCK)) {
                uiElement.setDisabled(!uiElement.getDisabled());
                zonesChangedFlag = true;
                
            }
            UI::secondaryColourEnd();
            
            ImGui::EndPopup();
        }
        
       
        
        ImGui::PopID();
        
    }
    
    for(MoveablePoly* uiElement : uiElementsToMoveBack) {
        vector<MoveablePoly*>::iterator it = find(uiElementsSorted.begin(), uiElementsSorted.end(), uiElement);
        if(it!=uiElementsSorted.end()) {
            uiElementsSorted.erase(it);
            uiElementsSorted.insert(uiElementsSorted.begin(), uiElement);
            uiElement->setSelected(false);
        }
        
    }
  
}



void CanvasViewController :: drawMoveables() {
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    // draw all the UI elements
    ViewWithMoveables::drawMoveables();
    ofDisableBlendMode();
    
}

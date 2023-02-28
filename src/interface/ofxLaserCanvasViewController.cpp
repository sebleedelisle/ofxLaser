//
//  ofxLaserCanvasViewController.cpp
//
//  Created by Seb Lee-Delisle on 11/02/2023.
//
//

#include "ofxLaserCanvasViewController.h"

using namespace ofxLaser;

bool CanvasViewController :: updateZonesFromUI(ShapeTargetCanvas& canvasTarget){
    //return false;
    for(int i = 0; i<uiElements.size(); i++) {
        
        InputZone* zonepointer = canvasTarget.getInputZoneForZoneIndex(i);
        if(zonepointer!=nullptr) {
            
            MoveablePoly& poly = *uiElements[i];
            vector<glm::vec2*> points = poly.getPoints();
            
            // TODO better way to get MoveablePoly as rect
            float x = points[0]->x;
            float y = points[0]->y;
            float w = points[2]->x - points[0]->x;
            float h = points[2]->y - points[0]->y;

            zonepointer->set( x, y, w, h);
        }
        
    }
    
    return true;
}
void CanvasViewController :: updateUIFromZones( ShapeTargetCanvas& canvasTarget) {
    
    for(int i = 0; i<canvasTarget.getNumZoneIds(); i++) {
        MoveablePoly* poly;
        if(uiElements.size()<=i) {
            poly = new MoveablePoly();
            uiElements.push_back(poly);
            uiElementsSorted.push_back(poly);
            poly->setHue(220); 
            
        } else {
            poly = uiElements[i];
        
        }
        vector<glm::vec2> points;
//        for(int j = 0; j<4; j++) {
//            int index = j;
//            if(index>1) index = 3 - (index%2);
//            points.push_back(zones[i]->handles[index]);
//
//        }
        InputZone* inputzone =canvasTarget.getInputZoneForZoneIndex(i);
        poly->setFromRect(inputzone->getRect());
        poly->setLabel(ofToString(i+1));
        poly->setGrid(true, 1); 
        
            
    }
    // delete extra elements from the end, and remove them from the sorted array
    for(int i = canvasTarget.getNumZoneIds(); i<uiElements.size(); i++) {
       
        uiElementsSorted.erase(std::remove(uiElementsSorted.begin(), uiElementsSorted.end(), uiElements[i]), uiElementsSorted.end());
        delete uiElements[i];
    }
    uiElements.resize(canvasTarget.getNumZoneIds());
    
    
    
}


void CanvasViewController :: drawImGui() {
    vector<MoveablePoly*> uiElementsToMoveBack;
    
    for(int i = 0; i<uiElements.size(); i++) {
        ImGui::PushID(uiElements[i]->getLabel().c_str());
        MoveablePoly& uiElement = *uiElements[i];
        // OutputZone* outputZone = getOutputZoneForZoneUI(zoneUi, laser->outputZones);
        
        if(uiElement.getRightClickPressed()) {
            ImGui::OpenPopup("CANVAS ZONE SETTINGS");
        }
        if(ImGui::BeginPopup("CANVAS ZONE SETTINGS")) {
            
            //OutputZone* laserZone : zoneUi->
            ImGui::Text("CANVAS ZONE");
            if(find(uiElementsSorted.begin(), uiElementsSorted.end(), &uiElement) == uiElementsSorted.begin()) {
                UI::startDisabled();
            }
            if(UI::Button("Move to back")) {
                uiElementsToMoveBack.push_back(&uiElement);
                ImGui::CloseCurrentPopup();
            }
            UI::stopDisabled();
            
            string buttonlabel = "DELETE ZONE";
            
            if(UI::DangerButton(buttonlabel.c_str())) {
                ImGui::OpenPopup("DELETE ZONE");
            }
            
            if(ImGui::BeginPopupModal("DELETE ZONE")) {
                
                ImGui::Text("Are you sure you want to delete this zone? All of its settings will be deleted.\n\n");
                ImGui::Separator();
                
                UI::dangerColourStart();
                if (ImGui::Button("DELETE", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    
                    // if this is an alt zone, just remove this
                    // otherwise remove this and also its alt zone if
                    // it has one
                    //int zoneindex = i;//outputZone->getZoneIndex();
                    //laser->removeZone(outputZone);
                    //laser->removeAltZone(zoneindex);
                    // LATER - TO DO - if this is a laser zone, delete it
                    // if it's a canvas zone, keep it
                    
                    deselectAll();
                    
                }
                
                UI::dangerColourEnd();
                
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            
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


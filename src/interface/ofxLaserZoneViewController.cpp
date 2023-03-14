//
//  LaserZoneView.cpp
//  Created by Seb Lee-Delisle on 13/01/2023.
//
//

#include "ofxLaserZoneViewController.h"

using namespace ofxLaser;

LaserZoneViewController :: LaserZoneViewController(){
    laser = nullptr;
    setSourceRect(ofRectangle(0,0,800,800));
    setOutputRect(ofRectangle(0,0,800,800));
}
LaserZoneViewController :: LaserZoneViewController(Laser* newlaser){
    laser = newlaser;
}

LaserZoneViewController ::  ~LaserZoneViewController() {

}


bool LaserZoneViewController :: update() {

    bool wasUpdated = false; //ScrollableView :: update();

    
    boundingRect = sourceRect;
    
    for(ZoneUiBase* zoneUi : zoneUis) {
        
        zoneUi->setScale(scale); 
        bool zoneupdated =zoneUi->update();
        
        if(zoneupdated) {
            OutputZone* outputZone = getOutputZoneForZoneUI(zoneUi, laser->outputZones);
            if(outputZone!=nullptr) {
                zoneUi->updateDataFromUi(outputZone);
               
            } else {
                ofLogError("missing zone ui for output zone! ");
            }
        }
        boundingRect.growToInclude(zoneUi->getBoundingBox());
        
        wasUpdated|=zoneupdated;
    }
    
    vector<QuadMask*>& quadMasks = laser->maskManager.quads;
    for(int i = 0; i< maskUis.size(); i++) {
        MaskUiQuad* maskUi = maskUis[i];
        // make sure the handles are resized
        maskUi->setScale(scale);
        bool maskupdated =maskUi->update();
        
        if(maskupdated) { //  && (quadMasks.size()>i)) {
            QuadMask* mask = quadMasks[i];
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
    uiElements.clear();
    
    for(ZoneUiBase* zoneUi: zoneUisSorted) {
        uiElements.push_back(zoneUi);
    }
    
    for(MaskUiQuad* maskUi: maskUisSorted) {
        uiElements.push_back(maskUi);
    }
    uiElementsSorted = uiElements;
}

void LaserZoneViewController :: moveMasksToBack() {
    MaskUiQuad* firstMask = nullptr;
    
    for(int i = 0; i<uiElementsSorted.size(); i++) {
        MoveablePoly* uiElement = uiElementsSorted[i];
        MaskUiQuad* mask = dynamic_cast<MaskUiQuad*>(uiElement);
        
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
    
    drawFrame();
    beginViewPort();
    drawEdges();
    drawGrid();
    
    
    drawMoveables();

    drawLaserPath();

//    ofNoFill();
//    ofSetColor(ofColor::red);
//    ofDrawRectangle(boundingRect);
    
    endViewPort();
    

}


// TODO we should probably have a function that clears everything and sets
// up this interface for a laser

bool LaserZoneViewController :: updateZones()  {

    // create dummy output zones in case we don't have a laser
    vector<OutputZone*> tempOutputZones;
    vector<OutputZone*>* outputZones = &tempOutputZones;
    if(laser!=nullptr) outputZones = &laser->outputZones;
    
    // create UI elements for all the zones
    bool changed = false;
    for(OutputZone* outputZone : *outputZones) {
        ZoneUiBase* zoneUi = getZoneInterfaceForOutputZone(outputZone);
        // if we don't have an interface object for the zone then make one
        if(zoneUi==nullptr) {
            createZoneUiForOutputZone(outputZone);
            changed = true;
        } else {
            // if we do have one let's make sure it's current
            // NOTE This only works because we call it after we have
            // update the data from the UI components
            zoneUi->updateFromData(outputZone);
        }
    }

    // delete no longer existing zones
    vector<ZoneUiBase*>::iterator it = zoneUis.begin();
    while(it != zoneUis.end()) {
        
        ZoneUiBase* zoneUi = *it;
        if(getOutputZoneForZoneUI(zoneUi, *outputZones)==nullptr) {
            zoneUisSorted.erase(std::remove(zoneUisSorted.begin(), zoneUisSorted.end(), zoneUi), zoneUisSorted.end());
            it = zoneUis.erase(it);
            delete zoneUi;
            changed = true;
        }
        else ++it;
    }

    return changed;
    
}



bool LaserZoneViewController :: updateMasks() {
    
    bool changed = false;
    
    vector<QuadMask*> tempVector;
    vector<QuadMask*>* vectorPointer = &tempVector;
    if(laser!=nullptr) vectorPointer = &laser->maskManager.quads;
    vector<QuadMask*>& quadMasks = *vectorPointer;
    
    for(size_t i = 0; i<quadMasks.size(); i++) {
        QuadMask* mask = quadMasks[i];
        
        // if we don't have an interface object for the mask then make one
        if(maskUis.size()<=i) {
            MaskUiQuad* newmask = new MaskUiQuad();
            newmask->updateFromData(mask);

            maskUis.push_back(newmask);
            
            changed = true;
        } else {
            // TODO - compare and set changed!
            changed|=maskUis[i]->updateFromData(mask);
        }
    }

    
    // delete no longer existing masks
    
    for(size_t i = quadMasks.size(); i<maskUis.size(); i++) {
        delete maskUis[i];
        changed = true;
    }
    maskUis.resize(quadMasks.size());
    if(changed ) maskUisSorted = maskUis; // bit nasty should probably do this better, but this should work for now
    
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


void LaserZoneViewController :: deselectAllButThis(MoveablePoly* uielement) {
    
    ViewWithMoveables::deselectAllButThis(uielement);
    moveMasksToBack(); 



}

ZoneUiBase* LaserZoneViewController ::  getZoneInterfaceForOutputZone(OutputZone* outputZone) {
    
    for(ZoneUiBase* zoneUi: zoneUis) {
        if((zoneUi->zoneId.getUid() == outputZone->getZoneId().getUid()) && (zoneUi->inputZoneAlt == outputZone->getIsAlternate())) {
            if((dynamic_cast<ZoneUiQuad*>(zoneUi)!=nullptr) && (outputZone->transformType==0)) {
                return zoneUi;
            } else if((dynamic_cast<ZoneUiLine*>(zoneUi)!=nullptr) && (outputZone->transformType==1)) {
                return zoneUi;
            } else {
                //NB assumes no doubles
                return nullptr;
            }
        }
    }
    return nullptr;
    
}

OutputZone* LaserZoneViewController ::  getOutputZoneForZoneUI(ZoneUiBase* zoneUi, vector<OutputZone*>& outputZones) {
    
    if(laser==nullptr) return nullptr;
    
    int zoneType;
    if(dynamic_cast<ZoneUiQuad*>(zoneUi)) {
        zoneType = 0;
    } else if(dynamic_cast<ZoneUiLine*>(zoneUi)) {
        zoneType = 1;
    }
    
    for(OutputZone* outputZone : laser->outputZones) {
        if((outputZone->getZoneId() == zoneUi->zoneId) && (outputZone->getIsAlternate() == zoneUi->inputZoneAlt) && (outputZone->transformType == zoneType) ) {
            return outputZone;
        }
    }
    
    return nullptr;
}


bool LaserZoneViewController :: doesAltZoneExistForZoneIndex(ZoneId zoneId) {
  
    for(ZoneUiBase* zoneUi :  zoneUis) {
        if(zoneUi->inputZoneAlt && (zoneUi->zoneId == zoneId)) {
            return true;
        }
    }
    return false; 

    
    
}

bool LaserZoneViewController :: createZoneUiForOutputZone(OutputZone* outputZone) {
    
    ZoneUiBase* zoneUi = nullptr;
    if(outputZone->transformType == 0 )  {
        
        zoneUi = new ZoneUiQuad();
        
        zoneUi->updateFromData(outputZone);
        zoneUi->setGrid(snapToGrid, gridSize);
        
    } else if(outputZone->transformType == 1 )  {
        
        zoneUi = new ZoneUiLine();
        
        zoneUi->updateFromData(outputZone);
        zoneUi->setGrid(snapToGrid, gridSize);
        
    }
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


void LaserZoneViewController :: drawImGui() {
    
    for(ZoneUiBase* zoneUi : zoneUis) {
        ImGui::PushID(zoneUi->getLabel().c_str());
        
        OutputZone* outputZone = getOutputZoneForZoneUI(zoneUi, laser->outputZones);
        
        if(zoneUi->getRightClickPressed()) {
           
            ImGui::OpenPopup("ZONE SETTINGS");
            
            
        }
        if(ImGui::BeginPopup("ZONE SETTINGS")) {
            
            //OutputZone* laserZone : zoneUi->
            ImGui::Text("ZONE SETTINGS");
            
//            if(ImGui::Checkbox("mute", &zoneUi->muted)) {
//                outputZone->muted = zoneUi->muted;
//            }
//
            
            if(UI::Button("MUTE", false, zoneUi->muted)) {
                zoneUi->muted = !zoneUi->muted;
                //outputZone->muted = zoneUi->muted;
                zoneUi->updateDataFromUi(outputZone);
            }
            
            bool quadZone = dynamic_cast<ZoneUiQuad*>(zoneUi)!=nullptr;
            
            if(quadZone) UI::secondaryColourStart();
            if(ImGui::Button("QUAD")) {
                outputZone->transformType = 0;
                zoneUi->setSelected(true);
            }
            
            UI::secondaryColourEnd();
            ImGui::SameLine();
            if(!quadZone)  UI::secondaryColourStart();
            if(ImGui::Button("LINE")) {
                outputZone->transformType = 1;
                zoneUi->setSelected(true);
            }
            UI::secondaryColourEnd();
            
            if(quadZone) {
                ZoneTransformQuadData* ztq = dynamic_cast<ZoneTransformQuadData*>(&outputZone->getZoneTransform());
                if(ztq!=nullptr) {
                    if(ztq->isAxisAligned()) {
                        UI::startDisabled();
                    }
                    if(UI::Button("Reset to square")) {
                        ztq->resetToSquare();

                    }
                    UI::stopDisabled();
                    UI::toolTip("Removes any distortion in the zone and makes all the corners right angles");
                    ImGui::SameLine();
                    if(UI::Button(ICON_FK_SQUARE_O))  {
                        ztq->setDst(ofRectangle(200,240,400,200));
                    }
                    UI::toolTip("Reset zone to default");

                    UI::addParameterGroup(ztq->transformParams, false);
                    
                }
            } else {
                
                ZoneTransformLineData* ztl = dynamic_cast<ZoneTransformLineData*>(&outputZone->getZoneTransform());
                if(ztl!=nullptr) {
                    //UI::addParameterGroup(laserZone->getZoneTransform().transformParams, false);
                    
                    UI::addFloatSlider(ztl->zoneWidth, "%.2f", 3);
                    UI::addCheckbox(ztl->locked);
//
//                    vector<BezierNode>& nodes = ztl->getNodes();
//                    for(int i = 0; i<nodes.size(); i++) {
//                        ImGui::PushID(i);
//                        BezierNode& node = nodes[i];
//                        int mode = node.mode;
//                        ImGui::Text("%d", i+1);
//                        ImGui::SameLine();
//                        //ofxLaser::UI::addCheckbox(synchroniser->useMidi);
//                        ImGui::RadioButton("LINES", &mode, 0); ImGui::SameLine();
//                        ImGui::RadioButton("FREE BEZIER", &mode, 1); ImGui::SameLine();
//                        ImGui::RadioButton("SMOOTH BEZIER", &mode, 2);
//
//                        if(mode!=node.mode) {
//                            node.mode = mode;
//                            ztl->setDirty(true);
//                        }
//                        if(nodes.size()>2) {
//                            ImGui::SameLine();
//
//                            string label = ofToString(ICON_FK_MINUS_CIRCLE) + "##" + ofToString(i);
//                            if (UI::DangerButton(label, false)) {
//                                ztl->deleteNode(i);
//
//                            }
//                        }
//
//                        ImGui::PopID();
//
//                    }
//
//                    string label = ofToString(ICON_FK_PLUS_CIRCLE) + "##addnode";
//                    if (UI::Button(label, false)) {
//                        ztl->addNode();
//
//                    }
                }
                
            }
            
            if(!zoneUi->inputZoneAlt) {
                if(doesAltZoneExistForZoneIndex(zoneUi->zoneId)) {
                    if(UI::DangerButton("DELETE ALT ZONE")) {
                        laser->removeAltZone(zoneUi->zoneId);
                    }
                    
                } else {
                    if(UI::Button("ADD ALT ZONE")) {
                        laser->addAltZone(zoneUi->zoneId);
                    }
                }
            }
            
            string buttonlabel = "DELETE ZONE";
            if(zoneUi->inputZoneAlt) buttonlabel = "DELETE ALT ZONE";
           
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
                    ZoneId zoneid = outputZone->getZoneId();
                    if(zoneUi->inputZoneAlt) {
                        laser->removeAltZone(zoneid);
                    } else {
                        ManagerBase::instance()->deleteBeamZone(zoneid);
                    }
                    //laser->removeZone(zoneid);
                    //
                    // LATER - TO DO - if this is a laser zone, delete it
                    // if it's a canvas zone, keep it
                    
                    deselectAllButThis(nullptr);
                    
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
    
    for(size_t i = 0; i< maskUis.size(); i++) {
        
        MaskUiQuad* maskUi = maskUis[i];
        
        ImGui::PushID(ofToString(maskUi).c_str()); // maybe UID of memory address? Bit dumb
        if(maskUi->getRightClickPressed()) {
            
            ImGui::OpenPopup("MASK SETTINGS");
        }
        
        if(ImGui::BeginPopup("MASK SETTINGS")) {
            ImGui::Text("MASK SETTINGS");
            int level = maskUi->maskLevel;
            ImGui::PushItemWidth(60);
            if (ImGui::DragInt("",&level,1,0,100,"%d%%")) {
                maskUi->maskLevel = level;
                maskUi->setDirty();
            }
            ImGui::PopItemWidth();
            
            string buttonlabel = "DELETE MASK";
           
            if(UI::DangerButton(buttonlabel.c_str())) {
                
                laser->maskManager.deleteQuadMask(laser->maskManager.quads[i]);
                deselectAllButThis(nullptr);
                ImGui::CloseCurrentPopup();
                //ImGui::OpenPopup("DELETE MASK");
                
            }
            
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
            
  
}
void LaserZoneViewController ::setGrid(bool snaptogrid, int gridsize) {
    
    ViewWithMoveables::setGrid(snaptogrid, gridsize);
    for(MaskUiQuad* mask : maskUis) {
        mask->setGrid(false, 1);
    }
    
}

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
    
    ScrollableView :: update();
    
    bool wasUpdated = false;
    
    for(ZoneUiBase* zoneUi : zoneUis) {
        // make sure the handles are resized
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

        wasUpdated|=maskupdated;
    }
    
    
    // 1. IF zones have changed THEN update the zone interfaces
    
    updateZones();
    updateMasks();

    
    return wasUpdated;
}
  

void LaserZoneViewController :: drawImGui() {
    
    for(ZoneUiBase* zoneUi : zoneUis) {
        ImGui::PushID(zoneUi->getLabel().c_str());
        
        OutputZone* outputZone = getOutputZoneForZoneUI(zoneUi, laser->outputZones);
        
        if(zoneUi->showContextMenu) {
            zoneUi->showContextMenu = false;
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
                outputZone->muted = zoneUi->muted;
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
                        UI::startGhosted();
                    }
                    if(UI::Button("Reset to square")) {
                        ztq->resetToSquare();

                    }
                    UI::stopGhosted();
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
                if(doesAltZoneExistForZoneIndex(zoneUi->inputZoneIndex)) {
//                    if(UI::DangerButton("DELETE ALT ZONE")) {
//                        laser->removeAltZone(zoneUi->inputZoneIndex);
//                    }
                    
                } else {
                    if(UI::Button("ADD ALT ZONE")) {
                        //laser->addAltZone(zoneUi->inputZoneIndex); ***** FIX
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
                    int zoneindex = outputZone->getZoneIndex();
                    laser->removeZone(outputZone);
                    laser->removeAltZone(zoneindex);
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
        if(maskUi->showContextMenu) {
            maskUi->showContextMenu = false;
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
//            if(ImGui::BeginPopupModal("DELETE MASK")) {
//
//                ImGui::Text("Are you sure you want to delete this mask? All of its settings will be deleted.\n\n");
//                ImGui::Separator();
//
//                UI::dangerColourStart();
//                if (ImGui::Button("DELETE", ImVec2(120, 0))) {
//                    ImGui::CloseCurrentPopup();
//
//
//                }
//
//                UI::dangerColourEnd();
//
//                ImGui::SetItemDefaultFocus();
//                ImGui::SameLine();
//                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
//                    ImGui::CloseCurrentPopup();
//                }
//                ImGui::EndPopup();
//            }
            
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
            
  
}

void LaserZoneViewController :: draw() {
    
    if(laser!=nullptr) {
        ofPushStyle();
        ofNoFill();
        ofSetColor(30);
        ofDrawRectangle(outputRect);
        ofPopStyle();
        
        ofPushView();
        ofViewport(outputRect);
        
        ofSetupScreen();
        
        ofPushMatrix();
        // offset for the viewport position
        ofTranslate(-outputRect.getTopLeft());
        ofTranslate(offset);
        ofScale(scale, scale);
        
        ofPushStyle();
        ofSetColor(0);
        ofFill();
        ofDrawRectangle(0,0,800,800);
        if(gridSize*scale<5) {
            ofSetColor(ofMap(gridSize*scale, 2, 5, 0,90, true));
        } else {
            ofSetColor(ofMap(gridSize*scale, 5, 100, 90,200, true));
        }
        ofDisableAntiAliasing();
        gridMesh.draw();
        ofEnableAntiAliasing();
        ofPopStyle();
       
        for(ZoneUiBase* zoneUi: zoneUisSorted) {
            zoneUi->draw();
            // NASTY HACK AHEAD...
            // seems to be a bit of a bug in the ofDrawBitmapString
            // when inside a viewport...
            ofPushMatrix();
            ofTranslate(-outputRect.getTopLeft() / scale);
            zoneUi->drawLabel();
            ofPopMatrix();
        }
        
       
        
        
        for(MaskUiQuad* maskUi: maskUisSorted) {
            maskUi->draw();
        }
        
        drawLaserPath();

        ofPopMatrix();
        ofPopView();
        
    }
}


// TODO we should probably have a function that clears everything and sets
// up this interface for a laser

bool LaserZoneViewController :: updateZones()  {

    // create UI elements for all the zones
    bool changed = false;
    for(OutputZone* outputZone : laser->outputZones) {
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
        if(getOutputZoneForZoneUI(zoneUi, this->laser->outputZones)==nullptr) {
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
    
    vector<QuadMask*>& quadMasks = laser->maskManager.quads;
    
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



void LaserZoneViewController :: setGrid(bool snaptogrid, int gridsize){

    if((snapToGrid!=snaptogrid) || (gridSize!=gridsize)) {
        snapToGrid = snaptogrid;
        gridSize = gridsize;
        for(ZoneUiBase* zoneUi : zoneUis) {
            zoneUi->setGrid(snaptogrid, gridsize);
        }
            

        // should we set grid on masks?
        
        gridMesh.clear();
        int spacing = gridSize;
        while(spacing<5) spacing *=2;
        for(int x = 0; x<=800; x+=spacing) {
            for(int y = 0; y<=800; y+=spacing) {
                gridMesh.addVertex(ofPoint(MIN(800,x+0.5),MIN(800,y+0.5)));
            }
        }
        gridMesh.setMode(OF_PRIMITIVE_POINTS);
    }
}

void LaserZoneViewController :: drawLaserPath() {
    
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
    
    // deselect all but the one we want
    for(MoveablePoly* zoneUi: zoneUis) {
        if(uielement!=zoneUi) zoneUi->setSelected(false);
    }
    
    // move selected to back of array
    ZoneUiBase* zoneUi = dynamic_cast<ZoneUiBase*>(uielement);
    if(zoneUi!=nullptr) {
        zoneUisSorted.erase(std::remove(zoneUisSorted.begin(), zoneUisSorted.end(), zoneUi), zoneUisSorted.end());
        zoneUisSorted.push_back(zoneUi);
    }
    
    for(MoveablePoly* maskUi: maskUis) {
        if(uielement!=maskUi) maskUi->setSelected(false);
    }
    MaskUiQuad* maskUi = dynamic_cast<MaskUiQuad*>(uielement);
    if(maskUi!=nullptr) {
        maskUisSorted.erase(std::remove(maskUisSorted.begin(), maskUisSorted.end(), maskUi), maskUisSorted.end());
        maskUisSorted.push_back(maskUi);
    }
    

}

void LaserZoneViewController :: mouseMoved(ofMouseEventArgs &e){
    ScrollableView :: mouseMoved(e);
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    for(MaskUiQuad* maskUi: maskUis) {
        maskUi->mouseMoved(mouseEvent);
        
    }
    for(ZoneUiBase* zoneUi: zoneUis) {
        zoneUi->mouseMoved(mouseEvent);
    }
    
}

bool LaserZoneViewController :: mousePressed(ofMouseEventArgs &e){

    bool propagate = true;

    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    // also check for middle button because if ALT is pressed it's reported
    // as button 2 lol 
    if((e.button == OF_MOUSE_BUTTON_LEFT)||(e.button == OF_MOUSE_BUTTON_MIDDLE)){
        
        for(int i = maskUisSorted.size()-1; i>=0; i--) {
        
            MaskUiQuad* maskUi = maskUisSorted[i];
            
            propagate &= maskUi->mousePressed(mouseEvent);
            
            if(maskUi->getSelected() ) {
                // deselect all but the one we want
                deselectAllButThis(maskUi);
                break;
            }
            
        }
        if(propagate) {
            for(int i = zoneUisSorted.size()-1; i>=0; i--) {
                ZoneUiBase* zoneUi = zoneUisSorted[i];
                
                propagate &= zoneUi->mousePressed(mouseEvent);
                
                if(zoneUi->getSelected() ) {
                    deselectAllButThis(zoneUi);
                    break;
                }
            }
        }
        // if no zones hit then check if the view should drag
        if(propagate) {
            propagate & ScrollableView::mousePressed(e);
        }
        
    } else if(e.button == OF_MOUSE_BUTTON_RIGHT) {
//
        for(size_t i = 0; i<maskUis.size(); i++) {
            MaskUiQuad* maskUi = maskUis[i];
            if(maskUi->hitTest(mouseEvent.x, mouseEvent.y)) {
                maskUi->setSelected(true);
                maskUi->showContextMenu = true;
                deselectAllButThis(maskUi);
                propagate = false;
                break;
            }

        }
        
        if(propagate) {
            
            for(int i = zoneUisSorted.size()-1; i>=0; i--) {
                ZoneUiBase* zoneUi = zoneUisSorted[i];
                
                if(zoneUi->hitTest(mouseEvent.x, mouseEvent.y)) {
                    zoneUi->setSelected(true);
                    zoneUi->showContextMenu = true;
                    deselectAllButThis(zoneUi);
                    propagate = false;
                    break;
                }
            }
        }
    }
    return propagate;
}


void LaserZoneViewController :: mouseDragged(ofMouseEventArgs &e){
    
    ScrollableView::mouseDragged(e);
    
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    
    for(MaskUiQuad* maskUi: maskUis) {
        maskUi->mouseDragged(mouseEvent);
        
    }
    for(ZoneUiBase* zoneUi: zoneUis) {
        zoneUi->mouseDragged(mouseEvent);
        
    }
    
    
    
    
}
void LaserZoneViewController :: mouseReleased(ofMouseEventArgs &e) {
    ScrollableView::mouseReleased(e);
    
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    for(MaskUiQuad* maskUi: maskUis) {
        maskUi->mouseReleased(mouseEvent);
        
    }
    for(ZoneUiBase* zoneUi: zoneUis) {
        zoneUi->mouseReleased(mouseEvent);
        
    }
    
  
    
}


ZoneUiBase* LaserZoneViewController ::  getZoneInterfaceForOutputZone(OutputZone* outputZone) {
    
    for(ZoneUiBase* zoneUi: zoneUis) {
        if((zoneUi->inputZoneIndex == outputZone->getZoneIndex()) && (zoneUi->inputZoneAlt == outputZone->getIsAlternate())) {
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
    
    int zoneType;
    if(dynamic_cast<ZoneUiQuad*>(zoneUi)) {
        zoneType = 0;
    } else if(dynamic_cast<ZoneUiLine*>(zoneUi)) {
        zoneType = 1;
    }
    
    for(OutputZone* outputZone : laser->outputZones) {
        if((outputZone->getZoneIndex() == zoneUi->inputZoneIndex) && (outputZone->getIsAlternate() == zoneUi->inputZoneAlt) && (outputZone->transformType == zoneType) ) {
            return outputZone;
        }
    }
    
    return nullptr;
}


bool LaserZoneViewController :: doesAltZoneExistForZoneIndex(int zoneIndex) {
  
    for(ZoneUiBase* zoneUi :  zoneUis) {
        if(zoneUi->inputZoneAlt && (zoneUi->inputZoneIndex == zoneIndex)) {
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

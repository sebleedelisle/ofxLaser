//
//  LaserZoneView.cpp
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
    
    zoomEnabled = !ImGui::GetIO().WantCaptureMouse;
    
    boundingRect = sourceRect;
    
    for(std::shared_ptr<ZoneUiBase>& zoneUi : zoneUis) {
        
        zoneUi->setScale(scale); 
        bool zoneupdated =zoneUi->update();
        
        if(zoneupdated) {
            std::shared_ptr<OutputZone> outputZone = getOutputZoneForZoneUI(zoneUi);
            if(outputZone!=nullptr) {
                zoneUi->updateDataFromUi(outputZone);
               
            } else {
                ofLogError("missing zone ui for output zone! ");
            }
        }
        boundingRect.growToInclude(zoneUi->getBoundingBox());
        
        wasUpdated|=zoneupdated;
    }
    
    vector<std::shared_ptr<QuadMask>>& quadMasks = laser->maskManager.quads;
    for(int i = 0; i< maskUis.size(); i++) {
        std::shared_ptr<MaskUiQuad>& maskUi = maskUis[i];
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
    
    for(std::shared_ptr<ZoneUiBase>& zoneUi: zoneUisSorted) {
        uiElementsSorted.push_back(zoneUi);
    }
    
    for(std::shared_ptr<MaskUiQuad>& maskUi: maskUisSorted) {
        uiElementsSorted.push_back(maskUi);
    }
    //uiElementsSorted = uiElements;
}

void LaserZoneViewController :: moveMasksToBack() {
    std::shared_ptr<MaskUiQuad> firstMask = nullptr;
    
    for(int i = 0; i<uiElementsSorted.size(); i++) {
        std::shared_ptr<MoveablePoly>& uiElement = uiElementsSorted[i];
        std::shared_ptr<MaskUiQuad> mask = std::dynamic_pointer_cast<MaskUiQuad>(uiElement);
        
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
    
    // draw "add point" option
    if(ofGetKeyPressed(OF_KEY_ALT) ) {
        for(std::shared_ptr<ZoneUiBase>& zoneUi: zoneUisSorted) {
            
          if(zoneUi->getSelected()) {
              std::shared_ptr<ZoneUiLine> uiLine = std::dynamic_pointer_cast<ZoneUiLine>(zoneUi);
              if(uiLine!=nullptr) {
                    glm::vec2 mousePos(ofGetMouseX(), ofGetMouseY());
                    
                    // TODO
                    
                    int closestvertexindex = uiLine->getClosestPointIndexToPosition(screenPosToLocalPos(mousePos));
                    glm::vec2 closestvertex = uiLine->getPointAtIndex(closestvertexindex);
                    float distance = glm::distance(localPosToScreenPos(closestvertex), mousePos);
                    if(distance<20 * GlobalScale::getScale()) { // distance to point, if we're close enough...

                        // and if there are more than 2 points in the line
                        if(uiLine->getNumPoints()>2) {
                            
                            ofPushMatrix();
                            
                            ofTranslate(closestvertex);
                            ofScale(1.0/scale, 1.0/scale);
                            ofSetColor(ofColor::black);
                            ofFill();
                            ofDrawCircle(0,0,8*GlobalScale::getScale());
#ifdef OFXLASER_USE_FONT_MANAGER
                            ofSetColor(uiLine->getStrokeColourSelected());
                            
                            ofxFontManager::drawString(ICON_FK_MINUS_CIRCLE, glm::vec2(), ofxFontManager::CENTRE, ofxFontManager::MIDDLE, "symbol-large" );
#else
                            
#endif
                           
                            ofPopMatrix();
                            // display the minus sign over that point...
                            
                            
                        }
                        
                        // otherwise...
                    } else  {
                        glm::vec2 closestpoint = uiLine->getClosestPointOnLine(screenPosToLocalPos(mousePos));
                        glm::vec2 closestpointscreen = localPosToScreenPos(closestpoint);
                        if(glm::distance(closestpointscreen, mousePos) < 20* GlobalScale::getScale()) {
                            
                            //ofNoFill();
                            
                            ofPushMatrix();
                            ofTranslate(closestpoint);
                            ofScale(1.0/scale, 1.0/scale);
                            ofSetColor(ofColor::black);
                            ofFill();
                            ofDrawCircle(0,0,8*GlobalScale::getScale());
#ifdef OFXLASER_USE_FONT_MANAGER
                            ofSetColor(uiLine->getStrokeColourSelected());
                            
                            ofxFontManager::drawString(ICON_FK_PLUS_CIRCLE, glm::vec2(), ofxFontManager::CENTRE, ofxFontManager::MIDDLE, "symbol-large" );
#else
                            
#endif
                            ofPopMatrix();
                        }
                        
                    }
                    
                    break;
                }
            }
            
        }
    }
    
    
    
    ofPopStyle();
    drawLaserPath();

   
    
    
    endViewPort();
    
   // if(getSelected()) {
//        ofNoFill();
//        ofSetColor(ofColor::red);
//        ofDrawRectangle(outputRect);
    //}
}



bool LaserZoneViewController :: mousePressed(ofMouseEventArgs &e){
    
    bool propagate = true;
    glm::vec2 mousePos = e;
    glm::vec2 mouseLocal = screenPosToLocalPos(e);
    
    
    if(ofGetKeyPressed(OF_KEY_ALT) ) {
        
        for(std::shared_ptr<ZoneUiBase>& zoneUi: zoneUisSorted) {
            
            std::shared_ptr<ZoneUiLine> uiLine = std::dynamic_pointer_cast<ZoneUiLine>(zoneUi);
            
            if((uiLine!=nullptr) && zoneUi->getSelected()) {
                
                int closestvertexindex = uiLine->getClosestPointIndexToPosition(mouseLocal);
                
                glm::vec2 closestvertex = uiLine->getPointAtIndex(closestvertexindex);
                
                float distance = glm::distance(localPosToScreenPos(closestvertex), mousePos);
                if(distance<20 * GlobalScale::getScale()) { // distance to point, if we're close enough...

                    // and if there are more than 2 points in the line
                    if(uiLine->getNumPoints()>2) {
                        // delete point at closestvertexindex
                        //ofLogNotice("LaserZoneViewController :: mousePressed - deleting node at ") << closestvertexindex;
                        uiLine->deleteVertex(closestvertexindex);
                        
                        propagate = false;
                    }
                    
                    // otherwise...
                } else  {
                    glm::vec2 closestpoint = uiLine->getClosestPointOnLine(screenPosToLocalPos(mousePos));
                    glm::vec2 closestpointscreen = localPosToScreenPos(closestpoint);
                    if(glm::distance(closestpointscreen, mousePos) < 20* GlobalScale::getScale()) {
                        //ofLogNotice("LaserZoneViewController :: mousePressed - adding node to line");
                        uiLine->addVertex(closestpoint);
                    }
                    //add point at closestpoint
                    propagate = false;
                    
                }
                
                
                
            }
        }
           
        if(propagate) return  ViewWithMoveables :: mousePressed(e);
        else return false;
        
    } else {
        
        return ViewWithMoveables :: mousePressed(e);
        
        
    }
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
        std::shared_ptr<ZoneUiBase> zoneUi = getZoneInterfaceForOutputZone(outputZone);
        if(zoneUi==nullptr) {
            createZoneUiForOutputZone(outputZone);
            changed = true;
        } else {
            // if we do have one let's make sure it's current
            // NOTE This only works because we call it after we have
            // updated the data from the UI components
            zoneUi->updateFromData(outputZone);
        }
    }
    
    vector<std::shared_ptr<ZoneUiBase>>::iterator it = zoneUis.begin();
    while(it != zoneUis.end()) {
        
        std::shared_ptr<ZoneUiBase>& zoneUi = *it;
        
        if(getOutputZoneForZoneUI(zoneUi)==nullptr) {
            SebUtils::removeElementFromVector(zoneUisSorted, zoneUi);
            it = zoneUis.erase(it);
            changed = true;
        }
        else ++it;
    }

    return changed;
    
    
    
//    
//    // create dummy output zones in case we don't have a laser
//    vector<OutputZone*> tempOutputZones;
//    vector<OutputZone*>* outputZones = &tempOutputZones;
//    if(laser!=nullptr) outputZones = &laser->outputZones;
//    
//    // create UI elements for all the zones
//    bool changed = false;
//    for(OutputZone* outputZone : *outputZones) {
//        std::shared_ptr<ZoneUiBase> zoneUi = getZoneInterfaceForOutputZone(outputZone);
//        // if we don't have an interface object for the zone then make one
//        if(zoneUi==nullptr) {
//            createZoneUiForOutputZone(outputZone);
//            changed = true;
//        } else {
//            // if we do have one let's make sure it's current
//            // NOTE This only works because we call it after we have
//            // update the data from the UI components
//            zoneUi->updateFromData(outputZone);
//        }
//    }
//
//    // delete no longer existing zones
//    vector<std::shared_ptr<ZoneUiBase>>::iterator it = zoneUis.begin();
//    while(it != zoneUis.end()) {
//        
//        std::shared_ptr<ZoneUiBase>& zoneUi = *it;
//        if(getOutputZoneForZoneUI(zoneUi, *outputZones)==nullptr) {
//            zoneUisSorted.erase(std::remove(zoneUisSorted.begin(), zoneUisSorted.end(), zoneUi), zoneUisSorted.end());
//            it = zoneUis.erase(it);
//            //delete zoneUi;
//            changed = true;
//        }
//        else ++it;
//    }
//
//    return changed;
    
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
        
        std::shared_ptr<MaskUiQuad>& maskUi = maskUis[i];
        
        if(!maskUi) {
            maskUi = std::make_shared<MaskUiQuad>();
        }
        std::shared_ptr<QuadMask>& mask = laser->maskManager.quads[i];
        
        changed|=maskUi->updateFromData(mask);
        
    }
    
    if(changed ) maskUisSorted = maskUis;
    
    return changed;
//
//    //for(size_t i = 0; i<quadMasks.size(); i++) {
//    
//    
//    vector<QuadMask*> tempVector;
//    vector<QuadMask*>* vectorPointer = &tempVector;
//    if(laser!=nullptr) vectorPointer = &laser->maskManager.quads;
//    vector<QuadMask*>& quadMasks = *vectorPointer;
//    
//    for(size_t i = 0; i<quadMasks.size(); i++) {
//        QuadMask* mask = quadMasks[i];
//        
//        // if we don't have an interface object for the mask then make one
//        if(maskUis.size()<=i) {
//            MaskUiQuad* newmask = new MaskUiQuad();
//            newmask->updateFromData(mask);
//
//            maskUis.push_back(newmask);
//            
//            changed = true;
//        } else {
//            // TODO - compare and set changed!
//            changed|=maskUis[i]->updateFromData(mask);
//        }
//    }
//
//    
//    // delete no longer existing masks
//    
//    for(size_t i = quadMasks.size(); i<maskUis.size(); i++) {
//        delete maskUis[i];
//        changed = true;
//    }
//    maskUis.resize(quadMasks.size());
//    if(changed ) maskUisSorted = maskUis; // bit nasty should probably do this better, but this should work for now
//    
//    return changed;
    
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


std::shared_ptr<ZoneUiBase> LaserZoneViewController ::  getZoneInterfaceForOutputZone(std::shared_ptr<OutputZone>& outputZone) {
    
    for(std::shared_ptr<ZoneUiBase>& zoneUi: zoneUis) {
        
        if((zoneUi->zoneId.getUid() == outputZone->getZoneId().getUid()) && (zoneUi->inputZoneAlt == outputZone->getIsAlternate())) {
            
            if((std::dynamic_pointer_cast<ZoneUiQuad>(zoneUi)!=nullptr) && (outputZone->transformType==0)) {
                return zoneUi;
            } else if((std::dynamic_pointer_cast<ZoneUiLine>(zoneUi)!=nullptr) && (outputZone->transformType==1)) {
                return zoneUi;
            } else if((std::dynamic_pointer_cast<ZoneUiQuadComplex>(zoneUi)!=nullptr) && (outputZone->transformType==2)) {
                return zoneUi;
            } else {
                //NB assumes no doubles
                return nullptr;
            }
        }
    }
    return nullptr;
    
}

std::shared_ptr<OutputZone> LaserZoneViewController ::  getOutputZoneForZoneUI(std::shared_ptr<ZoneUiBase>& zoneUi) {
    
    if(laser==nullptr) return nullptr;
    
    int zoneType;
    if(std::dynamic_pointer_cast<ZoneUiQuad>(zoneUi)) {
        zoneType = 0;
    } else if(std::dynamic_pointer_cast<ZoneUiLine>(zoneUi)) {
        zoneType = 1;
    } else if(std::dynamic_pointer_cast<ZoneUiQuadComplex>(zoneUi)) {
        zoneType = 2;
    }
    
    for(std::shared_ptr<OutputZone>& outputZone : laser->outputZones) {
        if((outputZone->getZoneId() == zoneUi->zoneId) && (outputZone->getIsAlternate() == zoneUi->inputZoneAlt) && (outputZone->transformType == zoneType) ) {
            return outputZone;
        }
    }
    
    return nullptr;
}


bool LaserZoneViewController :: doesAltZoneExistForZoneIndex(ZoneId zoneId) {
  
    for(std::shared_ptr<ZoneUiBase>& zoneUi :  zoneUis) {
        if(zoneUi->inputZoneAlt && (zoneUi->zoneId == zoneId)) {
            return true;
        }
    }
    return false; 

    
    
}

bool LaserZoneViewController :: createZoneUiForOutputZone(std::shared_ptr<OutputZone>& outputZone) {
    
    std::shared_ptr<ZoneUiBase> zoneUi;
    
    if(outputZone->transformType == 0 )  {
        
        zoneUi = std::make_shared<ZoneUiQuad>();
        
        zoneUi->updateFromData(outputZone);
        zoneUi->setGrid(snapToGrid, gridSize);
        
    } else if(outputZone->transformType == 1 )  {
        
        zoneUi =  std::make_shared<ZoneUiLine>();
        
        zoneUi->updateFromData(outputZone);
        zoneUi->setGrid(snapToGrid, gridSize);
        
    }  else if(outputZone->transformType == 2 )  {
        
        zoneUi =  std::make_shared<ZoneUiQuadComplex>();
        
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


void LaserZoneViewController :: drawImGui() {
    if(!isVisible) return;
    
    
#ifdef TARGET_OSX
    bool commandPressed = ofGetKeyPressed(OF_KEY_COMMAND);
    string altKey = "OPTION";
#else
    bool commandPressed = ofGetKeyPressed(OF_KEY_CONTROL);
    string altKey = "ALT";
#endif
    
    string openRenameZoneId = "";
    
    std::shared_ptr<OutputZone> outputZoneToDelete = nullptr;
    
    for(std::shared_ptr<ZoneUiBase>& zoneUi : zoneUis) {
        
        ImGui::PushID(zoneUi->zoneId.getUid().c_str());
        
        std::shared_ptr<OutputZone> outputZone = getOutputZoneForZoneUI(zoneUi);
        
        if(!outputZone) continue;
                
        bool updateOutputZone = false;
       
        
        ZoneTransformBase* zoneTransform = &outputZone->getZoneTransform();
        
        ZoneTransformQuadData* zoneTransformQuad = dynamic_cast<ZoneTransformQuadData*>(zoneTransform);
        ZoneTransformLineData* zoneTransformLine = dynamic_cast<ZoneTransformLineData*>(zoneTransform);
        ZoneTransformQuadComplexData* zoneTransformQuadComplex = dynamic_cast<ZoneTransformQuadComplexData*>(zoneTransform);
        
        string popupname = "ZONE SETTINGS##"+zoneUi->getLabel();
        
        // RIGHT CLIP MENU -----------------------------
        if(zoneUi->getRightClickPressed()) {
            ImGui::OpenPopup(popupname.c_str(), ImGuiPopupFlags_NoOpenOverExistingPopup);
            //ofLogNotice(zoneUi->getLabel().c_str());
        }
       // ImGuiID id = ImGui::GetID(popupname.c_str());
        if(ImGui::BeginPopup(popupname.c_str(), 0)) {
            
//            static char newZoneLabel[255];
//            ZoneId zoneId = outputZone->getZoneId();
//            string currentZoneLabel = zoneId.getLabel();
//            bool labelchanged = false;
            
            zoneUi->setSelected(true);
            
            ImGui::PushFont(UI::mediumFont);
            ImGui::Text("%s", zoneUi->getLabel().c_str());
            ImGui::PopFont();
            ImGui::SameLine();
            
            if(ImGui::Button("RENAME")) {
                //strcpy(newZoneLabel, currentZoneLabel.c_str());
                //string popupname = "Edit zone name " + zoneUi->zoneId.getUid();
                ImGui::CloseCurrentPopup();
                //ImGui::OpenPopup(popupname.c_str());
                //ofLogNotice("OPEN : ") << popupname;
                openRenameZoneId =zoneUi->zoneId.getUid();
            }
           
//            if (ImGui::BeginPopupModal("Edit zone name", 0)){
//                
//                if(ImGui::InputText("##1", newZoneLabel, IM_ARRAYSIZE(newZoneLabel), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsUppercase)){
//                    // don't need to do anything here
//                    //            for(int i=0;i<strlen(newDacAlias);i++){
//                    //                newDacAlias[i] = toupper(newDacAlias[i]);
//                    //            }
//                    //            ofLogNotice() << newDacAlias;
//                }
//                
//                ImGui::Separator();
//                string label = "OK## ";//+daclabel;
//                if (ImGui::Button(label.c_str(),  ImVec2(120, 0))) {
//                    string newzonelabel = newZoneLabel;
//                    //  rename zoneid here :
//                    if(zoneId.setLabel(newzonelabel)) {
//                        // save zones!
//                        labelchanged = true;// not sure if we need this
//                        zoneUi->zoneId = zoneId;
//                        updateOutputZone = true;
//                        
//                    }
//                    ImGui::CloseCurrentPopup();
//                }
//                
//                ImGui::SetItemDefaultFocus();
//                ImGui::SameLine();
//                label = "Cancel## ";//+daclabel;
//                if (ImGui::Button(label.c_str(), ImVec2(120, 0))) {
//                    ImGui::CloseCurrentPopup();
//                }
//                ImGui::EndPopup();
//            }
            
            
            ImGui::Separator();
            ImGui::PushFont(UI::mediumFont);
           
            if(UI::Button(ICON_FK_BAN, false, zoneUi->muted, ImVec2(25,25))) {
                zoneUi->muted = !zoneUi->muted;
                updateOutputZone = true;
            }
            ImGui::SameLine();
            
            UI::addDelayedTooltip("Disable output");
            UI::toolTip("Disables output to this zone");
            
            ImGui::SameLine();
            
            if(UI::Button(ICON_FK_LOCK, false, zoneUi->locked, ImVec2(25,25))) {
                zoneUi->locked = !zoneUi->locked;
                zoneUi->setSelected(!zoneUi->locked);
                updateOutputZone = true;
            }
            UI::addDelayedTooltip("Lock zone");
            UI::toolTip("Prevents the zone from being moved");
           
            ImGui::PopFont();

            ImGui::Separator();

            
            ImGui::Text("Zone shape type:");
            
            if(zoneTransformQuad) UI::secondaryColourStart();
            if(ImGui::Button("QUAD")) {
                outputZone->transformType = 0;
                zoneUi->setSelected(true);
            }
            
            UI::secondaryColourEnd();
            UI::toolTip("Standard rectangular zone. Alt-click corners to remove the constraints.");
            
            ImGui::SameLine();
            if(zoneTransformLine)  UI::secondaryColourStart();
            if(ImGui::Button("LINE / CURVE")) {
                outputZone->transformType = 1;
                zoneUi->setSelected(true);
            }
            UI::secondaryColourEnd();
            UI::toolTip("Line/curve zone. Great for very thin zones. Add points to create complex bezier curve zones.");
            
            
            ImGui::SameLine();
            if(zoneTransformQuadComplex)  UI::secondaryColourStart();
            if(ImGui::Button("SEGMENTED")) {
                outputZone->transformType = 2;
                zoneUi->setSelected(true);
            }
            UI::secondaryColourEnd();
            UI::toolTip("Segmented rectangular zone. Use for mapping and very fine control of  geometric correction.");

               

                        
    

            
            
            //ImGui::SameLine();
            if(UI::Button("RESET TO DEFAULT"))  {
                outputZone->resetAllTransforms();
            }
            UI::toolTip("Reset zone to default position, size and shape");
            
            
            
            if(zoneTransformQuad) {
                ZoneTransformQuadData* ztq = zoneTransformQuad;  //dynamic_cast<ZoneTransformQuadData*>(&outputZone->getZoneTransform());
                
                
                if(ztq->isAxisAligned()) {
                    UI::startDisabled();
                }
                
                if(UI::Button("REMOVE DISTORTION")) {
                    ztq->resetToSquare();
                }
                
                UI::stopDisabled();
                
                UI::toolTip("Removes any distortion in the zone and makes all the corners right angles");
                
                //UI::addParameterGroup(ztq->transformParams, false);
                ImGui::Checkbox("Perspective correction", (bool*)&ztq->useHomography.get());
                UI::toolTip("Undistorts the image using perspective correction (good for graphics), as opposed to bi-linear interpolation (which is better for beams). ");
                
                
            } else if(zoneTransformLine){
                
                ZoneTransformLineData* ztl = zoneTransformLine;
                
                
                //UI::addParameterGroup(laserZone->getZoneTransform().transformParams, false);
                
                UI::addFloatSlider("Zone thickness", ztl->zoneWidth, "%.2f", ImGuiSliderFlags_Logarithmic);
                
                

                bool manualbeziers = !ztl->autoSmooth;
                if(ImGui::Checkbox("Manually adjust bezier curves", &manualbeziers)) {
                    ztl->autoSmooth = !manualbeziers;
                    ztl->setDirty(true);
                }
                
                if(!ztl->autoSmooth) UI::startDisabled();
                if(ImGui::SliderFloat("Smooth level", &ztl->smoothLevel, 0, 0.5, "%.2f")) {
                    ztl->setDirty(true);
                }
                UI::stopDisabled();
                
                //s//tring altkey = "ALT";
                ImGui::Text("To add / remove points, %s click on the line", altKey.c_str());
                
//                vector<BezierNode>& nodes = ztl->getNodes();
//                for(int i = 0; i<nodes.size(); i++) {
//                    ImGui::PushID(i);
//                    BezierNode& node = nodes[i];
//                    //nt mode = node.mode;
//                    ImGui::Text("POINT %d", i+1);
//                    //ImGui::SameLine();
//                    
//                    //                        ImGui::RadioButton("LINES", &mode, 0); ImGui::SameLine();
//                    //                        ImGui::RadioButton("FREE BEZIER", &mode, 1); ImGui::SameLine();
//                    //                        ImGui::RadioButton("SMOOTH BEZIER", &mode, 2);
//                    //
//                    //                        if(mode!=node.mode) {
//                    //                            node.mode = mode;
//                    //                            ztl->setDirty(true);
//                    //                        }
//                    if(nodes.size()>2) {
//                        ImGui::SameLine();
//                        
//                        string label = "DELETE##" + ofToString(i);
//                        if (UI::DangerButton(label, false)) {
//                            ztl->deleteNode(i);
//                            
//                        }
//                    }
//                    
//                    ImGui::PopID();
//                    
//                }
                
//                string label = ofToString("ADD POINT##addnode");
//                if (UI::Button(label, false)) {
//                    //ztl->addNode();
//                    
//                }
//                
                
            } else if(zoneTransformQuadComplex) {
                
                ZoneTransformQuadComplexData* ztq = zoneTransformQuadComplex;
                
                
//                if(UI::Button("RESET TO DEFAULT"))  {
//                    ztq->setDefault(); // (ofRectangle(200,240,400,200));
//                }
//                UI::toolTip("Reset zone to default");
                
                ImGui :: Text("Subdivisions : ");
                ImGui :: SameLine();
                if(UI::Button(ofToString(ICON_FK_MINUS_CIRCLE))) {
                    ztq->decSubdivisionLevel();
                }

                ImGui :: SameLine();
                if(UI::Button(ofToString(ICON_FK_PLUS_CIRCLE))) {
                    ztq->incSubdivisionLevel();
                }
                
                //                if(ztq->isAxisAligned()) {
                //                    UI::startDisabled();
                //                }
                //                if(UI::Button("Reset to square")) {
                //                    ztq->resetToSquare();
                //                }
                //                UI::stopDisabled();
                
               
     
                
                UI::addParameterGroup(ztq->transformParams, false);
                
                
            }
            
           
            
            string buttonlabel = "DELETE ZONE";
            if(zoneUi->inputZoneAlt) buttonlabel = "DELETE ALT ZONE";
            
            if(UI::DangerButton(buttonlabel.c_str())) {
                if(commandPressed) {
                    outputZoneToDelete = outputZone;
                } else {
                    ImGui::OpenPopup("DELETE ZONE");
                }
            }
            
            if((!zoneUi->inputZoneAlt) && (!doesAltZoneExistForZoneIndex(zoneUi->zoneId))) {
                ImGui::SameLine();
                if(UI::Button("ADD ALT ZONE")) {
                    laser->addAltZone(zoneUi->zoneId);
                    zoneUi->setSelected(false);
                    ImGui::CloseCurrentPopup();
                }
            }
            
            if(ImGui::BeginPopup("DELETE ZONE")) {
                
                ImGui::Text("Are you sure you want to delete this zone?\nAll of its settings will be deleted.");
                ImGui::Separator();
                
                if (UI::DangerButton("DELETE")) {
                    //ImGui::ClosePopup("ZONE SETTINGS");
                    ImGui::ClosePopupToLevel(0, false);
                    
                    outputZoneToDelete = outputZone;
                    
                    // if this is an alt zone, just remove this
                    // otherwise remove this and also its alt zone if
                    // it has one
                    //ZoneId zoneid = outputZone->getZoneId();
                    
//                    if(zoneUi->inputZoneAlt) {
//                        //laser->removeAltZone(zoneid);
//                        zoneToDelete = zoneid;
//                        deleteAltZone = true;
//                        //deleteZone = true;
//                    } else {
//                        
//                        zoneToDelete = zoneid;
//                        deleteZone = true;
//                        
//                    }
                    //laser->removeZone(zoneid);
                    //
                    // LATER - TO DO - if this is a laser zone, delete it
                    // if it's a canvas zone, keep it
                    
                    deselectAll();
                    
                }
                
                
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (UI::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
           
            ImGui::EndPopup();
        }
        
        if(updateOutputZone) {
            zoneUi->updateDataFromUi(outputZone);
        }
        ImGui::PopID();
    }
    
    // MASK GUI --------------------------------------------------------------------
    for(size_t i = 0; i< maskUis.size(); i++) {
        
        std::shared_ptr<MaskUiQuad>& maskUi = maskUis[i];
        
        ImGui::PushID(ofToString(maskUi).c_str()); // maybe UID of memory address? Bit dumb
        if(maskUi->getRightClickPressed()) {
            
            ImGui::OpenPopup("MASK SETTINGS");
        }
        
        if(ImGui::BeginPopup("MASK SETTINGS")) {
            ImGui::Text("MASK SETTINGS");
            int level = maskUi->maskLevel;
            ImGui::PushItemWidth(60);
            string label = "##"+ofToString(i);
            if (ImGui::DragInt(label.c_str(),&level,1,0,100,"%d%%")) { 
                maskUi->maskLevel = level;
                maskUi->setDirty();
            }
            ImGui::PopItemWidth();
            
            string buttonlabel = "DELETE MASK";
           
            if(UI::DangerButton(buttonlabel.c_str())) {
                
                laser->maskManager.deleteQuadMask(laser->maskManager.quads[i]);
                deselectAll();
                ImGui::CloseCurrentPopup();
                //ImGui::OpenPopup("DELETE MASK");
                
            }
            ImGui::Text("%s %s click a corner point to make it non-uniform", ICON_FK_INFO_CIRCLE, altKey.c_str());
            
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
            
    if(outputZoneToDelete!=nullptr) {
        if(outputZoneToDelete->getZoneId().getType()==ZoneId::ZoneType::CANVAS) {
            laser->removeZone(outputZoneToDelete);
        } else {
            ManagerBase::instance()->deleteBeamZone(outputZoneToDelete);
        }
    }
    if(openRenameZoneId!="") {
        string popupname = "Edit zone name##" + openRenameZoneId;
        
        ImGui::OpenPopup(popupname.c_str());
       // ofLogNotice("OPEN : ") << popupname;
    }
  
}
void LaserZoneViewController ::setGrid(bool snaptogrid, int gridsize, bool visible) {
    
    ViewWithMoveables::setGrid(snaptogrid, gridsize, visible);
    for(std::shared_ptr<MaskUiQuad>& mask : maskUis) {
        mask->setGrid(false, 1);
    }
    
}

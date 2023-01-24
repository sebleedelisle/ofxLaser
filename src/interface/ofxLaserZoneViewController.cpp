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
    bool wasUpdated = false;
    
    // RETHINK. Need one object that can handle all the interface elements
    // of an output zone.
    // I don't really need to see any of it here, all i need to do
    // is check the output zones in the laser against my vector of
    // output zone interfaces and see if any have been added / taken away.
    
    // 1. IF zones have changed THEN update the zone interfaces
    
    updateZones();

        
//    for(int i = 0; i<laser->outputZones.size(); i++) {
//        // if the ui for this zone doesn't exist then make it
//        OutputZone* outputZone = laser->outputZones[i];
//        // if it's a quad zone
//        if(outputZone->transformType==0) {
//
//            ZoneUIQuad& zoneUiQuad = zoneUiQuads[i];
//
//            if(!zoneUiQuad.getVisible()) {
//                zoneUiQuad.setVisible(true);
//                zoneUiQuad.setEditable(true); // TODO check locked state
//                zoneUiQuad.setCorners(outputZone->zoneTransformQuad.getCornerPoints());
//            }
//            // TODO ADD HERE CODE TO CHECK IF THE UI ELEMENT IS DIRTY AND UPDATE THE DATA
//
//        } else {
//            zoneUiQuads[i].setVisible(false);
//
//        }
//
//    }
    
    // update each one, if any are dirty  DO SOMETHING (what?)
    
    for(ZoneUiBase* zoneUi : zoneUis) {
        // make sure the handles are resized
        zoneUi->setScale(scale); 
        bool zoneupdated =zoneUi->update();
        if(zoneupdated) {
            // how do we update the data?
            // For now let's just hack it
            OutputZone* outputZone = getOutputZoneForZoneUI(zoneUi, laser->outputZones);
            if(outputZone!=nullptr) {
                ZoneUiQuad* zoneUiQuad = dynamic_cast<ZoneUiQuad*>(zoneUi);
                if(zoneUiQuad!=nullptr) {
                    // THIS IS THE PART THAT CHANGES THE ZONE
                    // How do we separate this from a gross if/then statement?
                    // Should we send a message?
                    // Perhaps some functions within outputzone?
                    // Pretty gross design that we need two separate properties for
                    // different zone types lol
                    outputZone->zoneTransformQuad.setDstCorners(zoneUiQuad->handles[0], zoneUiQuad->handles[1], zoneUiQuad->handles[2], zoneUiQuad->handles[3]);
                   
                }
                
            }
            
        }
        wasUpdated|=zoneupdated;
    }
    
    return wasUpdated;
}
  
void LaserZoneViewController :: draw() {
    
    
    // OK so how do i approach this?
    // Each output zone can be one of two (three?) different types
    // each one needs a different type of interface
    // each one needs to retain information about itself so
    //  it can respond to mouse movement and drag etc
    // each one needs to share an index with the outputzone objects
    //  so we need to keep them in sync
    // So... when do we sync? I've started doing it here but maybe that's silly?
    // Don't we need to keep them updated elsewhere?
    // Or do I not keep them in sync and somehow associate visual elements with the output zones? Via an ID or something?
    // Maybe two vectors, with each type of interface? Let's try that
    
    if(laser!=nullptr) {
        ofPushView();
        ofViewport(0,0,1000,800);
        ofSetupScreen();
        ofPushMatrix();
        ofTranslate(offset);
        ofScale(scale, scale);
        
        ofPushStyle();
        ofSetColor(0);
        ofFill();
        ofDrawRectangle(0,0,800,800);
        if(gridSize*scale<3) {
            ofSetColor(ofMap(gridSize*scale, 2, 5, 0,30, true));
        } else {
            ofSetColor(ofMap(gridSize*scale, 5, 100, 30,200, true));
        }
        gridMesh.draw();
        ofPopStyle();
       
        // TODO probably need to draw these backwards yeah?
        for(ZoneUiBase* zoneUi: zoneUisSorted) {
            zoneUi->draw();
        }
        
        drawLaserPath();
        
        ofSetColor(ofColor::red);
        ofNoFill();
        ofDrawCircle(screenPosToLocalPos(glm::vec2(ofGetMouseX(), ofGetMouseY())), 10);
    
        
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
            // NO - can't do that or the interface stops working lol
            //zoneUi->updateFromOutputZone(outputZone);
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
    for(OutputZone* outputZone : laser->outputZones) {
        ZoneUiBase* zoneUi = getZoneInterfaceForOutputZone(outputZone);
        if(zoneUi!=nullptr) {
            zoneUi->setLocked(outputZone->getZoneTransform().locked);
            
        } else {
            ofLogError("Missing View for output zone!");
        }
        
    }
    
    return changed;
    
}




void LaserZoneViewController :: setGrid(bool snaptogrid, int gridsize){

    if((snapToGrid!=snaptogrid) || (gridSize!=gridsize)) {
        snapToGrid = snaptogrid;
        gridSize = gridsize;
        for(ZoneUiBase* zoneUi : zoneUis) {
            
            zoneUi->setGrid(snaptogrid, gridsize);
        }
            
    //    for(OutputZone* zone : outputZones) {
    //        zone->setGrid(snaptogrid, gridsize);
    //    }
        gridMesh.clear();
        int spacing = gridSize;
        while(gridSize<5) spacing *=2;
        for(int x = 0; x<=800; x+=spacing) {
            for(int y = 0; y<=800; y+=spacing) {
                gridMesh.addVertex(ofPoint(x,y));
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


void LaserZoneViewController :: updateSelected(ZoneUiBase* zoneUi) {
    
    // deselect all but the one we want
    for(ZoneUiBase* zoneUi2: zoneUis) {
        if(zoneUi!=zoneUi2) zoneUi2->setSelected(false);
    }
    
    // move selected to back of array
    zoneUisSorted.erase(std::remove(zoneUisSorted.begin(), zoneUisSorted.end(), zoneUi), zoneUisSorted.end());
    zoneUisSorted.push_back(zoneUi);
    

}

void LaserZoneViewController :: mouseMoved(ofMouseEventArgs &e){
    ScrollableView :: mouseMoved(e);
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    for(ZoneUiBase* zoneUi: zoneUis) {
        zoneUi->mouseMoved(mouseEvent);
    }
    
}

bool LaserZoneViewController :: mousePressed(ofMouseEventArgs &e){
    
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    bool propogate = true;
    
    for(int i = zoneUisSorted.size()-1; i>=0; i--) {
    //for(int i = 0; i<zoneUisSorted.size(); i++) {
        ZoneUiBase* zoneUi = zoneUisSorted[i];
        
        propogate &= zoneUi->mousePressed(mouseEvent);
        
        if(zoneUi->getSelected() ) {
            updateSelected(zoneUi);
            break;
        }
    }
    
    // if no zones hit then check if the view should drag
    if(propogate) {
        propogate & ScrollableView::mousePressed(e);

    }
    
    return propogate;
}


void LaserZoneViewController :: mouseDragged(ofMouseEventArgs &e){
    
    ScrollableView::mouseDragged(e);
    
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    for(ZoneUiBase* zoneUi: zoneUis) {
        zoneUi->mouseDragged(mouseEvent);
        
    }
    
    
    
    
}
void LaserZoneViewController :: mouseReleased(ofMouseEventArgs &e) {
    ScrollableView::mouseReleased(e);
    
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    for(ZoneUiBase* zoneUi: zoneUis) {
        zoneUi->mouseReleased(mouseEvent);
        
    }
    
  
    
}


ZoneUiBase* LaserZoneViewController ::  getZoneInterfaceForOutputZone(OutputZone* outputZone) {
    
    for(ZoneUiBase* zoneUi: zoneUis) {
        if((zoneUi->inputZoneIndex == outputZone->getZoneIndex()) && (zoneUi->inputZoneAlt == outputZone->getIsAlternate())) {
            if((dynamic_cast<ZoneUiQuad*>(zoneUi)!=nullptr) && (outputZone->transformType==0)) {
                return zoneUi;
//            } else if((dynamic_cast<ZoneUILine>(zoneUi)!=nullptr) && (outputZone->transformType==1)) {
//                return zoneUi;
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
//    } else if(dynamic_cast<ZoneUILine*>(zoneUi)) {
//        zoneType = 1;
    }
    
    for(OutputZone* outputZone : laser->outputZones) {
        if((outputZone->getZoneIndex() == zoneUi->inputZoneIndex) && (outputZone->getIsAlternate() == zoneUi->inputZoneAlt) && (outputZone->transformType == zoneType) ) {
            return outputZone;
        }
    }
    
    return nullptr;
}



bool LaserZoneViewController :: createZoneUiForOutputZone(OutputZone* outputZone) {
    
    ZoneUiBase* zoneUi = nullptr;
    if(outputZone->transformType == 0 )  {
        
        zoneUi = new ZoneUiQuad();
        zoneUi->updateFromOutputZone(outputZone);
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
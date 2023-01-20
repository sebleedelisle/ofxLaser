//
//  LaserZoneView.cpp
//  Created by Seb Lee-Delisle on 13/01/2023.
//
//

#include "ofxLaserZoneView.h"

using namespace ofxLaser;

LaserZoneView :: LaserZoneView(){
    laser = nullptr;
    
}
LaserZoneView :: LaserZoneView(Laser* newlaser){
    laser = newlaser;
    
}

LaserZoneView ::  ~LaserZoneView() {
    
    
}


void LaserZoneView :: draw() {
    
    
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
        
        ofPushMatrix();
        ofTranslate(offset);
        ofScale(scale, scale);
        
        drawLaserPath();
        
        zoneUiQuads.resize(laser->outputZones.size());
        
        // draw zone interfaces
        
        for(int i = 0; i<laser->outputZones.size(); i++) {
            // if the ui for this zone doesn't exist then make it
            OutputZone* outputZone = laser->outputZones[i];
            // if it's a quad zone
            if(outputZone->transformType==0) {
                ZoneUIQuad& zoneUiQuad = zoneUiQuads[i];
                vector<glm::vec2*> corners =outputZone->zoneTransformQuad.getCornerPoints();
                zoneUiQuad.setCorners(corners);
                zoneUiQuad.draw(); 
                
            }
 
        }
        
        
        
        ofPopMatrix();
        
        
        
        
 
    }
    
    
    
    
}


bool LaserZoneView :: updateZones()  {

    // create UI elements for all the zones
    
    for(OutputZone* outputZone : laser->outputZones) {
        
        
    } 
    
    return true;
    
}

void LaserZoneView :: setGrid(bool snaptogrid, int gridsize){

    snapToGrid = snaptogrid;
    gridSize = gridsize;
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

void LaserZoneView :: drawLaserPath() {
    
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


void LaserZoneView :: mouseMoved(ofMouseEventArgs &e){
    
    
}
bool LaserZoneView :: mousePressed(ofMouseEventArgs &e){
    
    // TODO figure out selected state
    ZoneUIQuad* selectedQuad = nullptr;
    for(ZoneUIQuad& zoneUiQuad : zoneUiQuads) {
       
        // zoneUiQuad needs to receieve a mousePressed event to handle
        // logic.
        // If the quad is selected, it'll need to check a hit with all it's dragpoints.
        // if one is hit then it starts the dragging process.
        // if none of the points are hit, a check for the shape is
        // executed. If true, then it starts dragging the whole shape.
        // If none of that is true and it's selected, deselect.
        

        if(zoneUiQuad.hitTest(e)) {
            // if the quad isn't selected, select it
            if(zoneUiQuad.getSelected()) {
                
                
            }
        }
        
    }
    
    return false;
}
void LaserZoneView :: mouseDragged(ofMouseEventArgs &e){
    for(ZoneUIQuad& zoneUiQuad : zoneUiQuads) {
        zoneUiQuad.mouseDragged(e);
        
    }
    
    
}
void LaserZoneView :: mouseReleased(ofMouseEventArgs &e) {
    for(ZoneUIQuad& zoneUiQuad : zoneUiQuads) {
        zoneUiQuad.mouseDragged(e);
        
    }
    
    
}

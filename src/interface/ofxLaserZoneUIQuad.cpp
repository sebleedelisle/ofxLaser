//
//  ofxLaserZoneUIQuad.cpp
//
//  Created by Seb Lee-Delisle on 14/01/2023.
//
//

#include "ofxLaserZoneUIQuad.h"

using namespace ofxLaser;


ZoneUIQuad :: ZoneUIQuad() {
    cornerHandles.resize(4);
    
    uiZoneFillColour  = ofColor::fromHex(0x001123, 128);
    uiZoneFillColourSelected = ofColor::fromHex(0x001123);
    uiZoneStrokeColour  = ofColor::fromHex(0x0E87E7);
    uiZoneStrokeColourSelected = ofColor::fromHex(0x0E87E7);
    uiZoneHandleColour = ofColor::fromHex(0x0E87E7);
    uiZoneHandleColourOver = ofColor :: fromHex(0xffffff);
    uiZoneStrokeSubdivisionColour = ofColor :: fromHex(0x00386D);;
    uiZoneStrokeSubdivisionColourSelected = ofColor :: fromHex(0x006ADB);
    
}

bool ZoneUIQuad :: setCorners(const vector<glm::vec2*>& points) {
    
    if(points.size()<4) return false;
    
    for(int i = 0; i<4; i++) {
        cornerHandles[i].set(*points[i]);
    }
    
    pointsUpdated();
    return true; 
    
}

void ZoneUIQuad :: draw() {
    
    ofPushStyle();
   
    ofSetColor(uiZoneStrokeColour);
    string label = "1";
    ofDrawBitmapString(label, centre - glm::vec3(4*label.size(),5, 0));
    
    ofSetColor(selected?uiZoneFillColourSelected : uiZoneFillColour);
    ofFill();
    zoneMesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
    zoneMesh.draw();

    ofNoFill();
    if(selected) {
        ofSetLineWidth(2);
    }
    ofSetColor(selected ? uiZoneStrokeColourSelected : uiZoneStrokeColour);
    zoneMesh.setMode(OF_PRIMITIVE_LINE_LOOP);
    zoneMesh.draw();
    
    for(DragHandle& handle : cornerHandles) {
        handle.draw();
    }
    
    ofPopStyle();

}

void ZoneUIQuad :: pointsUpdated() {
    
    centre = glm::vec2(0,0);
    for(DragHandle& handle : cornerHandles) {
        centre+=glm::vec2(handle);
    }
    centre/=4;
    
    zoneMesh.clear();
    zoneMesh.addVertex(glm::vec3(cornerHandles[0],0));
    zoneMesh.addVertex(glm::vec3(cornerHandles[1],0));
    zoneMesh.addVertex(glm::vec3(cornerHandles[3],0));
    zoneMesh.addVertex(glm::vec3(cornerHandles[2],0));
    
    zonePoly.clear();
    zonePoly.addVertex(glm::vec3(cornerHandles[0],0));
    zonePoly.addVertex(glm::vec3(cornerHandles[1],0));
    zonePoly.addVertex(glm::vec3(cornerHandles[3],0));
    zonePoly.addVertex(glm::vec3(cornerHandles[2],0));
    zonePoly.setClosed(true);

}

vector<DragHandle*> ZoneUIQuad :: getCornersClockwise() {

    vector<DragHandle*> corners;
    corners.push_back(&cornerHandles[0]);
    corners.push_back(&cornerHandles[1]);
    corners.push_back(&cornerHandles[3]);
    corners.push_back(&cornerHandles[2]);

    return corners;
    
    
}


void ZoneUIQuad :: mouseMoved(ofMouseEventArgs &e){
    
}
// TODO maybe we can handle selection at a higher level?
bool ZoneUIQuad :: mousePressed(ofMouseEventArgs &e) {
    mousePos = e;
    
    
     // zoneUiQuad needs to receieve a mousePressed event to handle
     // logic.
     // If the quad is selected, it'll need to check a hit with all it's dragpoints.
     // if one is hit then it starts the dragging process.
     // if none of the points are hit, a check for the shape is
     // executed. If true, then it starts dragging the whole shape.
     // If none of that is true and it's selected, deselect.
    if(getSelected()) {
        bool handlehit = false;
        // then check all the drag points
        vector<DragHandle*> corners = getCornersClockwise();
        for (int i = 0; i<corners.size() && !handlehit; i++) {
            
            
            if(corners[i]->hitTest(mousePos, scale)) {
                handlehit = true;
                corners[i]->startDrag(mousePos);
                // if we're not distorted then also start dragging the relavent corners
                if(isSquare()) {
                    
                    
                    
                }
            }
        }
    }
                
        
    
    bool hit = hitTest(mousePos);
    if((hit) &&(!selected)) {
        selected = true;
        return false; //  propogates
    }
    
    return false;
    
}
                
bool ZoneUIQuad :: isSquare() {
    
    vector<DragHandle>& corners = cornerHandles;
    return (corners[0].x == corners[2].x) && (corners[0].y == corners[1].y) && (corners[1].x == corners[3].x) && (corners[2].y == corners[3].y);
    
}
                
void ZoneUIQuad :: mouseDragged(ofMouseEventArgs &e){
    
    
}
void ZoneUIQuad :: mouseReleased(ofMouseEventArgs &e){
    
}


bool ZoneUIQuad :: hitTest(ofPoint mousePoint)  {
    ofLogNotice() << mousePoint;
    for(auto& vertex : zonePoly.getVertices()) {
        ofLogNotice() << vertex;
    }
    return zonePoly.inside(mousePoint);
    
}

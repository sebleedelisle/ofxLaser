//
//  ofxLaserZoneTransform.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransformQuad.h"


using namespace ofxLaser;

// static property / method



ZoneTransformQuad::ZoneTransformQuad() {
    initListeners();
    updateHandleColours();
    
    scale = 1;
    offset.set(0,0);
    
    editable = true;
    selected = false;
    visible = false;
    
    
     uiZoneFillColour  = ofColor::fromHex(0x001123, 128);
     uiZoneFillColourSelected = ofColor::fromHex(0x001123);
     uiZoneStrokeColour  = ofColor::fromHex(0x0E87E7);
     uiZoneStrokeColourSelected = ofColor::fromHex(0x0E87E7);
     uiZoneHandleColour = ofColor::fromHex(0x0E87E7);
     uiZoneHandleColourOver = ofColor :: fromHex(0xffffff);
     uiZoneStrokeSubdivisionColour = ofColor :: fromHex(0x00386D);;
     uiZoneStrokeSubdivisionColourSelected = ofColor :: fromHex(0x006ADB);;;
     snapToGrid = false;
     gridSize  = 0;
    
    
}


ZoneTransformQuad::~ZoneTransformQuad() {
    removeListeners();
}


void ZoneTransformQuad::init(ofRectangle& srcRect) {
    
    updateDivisions();
    updateQuads();
    
}

bool ZoneTransformQuad::update(){
    // check if laser is dirty and then update
//    updateHandleColours();
    return true;
}
    
    
    
void ZoneTransformQuad::draw() {
    
    if(!visible) return ;
    
    // read from laser eventually
    bool editSubdivisions = true;
    int xDivisions = 1;
    int yDivisions = 1;
    string label = "label";
    glm::vec3 centre;
    
    ofPushMatrix();
    ofTranslate(offset);
    ofScale(scale, scale);
    ofSetColor(selected?uiZoneFillColourSelected : uiZoneFillColour);
    // if invalid then change colours
//    if((!editSubdivisions) && useHomography && (!getIsConvex())) {
//        ofSetColor(80,0,0);
//    }
    
    // Draw the filled area within the zone segments
    ofFill();
    ofBeginShape();
    for(int i = 0; i<xDivisions; i++) {
        ofVertex(dstHandles[i]);
    }
    for(int i = 0; i<yDivisions; i++) {
        ofVertex(dstHandles[(i*(xDivisions+1))+xDivisions]);
    }
    for(int i = 0; i<xDivisions; i++) {
        int index = ((xDivisions+1) * (yDivisions+1) )-1-i;
        ofVertex(dstHandles[index]);
    }
    for(int i = 0; i<yDivisions; i++) {
        int index = (yDivisions-i)*(xDivisions+1);
        ofVertex(dstHandles[index]);
    }
    ofEndShape();
    ofNoFill();
    
    // draw the zone label
    ofSetColor(uiZoneStrokeColour);
    ofDrawBitmapString(label, centre - glm::vec3(4*label.size(),5, 0));
    
    
    // draw lines between all the vertices. Thick on outside, thin on the inside.
    for(size_t i= 0; i<dstHandles.size(); i++) {
        int x = i%(xDivisions+1);
        int y = (int)i/(xDivisions+1);
        
        if(x<xDivisions) {
            if((y>0)&&(y<yDivisions)) {
                // then it's an inner line
                if(editSubdivisions) {
                    ofSetColor(selected ? uiZoneStrokeSubdivisionColourSelected : uiZoneStrokeSubdivisionColour);
                    ofDrawLine(dstHandles[i], dstHandles[i+1]);
                }
            } else {
                
                if(selected) {
                    ofSetLineWidth(2);
                }
                ofSetColor(selected ? uiZoneStrokeColourSelected : uiZoneStrokeColour);
                ofDrawLine(dstHandles[i], dstHandles[i+1]);
                
                ofSetLineWidth(1);
            }
            
        }
        
        if(y<yDivisions) {
            if((x>0)&&(x<xDivisions)) {
                // then it's an inner line
                if(editSubdivisions) {
                    ofSetColor(selected ? uiZoneStrokeSubdivisionColourSelected : uiZoneStrokeSubdivisionColour);
                    ofDrawLine(dstHandles[i], dstHandles[i+xDivisions+1]);
                }
            } else {
                if(selected) {
                    ofSetLineWidth(2);
                }
                ofSetColor(selected ? uiZoneStrokeColourSelected : uiZoneStrokeColour);
                ofDrawLine(dstHandles[i], dstHandles[i+xDivisions+1]);
                
                ofSetLineWidth(1);
                
            }
            
        }
    }
    
    if(selected && editable) {
        for(size_t i = 0; i<dstHandles.size(); i++) {
            if((editSubdivisions) || (isCorner((int)i))) dstHandles[i].draw(mousePos,scale);
        }
    }
    ofPopMatrix();
}


void ZoneTransformQuad::initListeners() {
    
    ofAddListener(ofEvents().mouseMoved, this, &ZoneTransformQuad::mouseMoved, OF_EVENT_ORDER_AFTER_APP);
    ofAddListener(ofEvents().mousePressed, this, &ZoneTransformQuad::mousePressed, OF_EVENT_ORDER_AFTER_APP);
    ofAddListener(ofEvents().mouseReleased, this, &ZoneTransformQuad::mouseReleased, OF_EVENT_ORDER_AFTER_APP);
    ofAddListener(ofEvents().mouseDragged, this, &ZoneTransformQuad::mouseDragged, OF_EVENT_ORDER_AFTER_APP);
    
    
}

void ZoneTransformQuad :: removeListeners() {
    
    ofRemoveListener(ofEvents().mouseMoved, this, &ZoneTransformQuad::mouseMoved, OF_EVENT_ORDER_AFTER_APP);
    ofRemoveListener(ofEvents().mousePressed, this, &ZoneTransformQuad::mousePressed, OF_EVENT_ORDER_AFTER_APP);
    ofRemoveListener(ofEvents().mouseReleased, this, &ZoneTransformQuad::mouseReleased, OF_EVENT_ORDER_AFTER_APP);
    ofRemoveListener(ofEvents().mouseDragged, this, &ZoneTransformQuad::mouseDragged, OF_EVENT_ORDER_AFTER_APP);
    
}

void ZoneTransformQuad :: mouseMoved(ofMouseEventArgs &e){
    
    
//    if((!editable) || (!visible)) return;
//
    mousePos = e;
    mousePos-=offset;
    mousePos/=scale;
    //return false;
    
}

bool ZoneTransformQuad :: mousePressed(ofMouseEventArgs &e){

    // eventually read from laser object
    bool locked = false;
    bool editSubdivisions = true;
    bool isSquare = true;
    
    if((!editable) || (!visible)) return false;
   
    
    mousePos = e;
    mousePos-=offset;
    mousePos/=scale;
    
    
    bool hit = hitTest(mousePos);
    if((hit) &&(!selected)) {
        selected = true;
        return false; //  propogates
    }
    
    
    if(!selected) {
        return false; // propogates
    }
    
    bool handleHit = false;
    
    if(!locked) {
        // this section of code if we click drag anywhere in the zone
        
        for(int i= dstHandles.size()-1; i>=0 && !handleHit; i--) {
            
            if(dstHandles[i].hitTest(mousePos, scale)) {
                
                // ignore subdivisions if we're not editing them
                if(!editSubdivisions && !isCorner((int)i)) continue;
                
                dstHandles[i].startDrag(mousePos);
                handleHit = true;
                
                
                if(!editSubdivisions && (isSquare)) {
                    
                    DragHandle& currentHandle = dstHandles[i];
                    
                    // 1. we are dragging one of the points.
                    // 2. let's find the opposite one - that is the anchor
                    // 3. drag all the points relative to the anchor and the
                    //    current point being dragged
                    //    So, something like,
                    //    point.startDrag(clickpos, anchorpoint, relativetopoint, altpressed)
                    
                    vector<DragHandle*> corners;// = getCornerHandlesClockwise();
                    
                    int currenthandleindex = 0;
                    while(corners[currenthandleindex]!=&currentHandle) currenthandleindex++;
                    
                    ofLogNotice("Start drag index : " ) << currenthandleindex;
                    
                    DragHandle& anchorHandle = *corners[(currenthandleindex+2)%4];
                    DragHandle& dragHandle1 = *corners[(currenthandleindex+1)%4];
                    DragHandle& dragHandle2 = *corners[(currenthandleindex+3)%4];
                    
                    dragHandle1.startDragProportional(mousePos, anchorHandle, currentHandle, true);
                    dragHandle2.startDragProportional(mousePos, anchorHandle, currentHandle, true);

                    
                    
//                    vector<DragHandle*> corners;
//                    DragHandle& topLeft = dstHandles[0];
//                    DragHandle& topRight = dstHandles[xDivisions+1-1];
//                    DragHandle& bottomLeft = dstHandles[(yDivisions+1-1)*(xDivisions+1)];
//                    DragHandle& bottomRight = dstHandles.back();
//
//                    corners.push_back(&topLeft);
//                    corners.push_back(&topRight);
//                    corners.push_back(&bottomLeft);
//                    corners.push_back(&bottomRight);
//
//                    int handleIndex = 0;
//                    if(currentHandle == topLeft) handleIndex = 0;
//                    else if(currentHandle == topRight) handleIndex = 1;
//                    else if(currentHandle == bottomLeft) handleIndex = 2;
//                    else if(currentHandle == bottomRight) handleIndex =3;
//
//                    int x = ((handleIndex%2)+1)%2;
//                    int y = handleIndex/2;
//
//                    int xhandleindex = x+(y*2);
//
//                    x = handleIndex%2;
//                    y = ((handleIndex/2)+1)%2;
//                    int yhandleindex = x+(y*2);
//
//                    corners[xhandleindex]->startDrag(mousePos, false,true, true);
//                    corners[yhandleindex]->startDrag(mousePos, true,false, true);

                    //				bottomLeft.startDrag(mousePoint, false,true, true);
                    //				topRight.startDrag(mousePoint, true,false, true);
                    
                }
            }
            
        }
        
        // if the middle is clicked then drag all the handles!
        if(!handleHit && hit) {
            
            //centreHandle.startDrag(mousePoint);
            handleHit = true;
            DragHandle* gridHandle = &dstHandles[0];
            for(size_t i= 0; i<dstHandles.size(); i++) {
                if(&dstHandles[i] == gridHandle) {
                    dstHandles[i].startDrag(mousePos);
                } else {
                    dstHandles[i].startDrag(mousePos, gridHandle);
                }
            }
            
            
        }
    }
    if(!handleHit && !hit) {
        selected = false;
    }
    
    return false; // propogates, was : handleHit || hit;
    
}




void ZoneTransformQuad :: mouseDragged(ofMouseEventArgs &e){
    
    if((!editable) || (!visible)) return ;
    if(!selected) return ;
    
    glm::vec2 mousePoint;
    mousePoint.x = e.x;
    mousePoint.y = e.y;
    mousePoint-=offset;
    mousePoint/=scale;
    
    //ofRectangle bounds(centreHandle, 0, 0);
    int dragCount = 0;
    for(size_t i= 0; i<dstHandles.size(); i++) {
        if(dstHandles[i].updateDrag(mousePoint)) dragCount++;
    }

    // do we need to track dirtyness? Probably.
    isDirty |= (dragCount>0);

    
    
}


void ZoneTransformQuad :: mouseReleased(ofMouseEventArgs &e){
    
    //if(!editable) return false;
    if(!selected) return;
    
    bool wasDragging = false;
    
    for(size_t i= 0; i<dstHandles.size(); i++) {
        if(dstHandles[i].stopDrag()) wasDragging = true;
    }
    
    // TODO mark as dirty so auto save ********************
    //saveSettings();
    isDirty|=wasDragging;
    //return wasDragging;
    
}

bool ZoneTransformQuad::hitTest(ofPoint mousePoint) {
    // eventually read from laser object
    bool locked = false;
    bool editSubdivisions = true;
    bool isSquare = true;
    int xDivisions = 1;
    int yDivisions = 1;
    
    ofPolyline poly;
    for(int i = 0; i<=xDivisions; i++) {
        poly.addVertex(dstHandles[i].vec3());
        //ofLog(OF_LOG_NOTICE, ofToString(i));
    }
    //ofLog(OF_LOG_NOTICE, "---");
    for(int i = 2; i<=yDivisions; i++) {
        poly.addVertex(dstHandles[(i*(xDivisions+1))-1].vec3());
        //ofLog(OF_LOG_NOTICE, ofToString((i*(xDivisions+1))-1));
    }
    //ofLog(OF_LOG_NOTICE, "---");
    for(int i = ((xDivisions+1)*(yDivisions+1))-1; i>=(xDivisions+1)*(yDivisions); i--) {
        poly.addVertex(dstHandles[i].vec3());
        //ofLog(OF_LOG_NOTICE, ofToString(i));
    }
    //ofLog(OF_LOG_NOTICE, "---");
    for(int i = yDivisions-1; i>=0; i--) {
        poly.addVertex(dstHandles[(i*(xDivisions+1))].vec3());
        //ofLog(OF_LOG_NOTICE, ofToString((i*(xDivisions+1))));
    }
    
    if(poly.inside(mousePoint)) {
        return true;
    } else {
        ofPoint closest = poly.getClosestPoint(mousePoint);
        if(closest.distance(mousePoint)<(2.0f/scale)) {
            return true;
        } else {
            return false;
        }
    }
    //return poly.inside(mousePoint);
    
}
//
//void ZoneTransform::saveSettings() {
//
//	//ofLog(OF_LOG_NOTICE, "ZoneTransform::saveSettings");
//
//	ofJson json;
//	serialize(json);
//	ofSavePrettyJson(getSaveLabel()+".json", json);
//
//}
//

void ZoneTransformQuad::setHue(int hue) {

    uiZoneFillColour.setHue(hue);
    uiZoneFillColourSelected.setHue(hue);
    uiZoneStrokeColour.setHue(hue);
    uiZoneStrokeColourSelected.setHue(hue);
    uiZoneHandleColour.setHue(hue);
    uiZoneHandleColourOver.setHue(hue);
    uiZoneStrokeSubdivisionColour.setHue(hue);
    uiZoneStrokeSubdivisionColourSelected.setHue(hue);

    updateHandleColours();
  
}
void ZoneTransformQuad::updateHandleColours() {
    
    for(size_t i= 0; i<dstHandles.size(); i++) {
        dstHandles[i].setColour(uiZoneHandleColour, uiZoneHandleColourOver);
    }
    // topleft
    dstHandles[0].setColour(ofColor(180),ofColor(255));
}



bool ZoneTransformQuad :: setGrid(bool snapstate, int gridsize) {
    if((snapstate!=snapToGrid) || (gridSize!=gridsize)) {
        snapToGrid = snapstate;
        gridSize = gridsize;
        return true;
    } else {
        return false;
    }
}

void ZoneTransformQuad :: setScale(float _scale) {
scale = _scale;

}
void ZoneTransformQuad :: setOffset(ofPoint _offset) {
offset = _offset;
}


bool ZoneTransformQuad::getSelected() {
return selected;

};

bool ZoneTransformQuad::setSelected(bool v) {
if(selected!=v)  {
    selected = v;
   
    return true;
} else {
    return false;
}
};


void ZoneTransformQuad :: setEditable(bool warpvisible){
editable = warpvisible;
}
void ZoneTransformQuad :: setVisible(bool warpvisible){
visible = warpvisible;
}


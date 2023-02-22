//
//  ofxLaserViewPort.cpp
//
//
//  Created by Seb Lee-Delisle on 13/01/2023.
//
//

#include "ofxLaserScrollableView.h"

using namespace ofxLaser;

ScrollableView :: ScrollableView() {
    scale = 1;
    offset = glm::vec2(0,0);
    outputRect.set(0,0,800,800);
    sourceRect.set(0,0,800,800);
}

void ScrollableView :: drawFrame() {
    // draw frame
    ofPushStyle();
    ofNoFill();
    ofSetColor(30);
    ofDrawRectangle(outputRect);
    ofPopStyle();
    
    
}


void ScrollableView::beginViewPort() {
    
   
    
    ofPushView();
    ofViewport(outputRect);
    
    ofSetupScreen();
    
        
    
    ofPushMatrix();
    // offset for the viewport position
    ofTranslate(-outputRect.getTopLeft());

   


    ofTranslate(offset);
    ofScale(scale, scale);
    

    
    
    
    
}

void ScrollableView :: drawEdges() {
    
    
    // draw edges if visible
    ofPushMatrix();
    ofScale(1.0f/scale, 1.0f/scale);
    ofTranslate(-offset);
    ofTranslate(outputRect.getTopLeft());
    ofPushStyle();
    ofSetColor(0);
    ofFill();
    if(offset.x>0) {
        ofDrawRectangle(0, 0, offset.x-outputRect.getLeft(), outputRect.getHeight());
    }
   
    if(offset.y>0) {
        ofDrawRectangle(0, 0, outputRect.getWidth(), offset.y-outputRect.getTop());
    }
    
    float inputRight = offset.x+(sourceRect.getWidth()*scale)-outputRect.getLeft();
    if(inputRight<outputRect.getRight()) {
        ofDrawRectangle(inputRight, 0, outputRect.getRight()-inputRight, outputRect.getHeight());
    }
    
    float inputBottom = offset.y+(sourceRect.getHeight()*scale)-outputRect.getTop();
    if(inputBottom<outputRect.getBottom()) {
        ofDrawRectangle(0, inputBottom, outputRect.getWidth(), outputRect.getBottom()-inputBottom);
    }
    ofPopStyle();
    ofPopMatrix();
    
}
void ScrollableView::endViewPort() {
    ofPopMatrix();
    ofPopView();
    
}
bool ScrollableView::update(){
    
    return false;
}

bool ScrollableView::setIsVisible(bool visible) {
    if(visible!=isVisible) {
        isVisible = visible;
        return true;
    } else {
        return false;
    }
}
bool ScrollableView::getIsVisible(){
    return isVisible;
}

void ScrollableView::zoom(glm::vec2 anchor, float zoomMultiplier){
    
    glm::vec2 clickoffset = anchor-offset;
    clickoffset-=(clickoffset*zoomMultiplier);
    offset+=clickoffset;
    scale*=zoomMultiplier;
    
    
}
void ScrollableView::setSourceRect(ofRectangle rect) {
    sourceRect = rect;
}
void ScrollableView::setOutputRect(ofRectangle rect){
    outputRect = rect;
}

ofRectangle ScrollableView::getOutputRect() {
    return outputRect;
}
void ScrollableView::autoFitToOutput(){
    
    offset = outputRect.getTopLeft();
    scale =  MIN(outputRect.getWidth() / sourceRect.getWidth(), outputRect.getHeight() / sourceRect.getHeight());
    
}



void ScrollableView::setOffsetAndScale(glm::vec2 newoffset, float newscale){
    offset = newoffset;
    scale = newscale;
}

glm::vec2 ScrollableView::screenPosToLocalPos(glm::vec2 pos) {
    return (pos-offset)/scale;
}

ofMouseEventArgs ScrollableView::screenPosToLocalPos(ofMouseEventArgs e) {
    e-=offset;
    e/=scale;
    return e;
}
bool ScrollableView::mousePressed(ofMouseEventArgs &e){
    
    if(!getIsVisible()) return true;
    
    float clicktime = ofGetElapsedTimef();
    float clickinterval =clicktime-lastClickTime;
    lastClickTime =clicktime;
    if(hitTest(e)) {
        if(clickinterval < doubleClickMaxInterval) {
            cancelDrag();
            return mouseDoubleClicked(e);
        } else {
            
            startDrag(e);
            return false; // don't propagate
        }
    }
    return true; // propagate
};

bool ScrollableView::mouseDoubleClicked(ofMouseEventArgs &e){
    // hit test already done
    autoFitToOutput();
    return false; // don't propagate
    
}

bool ScrollableView :: cancelDrag() {
    if(!isDragging) {return false;
    } else {
        offset = dragStartPosition;
        isDragging = false;
        return true;
    }
}
void ScrollableView:: mouseMoved(ofMouseEventArgs &e){
    
}


void ScrollableView::mouseDragged(ofMouseEventArgs &e){
    if(!getIsVisible()) return;
    updateDrag(e);
};

void ScrollableView::mouseReleased(ofMouseEventArgs &e){
    stopDrag();
};

void ScrollableView::mouseScrolled(ofMouseEventArgs &e){
    if(!getIsVisible()) return ;
    
    if(hitTest(e)) {
        zoom(e, 1+(e.scrollY*zoomSpeed));
    }
};

bool ScrollableView :: hitTest(glm::vec2 screenpos) {
    return outputRect.inside(screenpos);
    
}
bool ScrollableView::startDrag(glm::vec2 mousepos){
    dragStartPosition = offset;
    dragOffset = mousepos-offset;
    isDragging = true;
    return true;
    
}
bool ScrollableView::updateDrag(glm::vec2 mousepos) {
    if(isDragging) {
        offset =  mousepos-dragOffset;
        return true;
    } else {
        return false;
    }
    
}

bool ScrollableView :: stopDrag(){
    if(isDragging) {
        isDragging = false;
        return true;
    } else {
        return false;
    }
}


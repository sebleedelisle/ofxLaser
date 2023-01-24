//
//  ofxLaserViewPort.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 13/01/2023.
//
//

#include "ofxLaserScrollableView.h"

using namespace ofxLaser;

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
void ScrollableView::autoFitToOutput(){
    offset = outputRect.getTopLeft();
    
    scale = outputRect.getWidth() / sourceRect.getWidth();
    scale = MIN(scale,outputRect.getHeight() / sourceRect.getHeight());
    
    
    
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
    
    // if(hitTest(e)) ...
    startDrag(e);
    return false;
};

void ScrollableView:: mouseMoved(ofMouseEventArgs &e){
  
   
    
}


void ScrollableView::mouseDragged(ofMouseEventArgs &e){
  
    updateDrag(e);
};

void ScrollableView::mouseReleased(ofMouseEventArgs &e){
    stopDrag();
};

void ScrollableView::mouseScrolled(ofMouseEventArgs &e){
   // ofLogNotice() << e.scrollX << " " << e.scrollY;
    //offset.x+=e.scrollX;
    //offset.y+=e.scrollY;
    if(hitTest(e)) {
        zoom(e, 1+(e.scrollY*zoomSpeed));
    }
};

bool ScrollableView :: hitTest(glm::vec2 screenpos) {
    return outputRect.inside(screenpos);
    
}
bool ScrollableView::startDrag(glm::vec2 mousepos){
    dragOffset = mousepos-offset;
    isDragging = true;
    
}
bool ScrollableView::updateDrag(glm::vec2 mousepos) {
    if(isDragging) {
        offset = mousepos-dragOffset;
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


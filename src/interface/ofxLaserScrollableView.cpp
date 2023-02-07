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

bool ScrollableView::update(){
  
    return false;
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


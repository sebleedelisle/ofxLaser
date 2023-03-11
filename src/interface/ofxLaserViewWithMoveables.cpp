//
//  ofxLaserViewWithMovables.cpp
//
//  Created by Seb Lee-Delisle on 11/02/2023.
//
//

#include "ofxLaserViewWithMoveables.h"

using namespace ofxLaser;
ViewWithMoveables :: ViewWithMoveables() {
    uiElementsEnabled = true;

    
    
}
bool ViewWithMoveables :: update() {
    
    //ScrollableView :: update();
   
    bool wasUpdated = false;
    

    boundingRect = sourceRect;
    for(MoveablePoly* uiElement : uiElements) {
        
        uiElement->setScale(scale);
        bool elementupdated =uiElement->update();
        wasUpdated|=elementupdated;
        
        boundingRect.growToInclude(uiElement->getBoundingBox());
    }
    checkEdges();
    
    return wasUpdated;
}
//

void ViewWithMoveables :: draw() {
    
    if(!getIsVisible()) return;
    
    drawFrame();
    beginViewPort();
    drawEdges();
    drawGrid();
    drawMoveables();
    endViewPort();
}

void ViewWithMoveables :: drawMoveables() {
    
    // draw all the UI elements
   
    for(MoveablePoly* uiElement: uiElementsSorted) {
        uiElement->draw();
        ofPushMatrix();
        ofTranslate(-outputRect.getTopLeft() / scale);
        uiElement->drawLabel();
        ofPopMatrix();
        
    }
    
}

void ViewWithMoveables :: drawGrid() {
    ofPushStyle();
    // draw the grid
    if(gridSize*scale<5) {
        ofSetColor(ofMap(gridSize*scale, 2, 5, 0,60, true));
    } else {
        ofSetColor(ofMap(gridSize*scale, 5, 100, 60,180, true));
    }
    ofDisableAntiAliasing();
    //ofPushMatrix();
    //ofTranslate(0.5f/scale, 0.5f/scale);
    gridMesh.draw();
    //ofPopMatrix();
    ofEnableAntiAliasing();
    ofPopStyle();
}


void ViewWithMoveables :: setGrid(bool snaptogrid, int gridsize){

    if((snapToGrid!=snaptogrid) || (gridSize!=gridsize)) {
        
        snapToGrid = snaptogrid;
        gridSize = gridsize;
        
        for(MoveablePoly* uiElement: uiElementsSorted) {
            uiElement->setGrid(snaptogrid, gridsize);
        }
            
        // update the grid mesh
        gridMesh.clear();
        int spacing = gridSize;
        while(spacing<5) spacing *=2;
        for(int x = 0; x<=800; x+=spacing) {
            for(int y = 0; y<=800; y+=spacing) {
                gridMesh.addVertex(ofPoint(MIN(800,x),MIN(800,y)));
            }
        }
        gridMesh.setMode(OF_PRIMITIVE_POINTS);
    }
}


void ViewWithMoveables :: deselectAllButThis(MoveablePoly* uielement) {
    
    // deselect all but the one we want
    for(MoveablePoly* uiElement2: uiElements) {
        if(uielement!=uiElement2) uiElement2->setSelected(false);
    }
    
    // move selected to back of array
    if(uielement!=nullptr) {
        uiElementsSorted.erase(std::remove(uiElementsSorted.begin(), uiElementsSorted.end(), uielement), uiElementsSorted.end());
        uiElementsSorted.push_back(uielement);
    }

}

void ViewWithMoveables :: deselectAll() {
    
    // deselect all but the one we want
    for(MoveablePoly* uiElement: uiElements) {
        uiElement->setSelected(false);
    }
    
}

void ViewWithMoveables :: mouseMoved(ofMouseEventArgs &e){
    ScrollableView :: mouseMoved(e);
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    for(MoveablePoly* uiElement: uiElements) {
        uiElement->mouseMoved(mouseEvent);
    }
    
}


bool ViewWithMoveables :: mousePressed(ofMouseEventArgs &e){

    
    bool propagate = true;

    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    if(uiElementsEnabled) {
        
        // also check for middle button because if ALT is pressed it's reported
        // as button 2 lol
        if((e.button == OF_MOUSE_BUTTON_LEFT)||(e.button == OF_MOUSE_BUTTON_MIDDLE)){
            
            for(int i = uiElementsSorted.size()-1; i>=0; i--) {
                
                MoveablePoly* uiElement = uiElementsSorted[i];
                
                propagate &= uiElement->mousePressed(mouseEvent);
                
                if(uiElement->getSelected() ) {
                    // deselect all but the one we want
                    deselectAllButThis(uiElement);
                    break;
                }
                
            }
        } else if(e.button == OF_MOUSE_BUTTON_RIGHT) {
            
            for(int i = uiElementsSorted.size()-1; i>=0; i--) {
                MoveablePoly* uiElement = uiElementsSorted[i];
                if(uiElement->hitTest(mouseEvent.x, mouseEvent.y)) {
                    uiElement->setSelected(true);
                    uiElement->setRightClickPressed();
                    deselectAllButThis(uiElement);
                    propagate = false;
                    break;
                }
                
            }
        }
        
    }
    
    // if no zones hit then check if the view should drag
    if(propagate) {
        propagate & ScrollableView::mousePressed(e);
    }
        
    
    
    return propagate;
}


void ViewWithMoveables :: mouseDragged(ofMouseEventArgs &e){
    
    ScrollableView::mouseDragged(e);

    if(uiElementsEnabled) {
        ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
       
        for(MoveablePoly* uiElement: uiElements) {
            uiElement->mouseDragged(mouseEvent);
        }
    }
}

void ViewWithMoveables :: mouseReleased(ofMouseEventArgs &e) {
    ScrollableView::mouseReleased(e);
    
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    for(MoveablePoly* uiElement: uiElements) {
        uiElement->mouseReleased(mouseEvent);
    }
}

bool ViewWithMoveables :: setUiElementsEnabled(bool enabled) {
    if(uiElementsEnabled!=enabled) {
        uiElementsEnabled = enabled;
        return true;
    } else {
        return false;
    }
    
}



void ViewWithMoveables :: setLockedAll(bool lockstate) {
    for(MoveablePoly* uiElement: uiElements) {
        uiElement->setDisabled(lockstate);
    }
    
}

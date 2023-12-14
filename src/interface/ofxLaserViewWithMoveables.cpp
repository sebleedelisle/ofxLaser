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
    
    bool wasUpdated = false;
    
    boundingRect = sourceRect;
    for(MoveablePoly* uiElement : uiElementsSorted) {
        
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
    if(!gridVisible) return; 
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


void ViewWithMoveables :: setGrid(bool snaptogrid, int gridsize, bool visible){

    if((snapToGrid!=snaptogrid) || (gridSize!=gridsize) || (gridVisible!=visible)) {
        
        snapToGrid = snaptogrid;
        gridSize = gridsize;
        gridVisible = visible;
        
        updateGrid();
     
    }
}

void ViewWithMoveables :: updateGrid() {
    for(MoveablePoly* uiElement: uiElementsSorted) {
        uiElement->setGrid(snapToGrid, gridSize);
    }
        
    // update the grid mesh
    gridMesh.clear();
    int spacing = gridSize;
    while(spacing<5) spacing *=2;
    for(int x = 0; x<=sourceRect.getWidth(); x+=spacing) {
        for(int y = 0; y<=sourceRect.getHeight(); y+=spacing) {
            gridMesh.addVertex(ofPoint(MIN(sourceRect.getWidth(),x),MIN(sourceRect.getHeight(),y)));
        }
    }
    gridMesh.setMode(OF_PRIMITIVE_POINTS);
}


void ViewWithMoveables :: deselectAllButThis(MoveablePoly* uiElementToKeepSelected) {
    
    // deselect all but the one we want
    for(MoveablePoly* uielement: uiElementsSorted) {
        if(uiElementToKeepSelected!=uielement) uielement->setSelected(false);
    }
    
    // move selected to back of array
    if(uiElementToKeepSelected!=nullptr) {
        //uiElementsSorted.erase(std::remove(uiElementsSorted.begin(), uiElementsSorted.end(), uiElementToKeepSelected), uiElementsSorted.end());
        //uiElementsSorted.push_back(uiElementToKeepSelected);
        
        auto it = find(uiElementsSorted.begin(), uiElementsSorted.end(), uiElementToKeepSelected);
        std::rotate(it, it + 1, uiElementsSorted.end());
    }

}

void ViewWithMoveables :: deselectAll() {
    
    // deselect all but the one we want
    for(MoveablePoly* uiElement: uiElementsSorted) {
        uiElement->setSelected(false);
    }
    
}

void ViewWithMoveables :: mouseMoved(ofMouseEventArgs &e){
    ScrollableView :: mouseMoved(e);
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    for(MoveablePoly* uiElement: uiElementsSorted) {
        uiElement->mouseMoved(mouseEvent);
    }
    
}


bool ViewWithMoveables :: mousePressed(ofMouseEventArgs &e){

    
    bool propagate = true;

    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    if(!getOutputRect().inside(e)) {
        return propagate;
    }
    
    if(uiElementsEnabled) {
        
        // also check for middle button because if ALT is pressed it's reported
        // as button 2 lol
        if((e.button == OF_MOUSE_BUTTON_LEFT)||(e.button == OF_MOUSE_BUTTON_MIDDLE)){
            
            for(int i = uiElementsSorted.size()-1; i>=0; i--) {
                
                MoveablePoly* uiElement = uiElementsSorted[i];
                
                propagate &= uiElement->mousePressed(mouseEvent);
                ofLogNotice(" ") << i  << " " << uiElement->getLabel() << " " << uiElement->getSelected();
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
       
        for(MoveablePoly* uiElement: uiElementsSorted) {
            uiElement->mouseDragged(mouseEvent);
        }
    }
}

void ViewWithMoveables :: mouseReleased(ofMouseEventArgs &e) {
    ScrollableView::mouseReleased(e);
    
    ofMouseEventArgs mouseEvent = screenPosToLocalPos(e);
    
    for(MoveablePoly* uiElement: uiElementsSorted) {
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
    for(MoveablePoly* uiElement: uiElementsSorted) {
        uiElement->setDisabled(lockstate);
    }
    
}



MoveablePoly* ViewWithMoveables :: getUiElementByUid(string _uid) {
    for(MoveablePoly* uiElement : uiElementsSorted) {
        if(uiElement->getUid() == _uid) {
            return uiElement;
        }
    }
    return nullptr;
    
    
    
}

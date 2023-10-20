//
//  ofxLaserShapeTarget.cpp
//
//  Created by Seb Lee-Delisle on 23/02/2023.
//
//

#include "ofxLaserShapeTarget.h"

using namespace ofxLaser;


ShapeTarget :: ShapeTarget (){
   boundsRect.set(0,0,800,800);
}

void ShapeTarget :: deleteShapes(){
        for(Shape* shape : shapes) {
            delete shape;
        }
        shapes.clear();
    
}
bool ShapeTarget :: addShape(Shape* shapetoadd){
    if(shapetoadd->intersectsRect(boundsRect)) {
        shapes.push_back(shapetoadd);
        return true;
    } else {
        // bit nasty
        delete shapetoadd;
        return false;
    }
    
}
bool ShapeTarget :: addShapes(vector<Shape*> shapestoadd){
    
    bool changed = false;
    for(Shape* shape : shapestoadd) {
        if(addShape(shape)) changed = true;
    }
    return changed; 
    
    
}
bool ShapeTarget :: setBounds(float x, float y, float w, float h){
    ofRectangle rect(x, y, w,h);
    return setBounds(rect);
}

bool ShapeTarget :: setBounds(ofRectangle& boundsrect){
    if(boundsRect!=boundsrect) {
        boundsRect = boundsrect;
        return true;
    } else {
        return false;
    }
    
}

ofRectangle ShapeTarget :: getBounds() {
    return boundsRect;
}

float ShapeTarget :: getWidth() {
    return boundsRect.getWidth();
}
float ShapeTarget :: getHeight() {
    return boundsRect.getHeight();
}

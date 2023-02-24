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

bool ShapeTarget :: deleteShapes(){
    for(Shape* shape : shapes) {
        delete shape;
    }
    shapes.clear(); 
}
bool ShapeTarget :: addShape(Shape* shapetoadd){
    if(shapetoadd->intersectsRect(boundsRect)) {
        shapes.push_back(shapetoadd);
    }
    
}
bool ShapeTarget :: setBounds(ofRectangle& boundsrect){
    if(boundsRect!=boundsrect) {
        boundsRect = boundsrect;
        return true;
    } else {
        return false;
    }
    
}

//
//  ofxLaserShapeTarget.h
//
//  Created by Seb Lee-Delisle on 23/02/2023.
//
//

#pragma once
#include "ofxLaserShape.h"
#include "ClipperUtils.h"

namespace ofxLaser {
class ShapeTarget {
    public :
    
    ShapeTarget();
    
    // converts shapes from 3D to 2D and
    // removes fills
    void processShapes();
    
    virtual void deleteShapes();
    virtual bool addShape(Shape* shapetoadd);
    bool addShapes(vector<Shape*> shapestoadd);

    virtual bool setBounds(ofRectangle& boundsrect);
    virtual bool setBounds(float x, float y, float w, float h);
    virtual ofRectangle getBounds();
    float getWidth();
    float getHeight();


    
    std::vector <ofxLaser::Shape*> shapes;

    protected:
    
    
    template<typename T>
    T convert3DTo2D(T p);
   
    template<typename T>
    T convert3DTo2D(T p, ofRectangle viewportrect, float fov = 550);
    
    ofRectangle boundsRect;

    
};
}

//
//  ofxLaserShapeTarget.h
//
//  Created by Seb Lee-Delisle on 23/02/2023.
//
//

#pragma once
#include "ofxLaserShape.h"

namespace ofxLaser {
class ShapeTarget {
    public :
    
    ShapeTarget();
    
    virtual bool deleteShapes();
    virtual bool addShape(Shape* shapetoadd);
    virtual bool setBounds(ofRectangle& boundsrect);
    virtual bool setBounds(float x, float y, float w, float h);
    float getWidth();
    float getHeight();
    
    
    std::deque <ofxLaser::Shape*> shapes;

    protected:
    ofRectangle boundsRect;

    
};
}

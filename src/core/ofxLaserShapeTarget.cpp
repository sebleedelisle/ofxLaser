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
bool ShapeTarget :: addShape(Shape* shapetoadd, bool useClipRectangle, ofRectangle clipRectangle){
    // removed! It doesn't work with 3D!
    //if(shapetoadd->intersectsRect(boundsRect)) {
        shapetoadd->setClipRectangle(boundsRect);
        if(useClipRectangle) {
            shapetoadd->setIntersectionClipRectangle(clipRectangle);
        }
        shapes.push_back(shapetoadd);
        return true;
   // } else {
        // bit nasty
   //     delete shapetoadd;
   //     return false;
   // }
    
}

bool ShapeTarget :: addShapes(vector<Shape*> shapestoadd){

    bool changed = false;
    for(Shape* shape : shapestoadd) {
        if(addShape(shape, true, shape->getClipRectangle())) changed = true;
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




void ShapeTarget ::  processShapes() {
    
    float fov = 550;
    
    //ofRectangle cliprect = getBounds();

    // TODO this should all be moved to the targets i think
    vector<ofxLaser::Shape*> emptyShapes;
    
    bool anyShapesFilled = false;
    for(ofxLaser::Shape* shape : shapes) {
        if(shape->isFilled()) {
            anyShapesFilled = true;
            break;
        }
    }
    
    
    // the shape containers are used to store individual polylines
    // that make up compound shapes.
    vector<vector<Shape*>> sortedShapeContainers;
    
    if(anyShapesFilled) {
        
        for(int i = 0; i<shapes.size(); i++) {
          
            Shape* shape = shapes[i];
            if(shape->id==-1) {
                sortedShapeContainers.push_back({shape});
            } else {
              
                bool found = false;
                
                for(vector<Shape*>& shapecontainer : sortedShapeContainers) {
                    
                    if(shapecontainer.front()->id==-1) continue;
                    //ofLogNotice("shapecontainer.front()->id : ") << shapecontainer.front()->id << " " << shape->id;
                    
                    if(shapecontainer.front()->id==shape->id) {
                        shapecontainer.push_back(shape);
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    sortedShapeContainers.push_back({shape});
                }
                
                
            }
            
            
        }
        
        std::sort(sortedShapeContainers.begin(), sortedShapeContainers.end(), [](const vector<ofxLaser::Shape*>& a, const vector<ofxLaser::Shape*>& b) -> bool {
           
            float d1 = a.front()->getMedianZDepth();
            float d2 = b.front()->getMedianZDepth();
            //ofLogNotice() << d1 << " " << d2;
            // don't sort if they're basically the same
            if(fabs(d1-d2)<0.001) return false;
            else return d1<d2;
        });
        
//        std::sort(shapes.begin(), shapes.end(), [](const ofxLaser::Shape* a, const ofxLaser::Shape* b) -> bool {
//            if((a->id!=-1) && (b->id!=-1) && (a->id==b->id)) return false;
//            float d1 = a->getMedianZDepth();
//            float d2 = b->getMedianZDepth();
//            //ofLogNotice() << d1 << " " << d2;
//            // don't sort if they're basically the same
//            if(fabs(d1-d2)<0.001) return false;
//            else return d1<d2;
//        });
        
    }
    
    for(Shape* shape : shapes) {
        
        // clip to near plane
        shape->clipNearPlane(fov-10);
        // convert 3D to 2D
        
        vector<glm::vec3>& points = shape->getPoints();
        for(glm::vec3& p : points) {
            p = convert3DTo2D(p);
        }
        shape->setDirty();

        // todo check to see if object is inside clipping rectangle?
        
        if(shape->isEmpty()) {
            emptyShapes.push_back(shape);
        }
        
    }
    
    // subtract shapes that are filled
    
    if(anyShapesFilled) {

        vector<Shape*> newShapes;
        ClipperLib::Paths clipperMaskPaths;
        
        // go from front to back
        for(int i = sortedShapeContainers.size()-1; i>=0; i--) {
            vector<Shape*>& shapeContainer = sortedShapeContainers[i];
            
            for(Shape* shape : shapeContainer)  {
                if(shape->isStroked() && (!shape->isEmpty())) {
                    // clip the shape to the current mask, if we have one
                    vector<Shape*> clippedShapes = ClipperUtils::clipShapeToMask(shape, clipperMaskPaths);
                    
                    newShapes.insert(newShapes.begin(), clippedShapes.begin(), clippedShapes.end());
                }
                    
            }
           
            // if this shape is filled, add it to the mask
            if(shapeContainer.front()->isFilled()) {
                ClipperUtils::addShapesToMasks(shapeContainer, clipperMaskPaths);

            }
                
        
        }
        
        deleteShapes();
        addShapes(newShapes);
        
        
    } else {
        
        shapes.erase(std::remove_if(shapes.begin(), shapes.end(),
            [](Shape* const& s) {
                return s->isEmpty(); // remove from vector if empty
            }), shapes.end());
        
        for(Shape* shape : emptyShapes) {
            delete shape;
        }
        
    }
    
    
    for(Shape* shape : shapes) {
        shape->clipToRectangle();
    }
    
//    shapes.erase(std::remove_if(shapes.begin(), shapes.end(),
//        [](Shape* const& s) {
//            return s->isEmpty(); // remove from vector if empty
//        }), shapes.end());
//
//    for(Shape* shape : emptyShapes) {
//        delete shape;
//
//    }
    
}


// converts 3D coords to screen coords //
template<typename T>
T ShapeTarget::convert3DTo2D(T p, ofRectangle viewportrect, float fov ) {
    
    T p1 = p; // getTransformed(p);

    if(p1.z==0) return p1;
  
    float scale = fov/(-p1.z+fov);
    p1.z = 0;
    p1-=viewportrect.getCenter();
    p1*=scale;
    p1+=viewportrect.getCenter();

    return p1;
 
}
template<typename T>
T ShapeTarget::convert3DTo2D(T p) {
    return convert3DTo2D(p, getBounds());
    
}

//
//  ofxLaserZoneTransformQuad.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransformBase.h"


using namespace ofxLaser;



ZoneTransformBase:: ZoneTransformBase(){
    
    isDirty = true;
   
    transformParams.setName("ZoneTransformBaseParams");
    
    //transformParams.add(locked.set("locked", false));
    srcRect.set(0,0,400,400); 
    boundaryRect.set(0,0,800,800); // this is the zone size
    
    
}
ZoneTransformBase:: ~ZoneTransformBase(){
    
    
}

bool ZoneTransformBase :: update(){
    return false;
};

void ZoneTransformBase:: updateSrc(const ofRectangle& rect) {
    
    if(srcRect != rect) {
        //ofLogNotice("ZoneTransformBase:: setSrc ") << rect;
        srcRect = rect;
        isDirty = true;
    }
};





void ZoneTransformBase :: setDirty(bool state) {
    isDirty = state;
    boundingBoxDirty = boundingBoxDirty | state;
    
};
bool ZoneTransformBase ::getIsDirty() {
    return isDirty;
}
vector<glm::vec2> ZoneTransformBase::getPerimeterPoints() {
    vector<glm::vec2> points;
    getPerimeterPoints(points);
    return points;
}



ofRectangle ZoneTransformBase:: getBoundingBox() {
    if(!boundingBoxDirty) return boundingBox;
    vector<glm::vec2> points = getPerimeterPoints();
    if(points.size()==0) {
        boundingBox.set(0,0,0,0);
    } else {
        boundingBox.set(points[0], 0,0);
        for(glm::vec2& p : points) {
            
            boundingBox.growToInclude(p);
        }
    }
    
    boundingBoxDirty = false;
    return boundingBox;
    
    
}



glm::vec2 ZoneTransformBase:: getVectorToBringWithinBoundingBox() {
        
    ofRectangle boundingbox = getBoundingBox();
    if(boundaryRect.inside(boundingbox)) return glm::vec2(0,0);
    
    glm::vec2 delta;
    
    if(boundingbox.getLeft()>boundaryRect.getRight()) {
        delta.x = boundaryRect.getRight() - boundingBox.getLeft();
    } else if(boundingbox.getRight()<boundaryRect.getLeft()) {
        delta.x = boundaryRect.getLeft() - boundingBox.getRight();
    }
    if(boundingbox.getTop()>boundaryRect.getBottom()) {
        delta.y = boundaryRect.getBottom() - boundingBox.getTop();
    } else if(boundingbox.getBottom()<boundaryRect.getTop()) {
        delta.y = boundaryRect.getTop() - boundingBox.getBottom();
    }
    return delta;
    
}

//
//  ofxLaserPolygon.cpp
//
//
//  Created by Seb Lee-Delisle on 04/02/2023.
//
//

#include "ofxLaserPolygonBase.h"

using namespace ofxLaser;

void PolygonBase::serialize(ofJson&json) const {
    ofJson& pointsjson = json["points"];
    for(int i = 0; i<size(); i++) {
        const glm::vec2& pos = at(i);
        pointsjson.push_back({pos.x, pos.y});
    }

}

bool PolygonBase::deserialize(ofJson& jsonGroup) {
    
    ofJson& pointsjson = jsonGroup["points"];
    if(pointsjson.size()>=3) {
        
        resize(pointsjson.size());
    
        for(int i = 0; i<size(); i++) {
            ofJson& point = pointsjson[i];
      
            at(i).x = point[0];
            at(i).y = point[1];
        }
        isDirty = true;
        return true;
    } else {
        return false;
    }
}

bool PolygonBase::update() {
    
    if(isDirty) {
        isDirty = false;
        updateBounds();
        return true;
    } else {
        return false;
    }
}

void PolygonBase :: updateBounds() {
    if(size()==0) return;
    boundsTopLeft = at(0);
    boundsBottomRight = at(0);
    
    for(glm::vec2& point :*this) {
        boundsTopLeft.x = MIN(boundsTopLeft.x, point.x);
        boundsTopLeft.y = MIN(boundsTopLeft.y, point.y);
        boundsBottomRight.x = MAX(boundsBottomRight.x, point.x);
        boundsBottomRight.y = MAX(boundsBottomRight.y, point.y);
    }
    
    
}

ofRectangle PolygonBase::getBoundingBox() {
    return ofRectangle(boundsTopLeft, boundsBottomRight);
}

void PolygonBase::setFromPoints(const vector<glm::vec2*>& newpoints) {
    
    resize(newpoints.size());
    
    for(size_t i = 0 ;i<newpoints.size(); i++) {
        if(at(i)!=*newpoints[i]) {
            at(i) = *newpoints[i];
            isDirty = true;
        } 
 
    }
    // huh. This should be auto updated in update
   // if(isDirty) updateBounds();
    
    
}

void PolygonBase::setFromPoints(const vector<glm::vec2>& newpoints) {
    
    resize(newpoints.size());
    
    for(size_t i = 0 ;i<newpoints.size(); i++) {
        if(at(i)!=newpoints[i]) {
            at(i) = newpoints[i];
            isDirty = true;
        }
 
    }
    // huh. This should be auto updated in update
   // if(isDirty) updateBounds();
    
    
}

void PolygonBase::setRectangle(float x, float y, float w, float h){
    resize(4);
    at(0) = {x, y};
    at(1) = {x+w, y};
    at(2) = {x+w, y+h};
    at(3) = {x, y+h};
    isDirty = true; 

}


bool PolygonBase::hitTest(glm::vec2& p) {
    return hitTest(p.x, p.y);
}


bool PolygonBase::hitTest(float x, float y) {
    if(isDirty) update();
    
    bool boundingboxhit = x>boundsTopLeft.x && x< boundsBottomRight.x && y>boundsTopLeft.y && y<boundsBottomRight.y;
    if(!boundingboxhit) {
        return false;
    }
    
    return GeomUtils::pointInPoly(glm::vec2(x,y), *this, boundsTopLeft.x);
    
}

bool PolygonBase::hitTestEdges(glm::vec2& p, float lineWidth) {
    return hitTestEdges(p.x, p.y, lineWidth);
}

bool PolygonBase::hitTestEdges(float x, float y, float lineWidth) {
    if(hitTest(x, y) ) return true;
    
    glm::vec2 pos(x,y);
    
    for(int i = 0; i<size(); i++) {
        glm::vec2& p1 = at(i);
        glm::vec2& p2 = at((i+1) % size());
        if(GeomUtils::pointDistanceFromLine(pos, p1, p2) < lineWidth) return true;
    }
    return false;
}



bool PolygonBase::isAxisAligned() {
//
//    if(size()!=4) return false;
//
//    float epsilon = 0.001f;
//
//    return (fabs(glm::dot(at(0) - at(3), at(1)- at(0)))<epsilon) && (fabs(glm::dot(at(2) - at(1), at(3)- at(2)))<epsilon);
//
    
    return GeomUtils::isPerpendicularQuad(*this); 
}



//
//  ofxLaserPolygon.cpp
//  example_HelloLaser
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
    bool boundingboxhit = x>boundsTopLeft.x && x< boundsBottomRight.x && y>boundsTopLeft.y && y<boundsBottomRight.y;
    if(!boundingboxhit) return false;
    
    return GeomUtils::pointInPoly(glm::vec2(x,y), *this);
    
}

bool PolygonBase::isSquare() {
    float epsilon = 0.001f;
    return (fabs(at(0).x - at(3).x)<epsilon) &&
        (fabs(at(0).y - at(1).y)<epsilon) &&
        (fabs(at(1).x - at(2).x)<epsilon) &&
        (fabs(at(3).y - at(2).y)<epsilon);
    
}



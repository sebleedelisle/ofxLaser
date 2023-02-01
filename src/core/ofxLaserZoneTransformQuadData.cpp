//
//  ofxLaserZoneTransform.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransformQuadData.h"


using namespace ofxLaser;


ZoneTransformQuadData::ZoneTransformQuadData() {
   
    
    dstPoints.resize(4);
    srcPoints.resize(4);
    
    // Used for serialize / deserialize
    transformParams.setName("ZoneTransformParams");

    transformParams.add(useHomography.set("perspective", false));
    
    updateSrc(ofRectangle(0,0,100,100));
    setDst(ofRectangle(100,100,200,200));
        
    ofAddListener(transformParams.parameterChangedE(), this, &ZoneTransformQuadData::paramChanged);
    
}

void ZoneTransformQuadData :: paramChanged(ofAbstractParameter& e) {
    isDirty= true;
    
}
ZoneTransformQuadData::~ZoneTransformQuadData() {
   
    ofRemoveListener(transformParams.parameterChangedE(), this, &ZoneTransformQuadData::paramChanged);
}


void ZoneTransformQuadData::init(ofRectangle& srcRect) {
    
    updateSrc(srcRect);
    
     ofRectangle destRect(600,600,100,100) ;
  
    setDst(destRect);
    
    
    updateQuads();
    
}


bool ZoneTransformQuadData::update(){
    if(isDirty) {
        //ofLogNotice("ZoneTransform::update() - isDirty");
        updateQuads();
        //updateHandleColours();
        updateConvex();
        isDirty = false;
        return true;
    } else {
        return false;
    }
    
    
}


glm::vec2 ZoneTransformQuadData::getCentre() {
    ofPoint centre = srcRect.getCenter();
    centre = getWarpedPoint(centre);
    return glm::vec2(centre.x, centre.y);
}

void ZoneTransformQuadData :: resetToSquare() {
    if(locked) return;
    vector<glm::vec2*> cornerhandles = getCornerPoints();
    vector<glm::vec2> corners;
    // convert to ofPoints
    for(glm::vec2* handle : cornerhandles) corners.push_back(*handle);
    
    corners[0].x = corners[2].x = getLeft();
    corners[0].y = corners[1].y = getTop(); //ofLerp(corners[0].y,corners[1].y, 0.5);
    corners[1].x = corners[3].x = getRight(); // ofLerp(corners[1].x,corners[3].x,0.5);
    corners[3].y = corners[2].y = getBottom(); // ofLerp(corners[3].y,corners[2].y,0.5);
    setDstCorners(corners[0], corners[1], corners[2], corners[3]);
    isDirty = true;
}

bool ZoneTransformQuadData :: isSquare() {

    vector<glm::vec2*> corners = getCornerPoints();
    
//
//    ofLogNotice() << *corners[0] << " " << *corners[1] << " " << *corners[2] << " " << *corners[3];
    
    return (fabs(corners[0]->x - corners[2]->x)<0.01f) &&
        (fabs(corners[0]->y - corners[1]->y)<0.01f) &&
        (fabs(corners[1]->x - corners[3]->x)<0.01f) &&
        (fabs(corners[2]->y - corners[3]->y)<0.01f);

    
}

ofPoint ZoneTransformQuadData::getWarpedPoint(const ofPoint& p){
    
    return quadWarper.getWarpedPoint(p, useHomography);
    
};

ofPoint ZoneTransformQuadData::getUnWarpedPoint(const ofPoint& p){
    //ofPoint rp = p - srcRect.getTopLeft();
    
    return quadWarper.getUnWarpedPoint(p);
    
};


ofxLaser::Point ZoneTransformQuadData::getWarpedPoint(const ofxLaser::Point& p){
    
 
    return  quadWarper.getWarpedPoint(p, useHomography);

    
};


void ZoneTransformQuadData::updateSrc(const ofRectangle& rect) {
    
    if(srcRect!=rect) {
        //ofLogNotice("ZoneTransform:: setSrc ") << rect;
        srcRect = rect;
            
        srcPoints[0] = srcRect.getTopLeft();
        srcPoints[1] = srcRect.getTopRight();
        srcPoints[2] = srcRect.getBottomLeft();
        srcPoints[3] = srcRect.getBottomRight();

        isDirty = true;
    }
}
void ZoneTransformQuadData::setDst(const ofRectangle& rect) {
    setDstCorners(rect.getTopLeft(), rect.getTopRight(), rect.getBottomLeft(), rect.getBottomRight());
    updateQuads();
}

void ZoneTransformQuadData :: setDstCorners(glm::vec2 topleft, glm::vec2 topright, glm::vec2 bottomleft, glm::vec2 bottomright) {
 
    
    vector<glm::vec2*> points = getCornerPointsClockwise();
    vector<glm::vec2*> newpoints = {&topleft, &topright, &bottomright, &bottomleft};

 
    bool pointschanged = false;
    // fix concave!
 
    for(size_t i = 0; i<4; i++) {
        
        if(*points[i]!=*newpoints[i]) {
            pointschanged = true;
            
//            // clamp between points to avoid concave shapes
//            if(!isSquare()) {
//                clampToVector(*newpoints[i], *points[(i+3)%4], *points[(i+1)%4]);
//                // clamp between edges to avoid over-extension
//                clampToVector(*newpoints[i], *points[(i+3)%4], *points[(i+2)%4]);
//                clampToVector(*newpoints[i], *points[(i+2)%4], *points[(i+1)%4]);
//            } else {
//   
//                
//            }
           
            
        }
    }
    
    if(pointschanged) {
        for(size_t i = 0; i<4; i++) {
            *points[i]=*newpoints[i];
        }
        
    }
    isDirty|=pointschanged;
 
}

bool ZoneTransformQuadData :: moveHandle(int handleindex, glm::vec2 newpos, bool lockSquare) {
    
    int i = handleindex;
    vector<glm::vec2*> points = getCornerPointsClockwise();
    bool pointchanged = false;
    if(*points[i]!=newpos) {
        
        
        // clamp between points to avoid concave shapes
        if(!lockSquare) {
            clampToVector(newpos, *points[(i+3)%4], *points[(i+1)%4]);
            // clamp between edges to avoid over-extension
            clampToVector(newpos, *points[(i+3)%4], *points[(i+2)%4]);
            clampToVector(newpos, *points[(i+2)%4], *points[(i+1)%4]);
            
            *points[i] = newpos;
            
        } else {
            // constrained version
        
            // vectors 90º apart based around the point
            glm::vec2 v1(10,0);
            glm::vec2 v2(0,10);
            v1 = glm::rotate(v1, float(PI/2*i));
            v2 = glm::rotate(v2, float(PI/2*i));
            
            glm::vec2& pointbefore = *points[(i+3)%4];
            glm::vec2& pointafter = *points[(i+1)%4];
            glm::vec2 edgebefore = pointbefore+v1;
            glm::vec2 edgeafter = pointafter-v2;

            // stop the point from crossing inside out
            clampToVector(newpos, pointbefore, edgebefore );
            clampToVector(newpos, pointafter, edgeafter);
            *points[i] = newpos;
            
            v1+=pointbefore;
            v2+=pointafter;

            // point before needs to clamp onto its own edge
            pointbefore = getClampedToVector(newpos, pointbefore, v1, true, true);
            pointafter = getClampedToVector(newpos, pointafter, v2, true, true);
            
            if((pointbefore.x!=newpos.x) || (pointafter.y!=newpos.y)) {
                std::setprecision(10);
                std::cout << pointbefore.x << " " << newpos.x << " " << pointafter.y << " " << newpos.y<< endl ;
            }
            
            
        }
        pointchanged = (*points[i]==newpos);
        *points[i]=newpos; 
        
    }
    isDirty|=pointchanged;
    
    return pointchanged;
    
    
}

bool ZoneTransformQuadData :: clampToVector(glm::vec2& pointToClamp, glm::vec2& p1, glm::vec2&p2){
    
    
    glm::vec2 clamped = getClampedToVector(pointToClamp, p1, p2, true, false);
    if(clamped!=pointToClamp) {
        pointToClamp = clamped;
        return true;
    } else {
        return false;
    }
//
//    glm::vec2 v1 = p2-p1;
//    v1 = glm::rotate(v1, ofDegToRad(-90));
//    glm::vec2 v2 = pointToClamp - p1;
//
//    float d = glm::dot(v2, v1);
//
//    if(d<0) {
//        glm::vec2 normal = glm::normalize(v1);
//        d = glm::dot(v2, normal);
//        pointToClamp-=(normal*d);
//        return true;
//
//
//
//    } else {
//        return false;
//    }
    
}

glm::vec2 ZoneTransformQuadData :: getClampedToVector(glm::vec2& source, glm::vec2& p1, glm::vec2&p2, bool clampinside, bool clampoutside){
    
    //glm::vec2 returnvector;
    
    glm::vec2 v1 = p2-p1;
    v1 = glm::rotate(v1, ofDegToRad(-90));
    glm::vec2 v2 = source - p1;
    
    float d = glm::dot(v2, v1);
    
    // if it's too short then we don't have a valid vector to project onto
    if(glm::length(v1)<0.01) {
        ofLogNotice() << v1 << " " << v2;
        return p1;
    }
    
    if(((d<0) && (clampinside) ) || ((d>0) && (clampoutside))) {
        glm::vec2 normal = glm::normalize(v1);
        d = glm::dot(v2, normal);
        
        return source-(normal*d);

    } else {
        return source;
    }
    
}

bool ZoneTransformQuadData :: isWindingClockwise(){
    
    vector<glm::vec2*> corners = getCornerPointsClockwise();
    float sum = 0;
    for(int i = 0; i<4; i++) {
        const glm::vec2& p1 = *corners[(i+1)%4];
        const glm::vec2& p2 = *corners[i];
        glm::vec2 edge(p2.x-p1.x, p2.y+p2.y);
        sum+=(edge.x*edge.y);
    }
    return sum>=0;
    
}
    

void ZoneTransformQuadData::resetFromCorners() {
    vector<glm::vec2> corners = getCorners();
    setDstCorners(corners[0], corners[1], corners[2], corners[3]);
    
}

vector<glm::vec2*> ZoneTransformQuadData::getCornerPoints(){
    vector<glm::vec2*> corners;
    corners.push_back(&dstPoints[0]);
    corners.push_back(&dstPoints[1]);
    corners.push_back(&dstPoints[2]);
    corners.push_back(&dstPoints[3]);
    return corners;
}

vector<glm::vec2> ZoneTransformQuadData::getCorners(){
    
    vector<glm::vec2> corners;
    
    corners.push_back(dstPoints[0]);
    corners.push_back(dstPoints[1]);
    corners.push_back(dstPoints[2]);
    corners.push_back(dstPoints[3]);
    return corners;
    
}

vector<glm::vec2*> ZoneTransformQuadData::getCornerPointsClockwise(){
    vector<glm::vec2*> corners;
    
    corners.push_back(&dstPoints[0]);
    corners.push_back(&dstPoints[1]);
    corners.push_back(&dstPoints[3]);
    corners.push_back(&dstPoints[2]);
    
    
    
    return corners;
}

void ZoneTransformQuadData::getPerimeterPoints(vector<glm::vec2>& points) {
    points.clear();
    points.push_back(dstPoints[0]);
    points.push_back(dstPoints[1]);
    points.push_back(dstPoints[3]);
    points.push_back(dstPoints[2]);

}



bool ZoneTransformQuadData :: isCorner(int i ) {
    return true;
    
}




void ZoneTransformQuadData::updateQuads() {
   

    quadWarper.updateHomography(srcPoints[0],
                              srcPoints[1],
                              srcPoints[2],
                              srcPoints[3],
                              dstPoints[0],
                              dstPoints[1],
                              dstPoints[2],
                              dstPoints[3]
                              );
    
   
    
    
}





bool ZoneTransformQuadData::serialize(ofJson&json) {
    ofSerialize(json, transformParams);
    ofJson& handlesjson = json["handles"];
    for(size_t i= 0; i<dstPoints.size(); i++) {
        glm::vec2& pos = dstPoints[i];
        handlesjson.push_back({pos.x, pos.y});
    }
    
    return true;
}

bool ZoneTransformQuadData::deserialize(ofJson& jsonGroup) {
    //ofLogNotice("ZoneTransform::deserialize()");
    // note that ofDeserialize looks for the json group
    // with the same name as the parameterGroup
    ofDeserialize(jsonGroup, transformParams);
    
    
    ofJson& handlejson = jsonGroup["handles"];
    //cout << handlejson.dump(3) << endl;
    if((int)handlejson.size()>=4) {
        for(int i = 0; i<4; i++) {
            ofJson& point = handlejson[i];
            dstPoints[i].x = point[0];
            dstPoints[i].y = point[1];
           
            
        }
    }
    return true;
}


float ZoneTransformQuadData::getRight() {
    float right = 0;
    for(glm::vec2& handle : dstPoints) {
        if(handle.x>right) right = handle.x;
    }
    
    return right;
}
float ZoneTransformQuadData::getLeft() {
    float left = 800;
    for(glm::vec2& handle : dstPoints) {
        if(handle.x<left) left = handle.x;
    }
    
    return left;
}

float ZoneTransformQuadData::getTop() {
    float top = 800;
    for(glm::vec2& handle : dstPoints) {
        if(handle.y<top) top = handle.y;
    }
    
    return top;
}
float ZoneTransformQuadData::getBottom() {
    float bottom = 0;
    for(glm::vec2& handle : dstPoints) {
        if(handle.y>bottom) bottom = handle.y;
    }
    
    return bottom;
}



bool ZoneTransformQuadData :: getIsConvex() {
    return isConvex;
    
}

void ZoneTransformQuadData :: updateConvex() {
    bool convex = true;
    vector<glm::vec2> corners = getCorners();
    vector<glm::vec2> points;
    points.push_back(corners[0]);
    points.push_back(corners[1]);
    points.push_back(corners[3]);
    points.push_back(corners[2]);
    for(size_t i = 0; i<4; i++) {
    
        ofVec2f p1 = points[i%4];
        ofVec2f p2 = points[(i+1)%4];
        ofVec2f p3 = points[(i+2)%4];
        ofVec2f v1 = p2-p1;
        ofVec2f v2 = p3-p2;
        v2.rotate(90);
        if(v2.dot(v1)>0) convex = false;
    }
    
    if(convex) {
        for(int i =0; i<3; i++) {
            for(int j = i+1; j<4; j++) {
                if(points[i] == points[j]) {
                    convex = false;
                }
            }
        }
    }
        
    
    isConvex =  convex;
    
}


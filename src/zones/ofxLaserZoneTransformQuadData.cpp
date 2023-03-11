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


void ZoneTransformQuadData::init() {
    
    ofRectangle destRect(300,300,200,200) ;
  
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

bool ZoneTransformQuadData :: isAxisAligned() {
    
    vector<glm::vec2> points;
    getPerimeterPoints(points);
    return GeomUtils::isPerpendicularQuad(points);
    
}

ofPoint ZoneTransformQuadData::getWarpedPoint(const ofPoint& p){
    
    return quadWarper.getWarpedPoint(p, useHomography&&!isConvex);
    
};

ofPoint ZoneTransformQuadData::getUnWarpedPoint(const ofPoint& p){
    //ofPoint rp = p - srcRect.getTopLeft();
    
    return quadWarper.getUnWarpedPoint(p);
    
};


ofxLaser::Point ZoneTransformQuadData::getWarpedPoint(const ofxLaser::Point& p){
    
 
    return  quadWarper.getWarpedPoint(p, useHomography&&!isConvex);

    
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
//            if(!isAxisAligned()) {
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
            // clamp to vector between neighbours
            GeomUtils::clampToVector(newpos, *points[(i+3)%4], *points[(i+1)%4], true, false);
            
            glm::vec2& pointbefore = *points[(i+3)%4];
            glm::vec2& pointopposite = *points[(i+2)%4];
            glm::vec2& pointafter = *points[(i+1)%4];
            
            // clamp between edges to avoid over-extension
            // this mess of code calculates a vector between the adjacent two points,
            // but then rotates it so that the moving point cannot get quite colinear
            float minangle = 2.0f;
            glm::vec2 beforeedge =  pointbefore - pointopposite;
            beforeedge = glm::rotate(beforeedge, float(minangle*PI/180.0f));
            GeomUtils::clampToVector(newpos,  pointopposite+beforeedge, pointopposite, true, false);
            

            glm::vec2 afteredge = pointafter - pointopposite;
            afteredge = glm::rotate(afteredge, float(-minangle*PI/180.0f)); // rotate it one degree
            GeomUtils::clampToVector(newpos,  pointafter, pointafter+afteredge, true, false);
            
            
            
            
            *points[i] = newpos;
            
        } else {
            // constrained version
            float minsize = 2;
            
            glm::vec2& pointbefore = *points[(i+3)%4];
            glm::vec2& pointopposite = *points[(i+2)%4];
            glm::vec2& pointafter = *points[(i+1)%4];
            
            // vectors 90º apart based around the opposite point
            glm::vec2 v1 = glm::normalize(pointopposite - pointbefore)* minsize;
            glm::vec2 v2 = glm::normalize(pointopposite - pointafter) * minsize;
            
            // adding v1 and v2 stops the square getting too small
            glm::vec2 edgebefore = pointbefore+v1;
            glm::vec2 edgeafter = pointafter-v2;

            // stop the point from crossing inside out
            GeomUtils::clampToVector(newpos, pointbefore-v2, edgebefore-v2, true, false);
            GeomUtils::clampToVector(newpos, pointafter-v1, edgeafter-v1, true, false);
            
            
            *points[i] = newpos;
            
            v1+=pointbefore;
            v2+=pointafter;

            // point before needs to clamp onto its own edge
            pointbefore = GeomUtils::getClampedToVector(newpos, pointbefore, v1, true, true);
            pointafter = GeomUtils::getClampedToVector(newpos, pointafter, v2, true, true);
            
            
            
        }
        pointchanged = (*points[i]==newpos);
        *points[i]=newpos; 
        
    }
    isDirty|=pointchanged;
    
    return pointchanged;
    
    
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

void ZoneTransformQuadData::serialize(ofJson&json) const {
    ofSerialize(json, transformParams);
    ofJson& handlesjson = json["handles"];
    for(size_t i= 0; i<dstPoints.size(); i++) {
        const glm::vec2& pos = dstPoints[i];
        handlesjson.push_back({pos.x, pos.y});
    }
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

    vector<glm::vec2*> pointsclockwise = getCornerPointsClockwise();
    isConvex =  GeomUtils::isConvex(pointsclockwise);
    
}


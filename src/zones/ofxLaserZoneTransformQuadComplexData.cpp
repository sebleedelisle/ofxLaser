//
//  ofxLaserZoneTransform.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransformQuadComplexData.h"


using namespace ofxLaser;


ZoneTransformQuadComplexData::ZoneTransformQuadComplexData() {
   
    //ofLogNotice("ZoneTransformQuadComplexData::ZoneTransformQuadComplexData()");
    dstPoints.resize(4);
    srcPoints.resize(4);
    subdivisionLevel = 1;
    
    // Used for serialize / deserialize
    transformParams.setName("ZoneTransformParams");

//    transformParams.add(useHomography.set("perspective", false));
    
    updateSrc(ofRectangle(0,0,100,100));
    resetDst(ofRectangle(100,100,200,200));
        
    ofAddListener(transformParams.parameterChangedE(), this, &ZoneTransformQuadComplexData::paramChanged);
    
}

void ZoneTransformQuadComplexData :: paramChanged(ofAbstractParameter& e) {
    isDirty= true;
    
}
ZoneTransformQuadComplexData::~ZoneTransformQuadComplexData() {
    ofRemoveListener(transformParams.parameterChangedE(), this, &ZoneTransformQuadComplexData::paramChanged);
}


void ZoneTransformQuadComplexData::init() {
    
    setDefault();

    
}


bool ZoneTransformQuadComplexData::update(){
    
    if(isDirty) {
        //ofLogNotice("ZoneTransform::update() - isDirty");
        //updateQuads();
        //updateHandleColours();
        //updateConvex();
        isDirty = false;
        return true;
    } else {
        return false;
    }
    
}


glm::vec2 ZoneTransformQuadComplexData::getCentre() {
    ofPoint centre = srcRect.getCenter();
    centre = getWarpedPoint(centre);
    return glm::vec2(centre.x, centre.y);
}



ofPoint ZoneTransformQuadComplexData::getWarpedPoint(const ofPoint& p){
    
    ofPoint rp = p - srcRect.getTopLeft();
    int xDivisions = getNumSubdivisions();
    int yDivisions = getNumSubdivisions();
    
    int segx = (rp.x / srcRect.getWidth()) * (float)(xDivisions);
    int segy = (rp.y / srcRect.getHeight()) * (float)(yDivisions);
    
    //ofLog(OF_LOG_NOTICE, ofToString(x) + " " + ofToString(y));
    
    segx = ofClamp(segx,0,xDivisions-1);
    segy = ofClamp(segy,0,yDivisions-1);
    glm::vec2 topleft = getPointForPosition(segx, segy);
    glm::vec2 topright = getPointForPosition(segx+1, segy);
    glm::vec2 botleft = getPointForPosition(segx, segy+1);
    glm::vec2 botright = getPointForPosition(segx+1, segy+1);
    
    glm::vec2 sourcesegmenttopleft = glm::vec2(srcRect.x + (srcRect.getWidth()/xDivisions*segx), srcRect.y + (srcRect.getHeight()/yDivisions*segy));
    glm::vec2 sourcesegmentbotright = glm::vec2(srcRect.x + (srcRect.getWidth()/xDivisions*(segx+1)), srcRect.y + (srcRect.getHeight()/yDivisions*(segy+1)));
    
    //glm::vec2 relativepoint = glm::vec2(p.x, p.y)-sourcesegmenttopleft;
    
    glm::vec2 d = sourcesegmentbotright - sourcesegmenttopleft;
    float u = (p.x-(sourcesegmenttopleft.x))/d.x;
    float v = (p.y-(sourcesegmenttopleft.y))/d.y;
    glm::vec2& A = topleft;
    glm::vec2& B = topright;
    glm::vec2& C = botright;
    glm::vec2& D = botleft;
    
    return A + (B-A)*u + (D-A)*v + (A-B+C-D)*u*v;
                                                     
};

ofPoint ZoneTransformQuadComplexData::getUnWarpedPoint(const ofPoint& p){
    //ofPoint rp = p - srcRect.getTopLeft();
    
    return p;//quadWarper.getUnWarpedPoint(p);
    
};


ofxLaser::Point ZoneTransformQuadComplexData::getWarpedPoint(const ofxLaser::Point& p){
    
    ofPoint cvp = ofPoint(p.x, p.y);
    cvp = getWarpedPoint(cvp);
    ofxLaser::Point lp = p;
    lp.x = cvp.x;
    lp.y = cvp.y;
    
    return lp;

    
};


void ZoneTransformQuadComplexData::updateSrc(const ofRectangle& rect) {
    
    if(srcRect!=rect) {
        //ofLogNotice("ZoneTransform:: setSrc ") << rect;
        srcRect = rect;
            
        srcPoints[0] = srcRect.getTopLeft();
        srcPoints[1] = srcRect.getTopRight();
        srcPoints[2] = srcRect.getBottomLeft();
        srcPoints[3] = srcRect.getBottomRight();
        //updateQuads();
        //updateConvex();

        //isDirty = true;
    }
}


void ZoneTransformQuadComplexData::resetDst(const ofRectangle& rect) {
    resetDst(rect.getTopLeft(), rect.getTopRight(), rect.getBottomLeft(), rect.getBottomRight());
}

void ZoneTransformQuadComplexData::resetDst(glm::vec2 topleft, glm::vec2 topright, glm::vec2 bottomleft, glm::vec2 bottomright) {
    
    int subdivisionsX = getNumSubdivisions();
    int subdivisionsY = getNumSubdivisions();
    
    vector<glm::vec2> newpoints;
    for(int y = 0; y<=subdivisionsY; y++) {
        
        glm::vec2 left = glm::mix(topleft, bottomleft, (float)y/subdivisionsY);
        glm::vec2 right = glm::mix(topright, bottomright, (float)y/subdivisionsY);
        
        for(int x = 0; x<=subdivisionsX; x++) {
            newpoints.push_back(glm::mix(left, right, (float)x/subdivisionsX));
            
        }
    }
    dstPoints = newpoints;
    isDirty = true;
 
}

void ZoneTransformQuadComplexData :: updatePoints(vector<glm::vec2*> points) {

    for(int i = 0; i<points.size(); i++) {
        dstPoints[i] = *points[i];
    }
    //
//    for(glm::vec2& p : dstPoints) {
//        p+=dragoffset;
//        
//    }
    isDirty = true;
    
}

bool ZoneTransformQuadComplexData :: moveHandle(int handleindex, glm::vec2 newpos, bool lockSquare) {
    
    //int i = handleindex;
    
    if(dstPoints[handleindex]!=newpos) {
        dstPoints[handleindex]=newpos;
        isDirty = true;
        return true;
        
    } else  {
        return false;
    }
    
}
  

void ZoneTransformQuadComplexData::getPerimeterPoints(vector<glm::vec2>& points) {
    points.clear();

    int subdivisionsX = getNumSubdivisions();
    int subdivisionsY = getNumSubdivisions();
    
    for(int i = 0; i<=subdivisionsX; i++) {
        points.push_back(dstPoints[i]);

    }

    for(int i = 2; i<=subdivisionsY; i++) {
        points.push_back(dstPoints[(i*(subdivisionsX+1))-1]);

    }

    for(int i = ((subdivisionsX+1)*(subdivisionsY+1))-1; i>=(subdivisionsX+1)*(subdivisionsY); i--) {
        points.push_back(dstPoints[i]);
       
    }

    for(int i = subdivisionsY-1; i>=0; i--) {
        points.push_back(dstPoints[(i*(subdivisionsX+1))]);
       
    }
    
}



vector<glm::vec2> ZoneTransformQuadComplexData::getPerimeterPoints() {
    vector<glm::vec2> points;
    getPerimeterPoints(points);
    return points;
}

//
//bool ZoneTransformQuadComplexData :: isCorner(int i ) {
//    return true;
//}
//
//void ZoneTransformQuadComplexData::updateQuads() {
//
//
//
////
////    quadWarper.updateHomography(srcPoints[0],
////                              srcPoints[1],
////                              srcPoints[2],
////                              srcPoints[3],
////                              dstPoints[0],
////                              dstPoints[1],
////                              dstPoints[2],
////                              dstPoints[3]
////                              );
//
//}

void ZoneTransformQuadComplexData :: setDefault() {
    setSubdivisionLevel(1);
    resetDst(ofRectangle(192,192,416,224));
}

bool ZoneTransformQuadComplexData :: setSubdivisionLevel(int newlevel) {
    if(newlevel!=subdivisionLevel) {
        while(newlevel>subdivisionLevel) incSubdivisionLevel();
        while(newlevel<subdivisionLevel) decSubdivisionLevel(); 
        return true;
        
    } else {
        return false;
    }
    
    
}

void ZoneTransformQuadComplexData :: incSubdivisionLevel() {
    
    vector<glm::vec2> newpoints;
    
    int subdivisionsX = getNumSubdivisions();
    int subdivisionsY = getNumSubdivisions();
    
    for(int y = 0; y<=subdivisionsY; y++) {
        // top row
        for(int x = 0; x<subdivisionsX; x++) {
            glm::vec2 topleft = getPointForPosition(x, y);
            glm::vec2 topright = getPointForPosition(x+1, y);
            // add top left point
            newpoints.push_back(topleft);
            newpoints.push_back(glm::mix(topleft, topright, 0.5));
        }
        // now add right most point in that row
        newpoints.push_back(getPointForPosition(subdivisionsX, y));
        
        // now, if we're not at the bottom, add the middle row
        if(y<subdivisionsY) {
            for(int x = 0; x<=subdivisionsX; x++) {
                glm::vec2 topleft = getPointForPosition(x, y);
                glm::vec2 botleft = getPointForPosition(x, y+1);
                glm::vec2 midleft = glm::mix(topleft, botleft, 0.5);
                newpoints.push_back(midleft);
                
                if(x<subdivisionsX) {
                    glm::vec2 topright = getPointForPosition(x+1, y);
                    glm::vec2 botright = getPointForPosition(x+1, y+1);
                    glm::vec2 midright = glm::mix(topright, botright, 0.5);
                 
                    newpoints.push_back(glm::mix(midleft, midright, 0.5));
                }
            }
        }
    }
    
    dstPoints = newpoints;
    subdivisionLevel++;
}

void ZoneTransformQuadComplexData :: decSubdivisionLevel(){
    if(subdivisionLevel==0) return;
    vector<glm::vec2> newpoints;
    
    int subdivisionsX = getNumSubdivisions();
    int subdivisionsY = getNumSubdivisions();

    for(int y=0; y<=subdivisionsY; y++) {
        for(int x = 0; x<=subdivisionsX; x++) {
            if((y%2==0) && (x%2==0)) {
                newpoints.push_back(getPointForPosition(x,y));
            }
        }
    }
    
    dstPoints = newpoints;
    subdivisionLevel--;
    
}


glm::vec2& ZoneTransformQuadComplexData :: getPointForPosition(int x, int y) {
    return dstPoints.at(getPointIndexForPosition(x,y));
    
    
}

int ZoneTransformQuadComplexData :: getPointIndexForPosition(int x, int y) {
    return (y*(getNumSubdivisions()+1)) +x;
    
    
}

int ZoneTransformQuadComplexData :: getNumSubdivisions() {
    if(subdivisionLevel==0) return 1;
    else return  pow(2, subdivisionLevel);
}



glm::vec2 ZoneTransformQuadComplexData :: getDestPointAt(int i) {
    return dstPoints.at(i);
    
}
int ZoneTransformQuadComplexData :: getNumPoints() {

    return dstPoints.size();
} 

void ZoneTransformQuadComplexData::serialize(ofJson&json) const {
    ofSerialize(json, transformParams);
    ofJson& handlesjson = json["handles"];
    for(size_t i= 0; i<dstPoints.size(); i++) {
        const glm::vec2& pos = dstPoints[i];
        handlesjson.push_back({pos.x, pos.y});
    }
    json["subdivisionlevel"] = subdivisionLevel;
}


bool ZoneTransformQuadComplexData::deserialize(ofJson& jsonGroup) {
    //ofLogNotice("ZoneTransform::deserialize()");
    // note that ofDeserialize looks for the json group
    // with the same name as the parameterGroup
    ofDeserialize(jsonGroup, transformParams);
    
    if(jsonGroup.contains("subdivisionlevel")) {
        subdivisionLevel = jsonGroup["subdivisionlevel"].get<int>();
    }
    
    int numpoints = (getNumSubdivisions()+1) * (getNumSubdivisions()+1);
    ofJson& handlejson = jsonGroup["handles"];
    dstPoints.resize(handlejson.size());
    //cout << handlejson.dump(3) << endl;
    if((int)handlejson.size()>=numpoints) {
        for(int i = 0; i<numpoints; i++) {
            ofJson& point = handlejson[i];
            dstPoints[i].x = point[0];
            dstPoints[i].y = point[1];
           
        }
    }
    isDirty = true;
    return true;
}
//
//
//float ZoneTransformQuadComplexData::getRight() {
//    float right = 0;
//    for(glm::vec2& handle : dstPoints) {
//        if(handle.x>right) right = handle.x;
//    }
//
//    return right;
//}
//float ZoneTransformQuadComplexData::getLeft() {
//    float left = 800;
//    for(glm::vec2& handle : dstPoints) {
//        if(handle.x<left) left = handle.x;
//    }
//
//    return left;
//}
//
//float ZoneTransformQuadComplexData::getTop() {
//    float top = 800;
//    for(glm::vec2& handle : dstPoints) {
//        if(handle.y<top) top = handle.y;
//    }
//
//    return top;
//}
//float ZoneTransformQuadComplexData::getBottom() {
//    float bottom = 0;
//    for(glm::vec2& handle : dstPoints) {
//        if(handle.y>bottom) bottom = handle.y;
//    }
//
//    return bottom;
//}
//


//bool ZoneTransformQuadComplexData :: getIsConvex() {
//    return isConvex;
//
//}
//
//void ZoneTransformQuadComplexData :: updateConvex() {
//
//    vector<glm::vec2*> pointsclockwise = getCornerPointsClockwise();
//    isConvex =  GeomUtils::isConvex(pointsclockwise);
//
//}

//
//void ZoneTransformQuadComplexData :: resetToSquare() {
//    if(locked) return;
//    vector<glm::vec2*> cornerhandles = getCornerPoints();
//    vector<glm::vec2> corners;
//    // convert to ofPoints
//    for(glm::vec2* handle : cornerhandles) corners.push_back(*handle);
//
//    corners[0].x = corners[2].x = getLeft();
//    corners[0].y = corners[1].y = getTop(); //ofLerp(corners[0].y,corners[1].y, 0.5);
//    corners[1].x = corners[3].x = getRight(); // ofLerp(corners[1].x,corners[3].x,0.5);
//    corners[3].y = corners[2].y = getBottom(); // ofLerp(corners[3].y,corners[2].y,0.5);
//    setDstCorners(corners[0], corners[1], corners[2], corners[3]);
//    isDirty = true;
//}
//
//bool ZoneTransformQuadComplexData :: isAxisAligned() {
//
//    vector<glm::vec2> points;
//    getPerimeterPoints(points);
//    return GeomUtils::isPerpendicularQuad(points);
//
//}

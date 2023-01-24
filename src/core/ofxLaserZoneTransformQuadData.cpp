//
//  ofxLaserZoneTransform.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransformQuadData.h"


using namespace ofxLaser;

// static property / method



ZoneTransformQuadData::ZoneTransformQuadData() {
   
    
    dstPoints.resize(4);
    srcPoints.resize(4);
    editSubdivisions = false;
    
//    gridParams.add(snapToGrid.set("snap to grid", true));
//    gridParams.add(gridSize.set("grid size", 10));
//    ofAddListener(gridParams.parameterChangedE(), this, &ZoneTransform::paramChanged);
//
    
    // Used for serialize / deserialize
    transformParams.setName("ZoneTransformParams");
    
    transformParams.add(editSubdivisions.set("edit subdivisions", false));
    transformParams.add(xDivisionsNew.set("x divisions", 1,1,6));
    transformParams.add(yDivisionsNew.set("y divisions", 1,1,6));
    transformParams.add(useHomography.set("perspective", false));
    
    xDivisions = 1;
    yDivisions = 1;
    updateSrc(ofRectangle(0,0,100,100));
    setDst(ofRectangle(100,100,200,200));
    
    xDivisionsNew.addListener(this, &ZoneTransformQuadData::divisionsChanged);
    yDivisionsNew.addListener(this, &ZoneTransformQuadData::divisionsChanged);
    
    ofAddListener(transformParams.parameterChangedE(), this, &ZoneTransformQuadData::paramChanged);
    
}

void ZoneTransformQuadData :: paramChanged(ofAbstractParameter& e) {
    isDirty= true;
    
}
ZoneTransformQuadData::~ZoneTransformQuadData() {
   
    xDivisionsNew.removeListener(this, &ZoneTransformQuadData::divisionsChanged);
    yDivisionsNew.removeListener(this, &ZoneTransformQuadData::divisionsChanged);
    ofRemoveListener(transformParams.parameterChangedE(), this, &ZoneTransformQuadData::paramChanged);
}


void ZoneTransformQuadData::init(ofRectangle& srcRect) {
    
//    float srcwidth = srcRect.getWidth();
//    float srcheight = srcRect.getHeight();
//
    updateSrc(srcRect);
    
    // TODO - better default???
    
    ofRectangle destRect(600,600,100,100) ;
    
    //= srcRect;
    //destRect.scale(srcwidth/800, srcheight/800);
    //destRect.x*=srcwidth/800;
    //destRect.y*=srcheight/800;
    setDst(destRect);
    //ofLogNotice("ZoneTransform::init - setDst");
    updateDivisions();
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
    return (corners[0]->x == corners[2]->x) && (corners[0]->y == corners[1]->y) && (corners[1]->x == corners[3]->x) && (corners[2]->y == corners[3]->y);
    
}

ofPoint ZoneTransformQuadData::getWarpedPoint(const ofPoint& p){
    
    //if(useHomography && (!isConvex())) return dstHandles[0];
    
    ofPoint rp = p - srcRect.getTopLeft();
    
    int x = (rp.x / srcRect.getWidth()) * (float)(xDivisions);
    int y = (rp.y / srcRect.getHeight()) * (float)(yDivisions);
    
    //ofLog(OF_LOG_NOTICE, ofToString(x) + " " + ofToString(y));
    
    x = ofClamp(x,0,xDivisions-1);
    y = ofClamp(y,0,yDivisions-1);
    
    int quadnum = x + (y*xDivisions);
    Warper & quad = quadWarpers[quadnum];
    return quad.getWarpedPoint(p, (!editSubdivisions) && useHomography);
    
};

ofPoint ZoneTransformQuadData::getUnWarpedPoint(const ofPoint& p){
    ofPoint rp = p - srcRect.getTopLeft();
    
    int x = (rp.x / srcRect.getWidth()) * (float)(xDivisions);
    int y = (rp.y / srcRect.getHeight()) * (float)(yDivisions);
    
    //ofLog(OF_LOG_NOTICE, ofToString(x) + " " + ofToString(y));
    
    x = ofClamp(x,0,xDivisions-1);
    y = ofClamp(y,0,yDivisions-1);
    
    int quadnum = x + (y*xDivisions);
    Warper & quad = quadWarpers[quadnum];
    return quad.getUnWarpedPoint(p);
    
};


ofxLaser::Point ZoneTransformQuadData::getWarpedPoint(const ofxLaser::Point& p){
    
    ofxLaser::Point rp = p;
    
    if((!editSubdivisions) && useHomography && (!getIsConvex())) {
        rp.set(dstPoints[0]);
        rp.setColour(0,0,0);
        return rp;
    }
        
    rp.x-=srcRect.getTopLeft().x;
    rp.y-=srcRect.getTopLeft().y;

    int x = (rp.x / srcRect.getWidth()) * (float)(xDivisions);
    int y = (rp.y / srcRect.getHeight()) * (float)(yDivisions);
    
    //ofLog(OF_LOG_NOTICE, ofToString(x) + " " + ofToString(y));
    
    x = ofClamp(x,0,xDivisions-1);
    y = ofClamp(y,0,yDivisions-1);
    
    int quadnum = x + (y*xDivisions);
    Warper & quad = quadWarpers[quadnum];
    rp = quad.getWarpedPoint(p, (!editSubdivisions) && useHomography);
    
    return rp;
    
};
//
//Point getUnWarpedPoint(const Point& p){
//    return p;
//};
void ZoneTransformQuadData::updateSrc(const ofRectangle& rect) {
    
    
    if((srcRect!=rect) || (srcPoints.size()!=((xDivisions+1)*(yDivisions+1)))) {
        //ofLogNotice("ZoneTransform:: setSrc ") << rect;
        srcRect = rect;
        // update source points?
        int xpoints = xDivisions+1;
        int ypoints = yDivisions+1;
        
        int numpoints = xpoints*ypoints;
        
        // srcPoints should already have enough but let's check
        if((int)srcPoints.size()!= numpoints) {
            srcPoints.resize(numpoints);
        }
        
        for(int i= 0; i<numpoints; i++) {
            float x = ofMap(i%xpoints, 0, xDivisions, rect.getLeft(), rect.getRight());
            float y = ofMap(i/xpoints, 0, yDivisions, rect.getTop(), rect.getBottom());
            
            //ofLog(OF_LOG_NOTICE, ofToString(x) + " " +ofToString(y));
            
            srcPoints[i] = glm::vec3(x, y,0);
            
        }
        isDirty = true;
    }
}
void ZoneTransformQuadData::setDst(const ofRectangle& rect) {
    setDstCorners(rect.getTopLeft(), rect.getTopRight(), rect.getBottomLeft(), rect.getBottomRight());
    updateQuads();
}

void ZoneTransformQuadData :: setDstCorners(glm::vec2 topleft, glm::vec2 topright, glm::vec2 bottomleft, glm::vec2 bottomright) {
    // interpolate dst handle points?
    
    // ofLog(OF_LOG_NOTICE, "ZoneTransform::setDstCorners "+displayLabel);
    vector<cv::Point2f> srcCVPoints, dstCVPoints;
    srcCVPoints.resize(4);
    dstCVPoints.resize(4);
    
    srcCVPoints[0] = toCv(srcRect.getTopLeft());
    srcCVPoints[1] = toCv(srcRect.getTopRight());
    srcCVPoints[2] = toCv(srcRect.getBottomLeft());
    srcCVPoints[3] = toCv(srcRect.getBottomRight());
    
    //cout << topleft << " " << topright << " " << bottomleft << " " << bottomright << endl;
    
    dstCVPoints[0] = toCv(topleft);
    dstCVPoints[1] = toCv(topright);
    dstCVPoints[2] = toCv(bottomleft);
    dstCVPoints[3] = toCv(bottomright);
    
    // NB 8 is CV_RANSAC which isn't defined in all platforms for some reason.
    cv::Mat homography = cv::findHomography(cv::Mat(srcCVPoints), cv::Mat(dstCVPoints),8, 100);
    
    srcCVPoints.resize(srcPoints.size());
    dstCVPoints.resize(srcPoints.size());
    
    
    for(size_t i= 0; i<srcPoints.size(); i++) {
        srcCVPoints[i] = toCv(srcPoints[i]);
        
    }
    try {
        cv::perspectiveTransform(srcCVPoints, dstCVPoints, homography);
    } catch ( cv::Exception & e ) {
        ofLog(OF_LOG_ERROR, e.msg ); // output exception message
    }
    
    for(size_t i= 0; i<dstPoints.size(); i++) {
        dstPoints[i] = (toOf(dstCVPoints[i]));
    }
    isDirty = true;
}

void ZoneTransformQuadData::resetFromCorners() {
    vector<glm::vec2> corners = getCorners();
    setDstCorners(corners[0], corners[1], corners[2], corners[3]);
    
}

vector<glm::vec2*> ZoneTransformQuadData::getCornerPoints(){
    vector<glm::vec2*> corners;
    corners.push_back(&dstPoints[0]);
    corners.push_back(&dstPoints[xDivisions]);
    corners.push_back(&dstPoints[yDivisions*(xDivisions+1)]);
    corners.push_back(&dstPoints[((xDivisions+1)*(yDivisions+1))-1]);
    return corners;
}

vector<glm::vec2> ZoneTransformQuadData::getCorners(){
    
    vector<glm::vec2> corners;
    
    int indextopleft = 0 ;
    int indextopright = xDivisions;
    int indexbotleft =yDivisions*(xDivisions+1);
    int indexbotright =((xDivisions+1)*(yDivisions+1))-1;
    
    corners.push_back(dstPoints[indextopleft]);
    corners.push_back(dstPoints[indextopright]);
    corners.push_back(dstPoints[indexbotleft]);
    corners.push_back(dstPoints[indexbotright]);
    return corners;
    
}

vector<glm::vec2*> ZoneTransformQuadData::getCornerPointsClockwise(){
    vector<glm::vec2*> corners;
    
    int indextopleft = 0 ;
    int indextopright = xDivisions;
    int indexbotleft =yDivisions*(xDivisions+1);
    int indexbotright =((xDivisions+1)*(yDivisions+1))-1;
    
    corners.push_back(&dstPoints[indextopleft]);
    corners.push_back(&dstPoints[indextopright]);
    corners.push_back(&dstPoints[indexbotright]);
    corners.push_back(&dstPoints[indexbotleft]);
    return corners;
}

void ZoneTransformQuadData::getPerimeterPoints(vector<glm::vec2>& points) {
    points.clear();
    
    for(int i = 0; i<xDivisions; i++) {
        points.push_back(dstPoints[i]);
    }
    for(int i = xDivisions; i<(((xDivisions+1)*(yDivisions+1))-1); i+=(xDivisions+1) ) {
        points.push_back(dstPoints[i]);
    }
    int start =((xDivisions+1)*(yDivisions+1))-1;
    int end = yDivisions*(xDivisions+1);
    for(int i =start ; i>end; i--){
        points.push_back(dstPoints[i]);
    }
    start = yDivisions*(xDivisions+1);
    end = 0;
    for(int i =start ; i>=end; i-=(xDivisions+1)){
        points.push_back(dstPoints[i]);
    }
    //    points.push_back(dstHandles[yDivisions*(xDivisions+1)]);
    //
}



bool ZoneTransformQuadData :: isCorner(int i ) {
    return (i==0) || (i == xDivisions) || (i == yDivisions*(xDivisions+1)) || (i==((xDivisions+1)*(yDivisions+1))-1);
    
}

void ZoneTransformQuadData :: setDivisions(int xdivisions, int ydivisions) {
    xDivisionsNew = xdivisions;
    yDivisionsNew = ydivisions;
    
    
    updateDivisions();
    
}

void ZoneTransformQuadData:: divisionsChanged(int& e){
    if((xDivisionsNew!=xDivisions) || (yDivisionsNew!=yDivisions))
        updateDivisions();
}

void ZoneTransformQuadData:: updateDivisions(){
    //ofLogNotice("ZoneTransform::updateDivisions()");
    
    //ofLog(OF_LOG_NOTICE, "divisionsChanged");
    
    vector<glm::vec2> corners  = getCorners();
    
    xDivisions = xDivisionsNew;
    yDivisions = yDivisionsNew;
    dstPoints.resize((xDivisions+1)*(yDivisions+1));
    // srcpoints is resized in setSrc
    //srcPoints.resize((xDivisions+1)*(yDivisions+1));
    
    updateSrc(srcRect);
    
        
    setDstCorners(corners[0], corners[1], corners[2], corners[3]);
    
    updateQuads();
    
    
}


void ZoneTransformQuadData::updateQuads() {
    //ofLogNotice("ZoneTransform::updateQuads()");
    
    int quadnum = xDivisions*yDivisions;
    quadWarpers.resize(quadnum);
    
    if(srcPoints.size()!=dstPoints.size()) {
        srcPoints.resize((xDivisions+1)*(yDivisions+1));
        
        updateSrc(srcRect);
    }
    
    for(int i = 0; i<quadnum; i++) {
        
        int x = i%xDivisions;
        int y = i/xDivisions;
        
        int topleft = x+(y*(xDivisions+1));
        int topright =x+1+(y*(xDivisions+1));
        int bottomleft=x+((y+1)*(xDivisions+1));
        int bottomright=x+1+((y+1)*(xDivisions+1));
        
        //cout << i<< " " <<x<< " " << y << " " << topleft<< " " << topright<< " " << bottomleft << " " << bottomright<< endl;
        
        Warper & quad = quadWarpers[i];
        quad.updateHomography(srcPoints[topleft],
                              srcPoints[topright],
                              srcPoints[bottomleft],
                              srcPoints[bottomright],
                              dstPoints[topleft],
                              dstPoints[topright],
                              dstPoints[bottomleft],
                              dstPoints[bottomright]
                              );
        
    }
    
   
    
    
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
    //cout << jsonGroup.dump(3) << endl;
    
    // number of handles could be different now
    int numhandles = (xDivisionsNew+1)*(yDivisionsNew+1);
    xDivisions = xDivisionsNew;
    yDivisions = yDivisionsNew;
    dstPoints.resize(numhandles);
    
    ofJson& handlejson = jsonGroup["handles"];
    //cout << handlejson.dump(3) << endl;
    if((int)handlejson.size()>=numhandles) {
        for(int i = 0; i<numhandles; i++) {
            ofJson& point = handlejson[i];
            dstPoints[i].x = point[0];
            dstPoints[i].y = point[1];
            //dstPoints[i].z = 0;
           // cout << "setting handle " << i << " : " << dstHandles[i] << endl;
            
        }
    }
    //updateDivisions(); //< SHOULD BE called automatically I think
    
   // ofLogNotice("ZoneTransform::deserialize");
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


//
//bool ZoneTransformQuadData :: setGrid(bool snapstate, int gridsize) {
//    if(ZoneTransformBase :: setGrid(snapstate, gridsize)) {
//        for(auto handle : dstHandles) {
//            handle.setGrid(snapToGrid, gridSize);
//        }
//        return true;
//    } else {
//        return false;
//    }
//}
//void ZoneTransformQuadData::setHue(int hue) {
//    ZoneTransformBase :: setHue(hue);
//    updateHandleColours();
//
//}
//void ZoneTransformQuadData::updateHandleColours() {
//
//    for(size_t i= 0; i<dstHandles.size(); i++) {
//        dstHandles[i].setColour(uiZoneHandleColour, uiZoneHandleColourOver);
//    }
//    // topleft
//    dstHandles[0].setColour(ofColor(180),ofColor(255));
//}


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
        //ofLogNotice()  << i << " " << p1 << " " << p2 << " " << p3 << " " << v2.dot(v1) ;
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


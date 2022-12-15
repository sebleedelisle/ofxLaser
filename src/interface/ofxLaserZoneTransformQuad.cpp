//
//  ofxLaserZoneTransform.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransformQuad.h"


using namespace ofxLaser;

// static property / method



ZoneTransformQuad::ZoneTransformQuad() {
     initListeners();
  
    
    dstHandles.resize(4);
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
    setSrc(ofRectangle(0,0,100,100));
    setDst(ofRectangle(100,100,200,200));
    
    xDivisionsNew.addListener(this, &ZoneTransformQuad::divisionsChanged);
    yDivisionsNew.addListener(this, &ZoneTransformQuad::divisionsChanged);
    
    ofAddListener(transformParams.parameterChangedE(), this, &ZoneTransformQuad::paramChanged);
    
    updateHandleColours();
}

void ZoneTransformQuad :: paramChanged(ofAbstractParameter& e) {
    isDirty= true;
    
}
ZoneTransformQuad::~ZoneTransformQuad() {
    removeListeners();
    xDivisionsNew.removeListener(this, &ZoneTransformQuad::divisionsChanged);
    yDivisionsNew.removeListener(this, &ZoneTransformQuad::divisionsChanged);
    ofRemoveListener(transformParams.parameterChangedE(), this, &ZoneTransformQuad::paramChanged);
}


void ZoneTransformQuad::init(ofRectangle& srcRect) {
    
//    float srcwidth = srcRect.getWidth();
//    float srcheight = srcRect.getHeight();
//
    setSrc(srcRect);
    
    // TODO - better default???
    
    ofRectangle destRect(600,600,100,100) ;
    
    //= srcRect;
    //destRect.scale(srcwidth/800, srcheight/800);
    //destRect.x*=srcwidth/800;
    //destRect.y*=srcheight/800;
    setDst(destRect);
    ofLogNotice("ZoneTransform::init - setDst");
    updateDivisions();
    updateQuads();
    
}


bool ZoneTransformQuad::update(){
    if(isDirty) {
        //ofLogNotice("ZoneTransform::update() - isDirty");
        updateQuads();
        updateHandleColours();
        updateConvex();
        isDirty = false;
        return true;
    } else {
        return false;
    }
    
    
}
void ZoneTransformQuad::draw(string label) {
    
    if(!visible) return ;
    
    ofPushMatrix();
    ofTranslate(offset);
    ofScale(scale, scale);
    ofSetColor(selected?uiZoneFillColourSelected : uiZoneFillColour);
    if((!editSubdivisions) && useHomography && (!getIsConvex())) {
        ofSetColor(80,0,0);
    }
    ofFill();
    ofBeginShape();
    for(int i = 0; i<xDivisions; i++) {
        ofVertex(dstHandles[i]);
    }
    for(int i = 0; i<yDivisions; i++) {
        ofVertex(dstHandles[(i*(xDivisions+1))+xDivisions]);
    }
    for(int i = 0; i<xDivisions; i++) {
        int index = ((xDivisions+1) * (yDivisions+1) )-1-i;
        ofVertex(dstHandles[index]);
    }
    for(int i = 0; i<yDivisions; i++) {
        int index = (yDivisions-i)*(xDivisions+1);
        ofVertex(dstHandles[index]);
    }
    
    ofEndShape();
    ofNoFill();
    
    ofSetColor(uiZoneStrokeColour);
   
    ofDrawBitmapString(label,getCentre() - ofPoint(4*label.size(),5));
    
    for(size_t i= 0; i<dstHandles.size(); i++) {
        int x = i%(xDivisions+1);
        int y = (int)i/(xDivisions+1);
        
        
        if(x<xDivisions) {
            if((y>0)&&(y<yDivisions)) {
                // then it's an inner line
                if(editSubdivisions) {
                    ofSetColor(selected ? uiZoneStrokeSubdivisionColourSelected : uiZoneStrokeSubdivisionColour);
                    ofDrawLine(dstHandles[i], dstHandles[i+1]);
                }
            } else {
                
                if(selected) {
                    ofSetLineWidth(2);
                }
                ofSetColor(selected ? uiZoneStrokeColourSelected : uiZoneStrokeColour);
                ofDrawLine(dstHandles[i], dstHandles[i+1]);
                
                ofSetLineWidth(1);
            }
            
        }
        
        if(y<yDivisions) {
            if((x>0)&&(x<xDivisions)) {
                // then it's an inner line
                if(editSubdivisions) {
                    ofSetColor(selected ? uiZoneStrokeSubdivisionColourSelected : uiZoneStrokeSubdivisionColour);
                    ofDrawLine(dstHandles[i], dstHandles[i+xDivisions+1]);
                }
            } else {
                if(selected) {
                    ofSetLineWidth(2);
                }
                ofSetColor(selected ? uiZoneStrokeColourSelected : uiZoneStrokeColour);
                ofDrawLine(dstHandles[i], dstHandles[i+xDivisions+1]);
                
                ofSetLineWidth(1);
                
            }
            
        }
    }
    
    if(selected && editable) {
        for(size_t i = 0; i<dstHandles.size(); i++) {
            if((editSubdivisions) || (isCorner((int)i))) dstHandles[i].draw(mousePos,scale);
        }
    }
    ofPopMatrix();
}

ofPoint ZoneTransformQuad::getCentre() {
    ofPoint centre = srcRect.getCenter();
    return getWarpedPoint(centre);
}

void ZoneTransformQuad :: resetToSquare() {
    vector<DragHandle*> cornerhandles = getCornerHandles();
    vector<ofPoint> corners;
    // convert to ofPoints
    for(DragHandle* handle : cornerhandles) corners.push_back(*handle);
    
    corners[0].x = corners[2].x = getLeft();
    corners[0].y = corners[1].y = getTop(); //ofLerp(corners[0].y,corners[1].y, 0.5);
    corners[1].x = corners[3].x = getRight(); // ofLerp(corners[1].x,corners[3].x,0.5);
    corners[3].y = corners[2].y = getBottom(); // ofLerp(corners[3].y,corners[2].y,0.5);
    setDstCorners(corners[0], corners[1], corners[2], corners[3]);
    isDirty = true;
}

bool ZoneTransformQuad :: isSquare() {
    
    vector<DragHandle*> corners = getCornerHandles();
    return (corners[0]->x == corners[2]->x) && (corners[0]->y == corners[1]->y) && (corners[1]->x == corners[3]->x) && (corners[2]->y == corners[3]->y);
    
}

ofPoint ZoneTransformQuad::getWarpedPoint(const ofPoint& p){
    
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

ofPoint ZoneTransformQuad::getUnWarpedPoint(const ofPoint& p){
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


ofxLaser::Point ZoneTransformQuad::getWarpedPoint(const ofxLaser::Point& p){
    
    ofxLaser::Point rp = p;
    
    if((!editSubdivisions) && useHomography && (!getIsConvex())) {
        rp.set(dstHandles[0]);
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
//	return p;
//};
void ZoneTransformQuad::setSrc(const ofRectangle& rect) {
    
    
    if((srcRect!=rect) || (srcPoints.size()!=((xDivisions+1)*(yDivisions+1)))) {
        ofLogNotice("ZoneTransform:: setSrc ") << rect;
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
void ZoneTransformQuad::setDst(const ofRectangle& rect) {
    setDstCorners(rect.getTopLeft(), rect.getTopRight(), rect.getBottomLeft(), rect.getBottomRight());
    updateQuads(); 
}

void ZoneTransformQuad :: setDstCorners(glm::vec3 topleft, glm::vec3 topright, glm::vec3 bottomleft, glm::vec3 bottomright) {
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
    
    for(size_t i= 0; i<dstHandles.size(); i++) {
        dstHandles[i].set(toOf(dstCVPoints[i]));
    }
}

void ZoneTransformQuad::resetFromCorners() {
    vector<ofPoint> corners = getCorners();
    setDstCorners(corners[0],corners[1],corners[2],corners[3]);
    
}

vector<DragHandle*> ZoneTransformQuad::getCornerHandles(){
    vector<DragHandle*> corners;
    corners.push_back(&dstHandles[0]);
    corners.push_back(&dstHandles[xDivisions]);
    corners.push_back(&dstHandles[yDivisions*(xDivisions+1)]);
    corners.push_back(&dstHandles[((xDivisions+1)*(yDivisions+1))-1]);
    return corners;
}

vector<ofPoint> ZoneTransformQuad::getCorners(){
    
    vector<ofPoint> corners;
    
    int indextopleft = 0 ;
    int indextopright = xDivisions;
    int indexbotleft =yDivisions*(xDivisions+1);
    int indexbotright =((xDivisions+1)*(yDivisions+1))-1;
    
    corners.push_back(dstHandles[indextopleft]);
    corners.push_back(dstHandles[indextopright]);
    corners.push_back(dstHandles[indexbotleft]);
    corners.push_back(dstHandles[indexbotright]);
    return corners;
    
}

vector<DragHandle*> ZoneTransformQuad::getCornerHandlesClockwise(){
    vector<DragHandle*> corners;
    
    int indextopleft = 0 ;
    int indextopright = xDivisions;
    int indexbotleft =yDivisions*(xDivisions+1);
    int indexbotright =((xDivisions+1)*(yDivisions+1))-1;
    
    corners.push_back(&dstHandles[indextopleft]);
    corners.push_back(&dstHandles[indextopright]);
    corners.push_back(&dstHandles[indexbotright]);
    corners.push_back(&dstHandles[indexbotleft]);
    return corners;
}

void ZoneTransformQuad::getPerimeterPoints(vector<glm::vec3>& points) {
    points.clear();
    
    for(int i = 0; i<xDivisions; i++) {
        points.push_back(dstHandles[i]);
    }
    for(int i = xDivisions; i<(((xDivisions+1)*(yDivisions+1))-1); i+=(xDivisions+1) ) {
        points.push_back(dstHandles[i]);
    }
    int start =((xDivisions+1)*(yDivisions+1))-1;
    int end = yDivisions*(xDivisions+1);
    for(int i =start ; i>end; i--){
        points.push_back(dstHandles[i]);
    }
    start = yDivisions*(xDivisions+1);
    end = 0;
    for(int i =start ; i>=end; i-=(xDivisions+1)){
        points.push_back(dstHandles[i]);
    }
    //    points.push_back(dstHandles[yDivisions*(xDivisions+1)]);
    //
}



bool ZoneTransformQuad :: isCorner(int i ) {
    return (i==0) || (i == xDivisions) || (i == yDivisions*(xDivisions+1)) || (i==((xDivisions+1)*(yDivisions+1))-1);
    
}

void ZoneTransformQuad :: setDivisions(int xdivisions, int ydivisions) {
    xDivisionsNew = xdivisions;
    yDivisionsNew = ydivisions;
    
    
    updateDivisions();
    
}

void ZoneTransformQuad:: divisionsChanged(int& e){
    if((xDivisionsNew!=xDivisions) || (yDivisionsNew!=yDivisions))
        updateDivisions();
}

void ZoneTransformQuad:: updateDivisions(){
    //ofLogNotice("ZoneTransform::updateDivisions()");
    
    //ofLog(OF_LOG_NOTICE, "divisionsChanged");
    
    vector<ofPoint> corners  = getCorners();
    
    xDivisions = xDivisionsNew;
    yDivisions = yDivisionsNew;
    dstHandles.resize((xDivisions+1)*(yDivisions+1));
    // srcpoints is resized in setSrc
    //srcPoints.resize((xDivisions+1)*(yDivisions+1));
    
    setSrc(srcRect);
    
        
    setDstCorners(corners[0], corners[1], corners[2], corners[3]);
    
    updateQuads();
    
    
}


void ZoneTransformQuad::updateQuads() {
    //ofLogNotice("ZoneTransform::updateQuads()");
    
    int quadnum = xDivisions*yDivisions;
    quadWarpers.resize(quadnum);
    
    if(srcPoints.size()!=dstHandles.size()) {
        srcPoints.resize((xDivisions+1)*(yDivisions+1));
        
        setSrc(srcRect);
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
                              dstHandles[topleft],
                              dstHandles[topright],
                              dstHandles[bottomleft],
                              dstHandles[bottomright]
                              );
        
    }
    
    for(DragHandle& handle : dstHandles) {
        handle.setGrid(snapToGrid, gridSize);
    } 
  
    
    
}


void ZoneTransformQuad::initListeners() {
    
    ofAddListener(ofEvents().mouseMoved, this, &ZoneTransformQuad::mouseMoved, OF_EVENT_ORDER_AFTER_APP);
    ofAddListener(ofEvents().mousePressed, this, &ZoneTransformQuad::mousePressed, OF_EVENT_ORDER_AFTER_APP);
    ofAddListener(ofEvents().mouseReleased, this, &ZoneTransformQuad::mouseReleased, OF_EVENT_ORDER_AFTER_APP);
    ofAddListener(ofEvents().mouseDragged, this, &ZoneTransformQuad::mouseDragged, OF_EVENT_ORDER_AFTER_APP);
    
    
}

void ZoneTransformQuad :: removeListeners() {
    
    ofRemoveListener(ofEvents().mouseMoved, this, &ZoneTransformQuad::mouseMoved, OF_EVENT_ORDER_AFTER_APP);
    ofRemoveListener(ofEvents().mousePressed, this, &ZoneTransformQuad::mousePressed, OF_EVENT_ORDER_AFTER_APP);
    ofRemoveListener(ofEvents().mouseReleased, this, &ZoneTransformQuad::mouseReleased, OF_EVENT_ORDER_AFTER_APP);
    ofRemoveListener(ofEvents().mouseDragged, this, &ZoneTransformQuad::mouseDragged, OF_EVENT_ORDER_AFTER_APP);
    
}

void ZoneTransformQuad :: mouseMoved(ofMouseEventArgs &e){
    
    
//    if((!editable) || (!visible)) return;
//
    mousePos = e;
    mousePos-=offset;
    mousePos/=scale;
    //return false;
    
}

bool ZoneTransformQuad :: mousePressed(ofMouseEventArgs &e){
    //ofLogNotice("ZoneTransform::mousePressed");
    // TODO there is currently an issue where if a zone is on top of another
    // zone, you can't click on a handle underneath. Not sure of how to fix this...
    // but possibly needs some higher level logic than here.
    
    if((!editable) || (!visible)) return false;
    //if(ofGetKeyPressed(' ')) return ; // for dragging around previews.
    
    mousePos = e;
    mousePos-=offset;
    mousePos/=scale;
    
    
    bool hit = hitTest(mousePos);
    if((hit) &&(!selected)) {
        selected = true;
        return false; //  propogates
    }
    
    
    if(!selected) {
        return false; // propogates
    }
    
    bool handleHit = false;
    
    if(!locked) {
        // this section of code if we click drag anywhere in the zone
        
        for(int i= dstHandles.size()-1; i>=0 && !handleHit; i--) {
            
            if(dstHandles[i].hitTest(mousePos, scale)) {
                
                // ignore subdivisions if we're not editing them
                if(!editSubdivisions && !isCorner((int)i)) continue;
                
                dstHandles[i].startDrag(mousePos);
                handleHit = true;
                
                
                if(!editSubdivisions && (isSquare())) {
                    
                    DragHandle& currentHandle = dstHandles[i];
                    
                    // OK MAJOR REWORK AHEAD >>>> >
                    // 1. we are dragging one of the points.
                    // 2. let's find the opposite one - that is the anchor
                    // 3. drag all the points relative to the anchor and the
                    //    current point being dragged
                    //    So, something like,
                    //    point.startDrag(clickpos, anchorpoint, relativetopoint, altpressed)
                    
                    vector<DragHandle*> corners = getCornerHandlesClockwise();
                    
                    int currenthandleindex = 0;
                    while(corners[currenthandleindex]!=&currentHandle) currenthandleindex++;
                    
                    ofLogNotice("Start drag index : " ) << currenthandleindex;
                    
                    DragHandle& anchorHandle = *corners[(currenthandleindex+2)%4];
                    DragHandle& dragHandle1 = *corners[(currenthandleindex+1)%4];
                    DragHandle& dragHandle2 = *corners[(currenthandleindex+3)%4];
                    
                    dragHandle1.startDragProportional(mousePos, anchorHandle, currentHandle, true);
                    dragHandle2.startDragProportional(mousePos, anchorHandle, currentHandle, true);

                    
                    
//                    vector<DragHandle*> corners;
//                    DragHandle& topLeft = dstHandles[0];
//                    DragHandle& topRight = dstHandles[xDivisions+1-1];
//                    DragHandle& bottomLeft = dstHandles[(yDivisions+1-1)*(xDivisions+1)];
//                    DragHandle& bottomRight = dstHandles.back();
//
//                    corners.push_back(&topLeft);
//                    corners.push_back(&topRight);
//                    corners.push_back(&bottomLeft);
//                    corners.push_back(&bottomRight);
//
//                    int handleIndex = 0;
//                    if(currentHandle == topLeft) handleIndex = 0;
//                    else if(currentHandle == topRight) handleIndex = 1;
//                    else if(currentHandle == bottomLeft) handleIndex = 2;
//                    else if(currentHandle == bottomRight) handleIndex =3;
//
//                    int x = ((handleIndex%2)+1)%2;
//                    int y = handleIndex/2;
//
//                    int xhandleindex = x+(y*2);
//
//                    x = handleIndex%2;
//                    y = ((handleIndex/2)+1)%2;
//                    int yhandleindex = x+(y*2);
//
//                    corners[xhandleindex]->startDrag(mousePos, false,true, true);
//                    corners[yhandleindex]->startDrag(mousePos, true,false, true);

                    //				bottomLeft.startDrag(mousePoint, false,true, true);
                    //				topRight.startDrag(mousePoint, true,false, true);
                    
                }
            }
            
        }
        
        // if the middle is clicked then drag all the handles!
        if(!handleHit && hit) {
            
            //centreHandle.startDrag(mousePoint);
            handleHit = true;
            DragHandle* gridHandle = &dstHandles[0];
            for(size_t i= 0; i<dstHandles.size(); i++) {
                if(&dstHandles[i] == gridHandle) {
                    dstHandles[i].startDrag(mousePos);
                } else {
                    dstHandles[i].startDrag(mousePos, gridHandle);
                }
            }
            
            
        }
    }
    if(!handleHit && !hit) {
        selected = false;
    }
    
    return false; // propogates, was : handleHit || hit;
    
}




void ZoneTransformQuad :: mouseDragged(ofMouseEventArgs &e){
    
    if((!editable) || (!visible)) return ;
    if(!selected) return ;
    
    ofPoint mousePoint;
    mousePoint.x = e.x;
    mousePoint.y = e.y;
    mousePoint-=offset;
    mousePoint/=scale;
    
    //ofRectangle bounds(centreHandle, 0, 0);
    int dragCount = 0;
    for(size_t i= 0; i<dstHandles.size(); i++) {
        if(dstHandles[i].updateDrag(mousePoint)) dragCount++;
        //bounds.growToInclude(handles[i]);
    }
    //	if(!dragging) {
    //		dragging = centreHandle.updateDrag(mousePoint);
    //	} else {
    //		updateCentreHandle();
    //
    //	}
    
    isDirty |= (dragCount>0);
    //if((dragCount>0)&&(!editSubdivisions)) resetFromCorners();
    
    //return dragCount>0;
    
    
}


void ZoneTransformQuad :: mouseReleased(ofMouseEventArgs &e){
    
    //if(!editable) return false;
    if(!selected) return;
    
    bool wasDragging = false;
    
    for(size_t i= 0; i<dstHandles.size(); i++) {
        if(dstHandles[i].stopDrag()) wasDragging = true;
    }
    
    // TODO mark as dirty so auto save ********************
    //saveSettings();
    isDirty|=wasDragging;
    //return wasDragging;
    
}

bool ZoneTransformQuad::hitTest(ofPoint mousePoint) {
    
    ofPolyline poly;
    for(int i = 0; i<=xDivisions; i++) {
        poly.addVertex(dstHandles[i]);
        //ofLog(OF_LOG_NOTICE, ofToString(i));
    }
    //ofLog(OF_LOG_NOTICE, "---");
    for(int i = 2; i<=yDivisions; i++) {
        poly.addVertex(dstHandles[(i*(xDivisions+1))-1]);
        //ofLog(OF_LOG_NOTICE, ofToString((i*(xDivisions+1))-1));
    }
    //ofLog(OF_LOG_NOTICE, "---");
    for(int i = ((xDivisions+1)*(yDivisions+1))-1; i>=(xDivisions+1)*(yDivisions); i--) {
        poly.addVertex(dstHandles[i]);
        //ofLog(OF_LOG_NOTICE, ofToString(i));
    }
    //ofLog(OF_LOG_NOTICE, "---");
    for(int i = yDivisions-1; i>=0; i--) {
        poly.addVertex(dstHandles[(i*(xDivisions+1))]);
        //ofLog(OF_LOG_NOTICE, ofToString((i*(xDivisions+1))));
    }
    
    if(poly.inside(mousePoint)) {
        return true;
    } else {
        ofPoint closest = poly.getClosestPoint(mousePoint);
        if(closest.distance(mousePoint)<(2.0f/scale)) {
            return true;
        } else {
            return false;
        }
    }
    //return poly.inside(mousePoint);
    
}
//
//void ZoneTransform::saveSettings() {
//
//	//ofLog(OF_LOG_NOTICE, "ZoneTransform::saveSettings");
//
//	ofJson json;
//	serialize(json);
//	ofSavePrettyJson(getSaveLabel()+".json", json);
//
//}

bool ZoneTransformQuad::serialize(ofJson&json) {
    ofSerialize(json, transformParams);
    ofJson& handlesjson = json["handles"];
    for(size_t i= 0; i<dstHandles.size(); i++) {
        DragHandle& pos = dstHandles[i];
        handlesjson.push_back({pos.x, pos.y});
    }
    
    return true;
}

bool ZoneTransformQuad::deserialize(ofJson& jsonGroup) {
    //ofLogNotice("ZoneTransform::deserialize()");
    // note that ofDeserialize looks for the json group
    // with the same name as the parameterGroup
    ofDeserialize(jsonGroup, transformParams);
    //cout << jsonGroup.dump(3) << endl;
    
    // number of handles could be different now
    int numhandles = (xDivisionsNew+1)*(yDivisionsNew+1);
    xDivisions = xDivisionsNew;
    yDivisions = yDivisionsNew;
    dstHandles.resize(numhandles);
    
    ofJson& handlejson = jsonGroup["handles"];
    cout << handlejson.dump(3) << endl;
    if((int)handlejson.size()>=numhandles) {
        for(int i = 0; i<numhandles; i++) {
            ofJson& point = handlejson[i];
            dstHandles[i].x = point[0];
            dstHandles[i].y = point[1];
            dstHandles[i].z = 0;
            cout << "setting handle " << i << " : " << dstHandles[i] << endl;
            
        }
    }
    //updateDivisions(); //< SHOULD BE called automatically I think
    
    ofLogNotice("ZoneTransform::deserialize");
    return true;
}


float ZoneTransformQuad::getRight() {
    float right = 0;
    for(DragHandle& handle : dstHandles) {
        if(handle.x>right) right = handle.x;
    }
    
    return right;
}
float ZoneTransformQuad::getLeft() {
    float left = 800;
    for(DragHandle& handle : dstHandles) {
        if(handle.x<left) left = handle.x;
    }
    
    return left;
}

float ZoneTransformQuad::getTop() {
    float top = 800;
    for(DragHandle& handle : dstHandles) {
        if(handle.y<top) top = handle.y;
    }
    
    return top;
}
float ZoneTransformQuad::getBottom() {
    float bottom = 0;
    for(DragHandle& handle : dstHandles) {
        if(handle.y>bottom) bottom = handle.y;
    }
    
    return bottom;
}



bool ZoneTransformQuad :: setGrid(bool snapstate, int gridsize) {
    if(ZoneTransformBase :: setGrid(snapstate, gridsize)) {
        for(auto handle : dstHandles) {
            handle.setGrid(snapToGrid, gridSize);
        }
        return true;
    } else {
        return false;
    }
}
void ZoneTransformQuad::setHue(int hue) {
    ZoneTransformBase :: setHue(hue);
    updateHandleColours();
  
}
void ZoneTransformQuad::updateHandleColours() {
    
    for(size_t i= 0; i<dstHandles.size(); i++) {
        dstHandles[i].setColour(uiZoneHandleColour, uiZoneHandleColourOver);
    }
    // topleft
    dstHandles[0].setColour(ofColor(180),ofColor(255));
}


bool ZoneTransformQuad :: getIsConvex() {
    return isConvex;
    
}

void ZoneTransformQuad :: updateConvex() {
    bool convex = true;
    vector<ofPoint> corners = getCorners();
    vector<ofPoint> points;
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


bool ZoneTransformQuad::setSelected(bool v) {
    
    if(ZoneTransformBase::setSelected(v)) {
        if(!selected) {
            for(size_t i= 0; i<dstHandles.size(); i++) {
                dstHandles[i].stopDrag();
            }
        }
        return true;
    } else {
        return false;
    }
    
};


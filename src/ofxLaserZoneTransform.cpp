//
//  ofxLaserZoneTransform.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransform.h"

using namespace ofxLaser;


ZoneTransform::ZoneTransform() {

	initListeners();
	visible = true;
	//isDirty = true;
	selected = true;
	initialised = true;
	
	xDivisions = 1;
	yDivisions = 1;
	dstHandles.resize(4);
	srcPoints.resize(4);
	
	setDivisions(2,2);
	setSrc(ofRectangle(0,0,100,100));
	setDst(ofRectangle(100,100,200,200));

	
	
}

ZoneTransform::~ZoneTransform() {
	if(initialised) removeListeners();
}


void ZoneTransform::update(){
//	for(int i = 0; i<dstHandles.size(); i++) {
//		//dstHandles[i].update();
//		
//	}
}

void ZoneTransform::draw() {
	
	
	for(int i= 0; i<dstHandles.size(); i++) {
		int x = i%(xDivisions+1);
		int y = i/(xDivisions+1);
		
		if(x<xDivisions) {
			drawDashedLine(dstHandles[i], dstHandles[i+1]);
		}
		if(y<xDivisions) {
			drawDashedLine(dstHandles[i], dstHandles[i+xDivisions+1]);
		}
	}
	
	for(int i = 0; i<dstHandles.size(); i++) {
		dstHandles[i].draw();
	}
}

void ZoneTransform::setSrc(const ofRectangle& rect) {
	srcRect = rect;
	// update source points?
	int xpoints = xDivisions+1;
	int ypoints = yDivisions+1;
	
	// srcPoints should already have enough
	
	int numpoints = xpoints*ypoints;
	
	for(int i= 0; i<numpoints; i++) {
		float x = ofMap(i%xpoints, 0, xDivisions, rect.getLeft(), rect.getRight());
		float y = ofMap(i/xpoints, 0, yDivisions, rect.getTop(), rect.getBottom());
		
		ofLog(OF_LOG_NOTICE, ofToString(x) + " " +ofToString(y));
		
		srcPoints[i].set(x, y);
		
	}
	
}
void ZoneTransform::setDst(const ofRectangle& rect) {
	setDstCorners(rect.getTopLeft(), rect.getTopRight(), rect.getBottomLeft(), rect.getBottomRight());
	
}

void ZoneTransform :: setDstCorners(ofPoint topleft, ofPoint topright, ofPoint bottomleft, ofPoint bottomright) {
	// interpolate dst handle points?
	
	
	vector<cv::Point2f> srcCVPoints, dstCVPoints;
	srcCVPoints.resize(4);
	dstCVPoints.resize(4);
	
	srcCVPoints[0] = toCv(srcRect.getTopLeft());
	srcCVPoints[1] = toCv(srcRect.getTopRight());
	srcCVPoints[2] = toCv(srcRect.getBottomLeft());
	srcCVPoints[3] = toCv(srcRect.getBottomRight());
	
	dstCVPoints[0] = toCv(topleft);
	dstCVPoints[1] = toCv(topright);
	dstCVPoints[2] = toCv(bottomleft);
	dstCVPoints[3] = toCv(bottomright);
	
	cv::Mat homography = cv::findHomography(cv::Mat(srcCVPoints), cv::Mat(dstCVPoints),CV_RANSAC, 100);
	
	srcCVPoints.resize(srcPoints.size());
	dstCVPoints.resize(srcPoints.size());
	

	for(int i = 0; i<srcPoints.size(); i++) {
		srcCVPoints[i] = toCv(srcPoints[i]);
	
	}
	cv::perspectiveTransform(srcCVPoints, dstCVPoints, homography);
	
	for(int i = 0; i<dstHandles.size(); i++) {
		dstHandles[i].set(toOf(dstCVPoints[i]));
		
	}
	
	
}


void ZoneTransform :: setDivisions(int xdivisions, int ydivisions) {
	xDivisions = xdivisions;
	yDivisions = ydivisions; 
	// create source points from source rectangle
	dstHandles.resize((xDivisions+1)*(yDivisions+1));
	srcPoints.resize((xDivisions+1)*(yDivisions+1));
	
	// create dest points from dest quad
	
}



void ZoneTransform::initListeners() {
	
	ofAddListener(ofEvents().mousePressed, this, &ZoneTransform::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().mouseReleased, this, &ZoneTransform::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().mouseDragged, this, &ZoneTransform::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
	
	
}

void ZoneTransform :: removeListeners() {
	
	ofRemoveListener(ofEvents().mousePressed, this, &ZoneTransform::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(ofEvents().mouseReleased, this, &ZoneTransform::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(ofEvents().mouseDragged, this, &ZoneTransform::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
	
}

bool ZoneTransform :: mousePressed(ofMouseEventArgs &e){
	
	
	if(!visible) return false;
	
	bool hit = false; //hitTest(e);
	if((hit) &&(!selected)) {
		selected = true;
		return false;
	}
	
	
	if(!selected) {
		return false;
	}
	ofPoint mousePoint = e;
	mousePoint-=offset;
	mousePoint/=scale;
	
	
	
	bool handleHit = false;
	
	// this section of code if we click drag anywhere in the zone
//	if(centreHandle.hitTest(mousePoint)) {
//		
//		centreHandle.startDrag(mousePoint);
//		handleHit = true;
//		for(int i = 0; i<numHandles; i++) {
//			handles[i].startDrag(mousePoint);
//		}
//		
//		
//	} else {
	
	for(int i = 0; i<dstHandles.size(); i++) {
		if(dstHandles[i].hitTest(mousePoint)) {
			//startDragging(i, mousePoint);
			dstHandles[i].startDrag(mousePoint);
			handleHit = true;
		}
		
	}

	
	if(!handleHit && !hit) {
		selected = false;
	}
	return handleHit;
	
}

bool ZoneTransform :: mouseDragged(ofMouseEventArgs &e){
	
	if(!visible) return false;
	if(!selected) return false;
	
	ofPoint mousePoint = e;
	mousePoint-=offset;
	mousePoint/=scale;
	
	//ofRectangle bounds(centreHandle, 0, 0);
	bool dragging = false;
	for(int i = 0; i<dstHandles.size(); i++) {
		if(dstHandles[i].updateDrag(mousePoint)) dragging = true;
		//bounds.growToInclude(handles[i]);
	}
//	if(!dragging) {
//		dragging = centreHandle.updateDrag(mousePoint);
//	} else {
//		updateCentreHandle();
//		
//	}
	
	//isDirty |= dragging;
	
	
	return dragging;
	
	
}


bool ZoneTransform :: mouseReleased(ofMouseEventArgs &e){
	
	if(!visible) return false;
	if(!selected) return false;
	
	
	bool wasDragging = false;
	
	for(int i = 0; i<dstHandles.size(); i++) {
		if(dstHandles[i].stopDrag()) wasDragging = true;
	}
	
	//saveSettings();
	return wasDragging;
	
}

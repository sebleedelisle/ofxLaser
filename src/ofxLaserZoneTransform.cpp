//
//  ofxLaserZoneTransform.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransform.h"

using namespace ofxLaser;


ZoneTransform::ZoneTransform(string labelname, string filename) {

	saveLabel = filename;
	displayLabel = labelname;

	scale = 1;
	offset.set(0,0);
	initListeners();
	visible = true;
	isDirty = true;
	selected = true;
	initialised = true;
	
	dstHandles.resize(4);
	srcPoints.resize(4);
	simpleMode = true;
	
	params.setName("ZoneTransform");

	params.add(xDivisionsNew.set("x divisions", 1,1,6));
	params.add(yDivisionsNew.set("y divisions", 1,1,6));
	params.add(simpleMode.set("simple mode", true));

	xDivisions = 1;
	yDivisions = 1;
	setSrc(ofRectangle(0,0,100,100));
	setDst(ofRectangle(100,100,200,200));
    //setDivisions(3,3);
	
	loadSettings();
	
	xDivisionsNew.addListener(this, &ZoneTransform::divisionsChanged);
	yDivisionsNew.addListener(this, &ZoneTransform::divisionsChanged);
	

}

ZoneTransform::~ZoneTransform() {
	if(initialised) removeListeners();
}


void ZoneTransform::update(){
	if(isDirty) {
		
		updateQuads();
	}
	isDirty = false;

}
void ZoneTransform :: setVisible(bool warpvisible){
	visible = warpvisible;
}

void ZoneTransform::draw() {
	
	ofPushMatrix();
	ofTranslate(offset);
	ofScale(scale, scale);
	
	for(int i= 0; i<dstHandles.size(); i++) {
		int x = i%(xDivisions+1);
		int y = i/(xDivisions+1);
		
		
		
		ofColor edge = ofColor(255);
		ofColor inside  = simpleMode?ofColor(0,0,255) : ofColor(100,100,255);
		
		if(x<xDivisions) {
			if((y>0)&&(y<yDivisions)) {
				ofSetColor(inside);
			} else {
				ofSetColor(edge);
			}
			drawDashedLine(dstHandles[i], dstHandles[i+1]);
		}
		if(y<yDivisions) {
			if((x>0)&&(x<xDivisions)) {
				ofSetColor(inside);
			} else {
				ofSetColor(edge);
			}
			drawDashedLine(dstHandles[i], dstHandles[i+xDivisions+1]);
		}
	}
	
	if(selected) {
		for(int i = 0; i<dstHandles.size(); i++) {
			if((!simpleMode) || (isCorner(i))) dstHandles[i].draw();
		}
	}
	ofPopMatrix();
}


ofPoint ZoneTransform::getWarpedPoint(const ofPoint& p){
	ofPoint rp = p - srcRect.getTopLeft();
	
	int x = (rp.x / srcRect.getWidth()) * (float)(xDivisions);
	int y = (rp.y / srcRect.getHeight()) * (float)(yDivisions);
	
	//ofLog(OF_LOG_NOTICE, ofToString(x) + " " + ofToString(y));
	
	x = ofClamp(x,0,xDivisions-1);
	y = ofClamp(y,0,yDivisions-1);
	
	int quadnum = x + (y*xDivisions);
	Warper & quad = quadWarpers[quadnum];
	return quad.getWarpedPoint(p);
	
};

ofxLaser::Point ZoneTransform::getWarpedPoint(const ofxLaser::Point& p){
	ofxLaser::Point rp = p;
	rp.x-=srcRect.getTopLeft().x;
	rp.y-=srcRect.getTopLeft().y;
	
	int x = (rp.x / srcRect.getWidth()) * (float)(xDivisions);
	int y = (rp.y / srcRect.getHeight()) * (float)(yDivisions);
	
	//ofLog(OF_LOG_NOTICE, ofToString(x) + " " + ofToString(y));
	
	x = ofClamp(x,0,xDivisions-1);
	y = ofClamp(y,0,yDivisions-1);
	
	int quadnum = x + (y*xDivisions);
	Warper & quad = quadWarpers[quadnum];
	return quad.getWarpedPoint(p);
	
};
//
//Point getUnWarpedPoint(const Point& p){
//	return p;
//};
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
		
		//ofLog(OF_LOG_NOTICE, ofToString(x) + " " +ofToString(y));
		
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

void ZoneTransform::resetFromCorners() {
	vector<ofPoint> corners = getCorners();
	setDstCorners(corners[0],corners[1],corners[2],corners[3]);

}
vector<ofPoint> ZoneTransform::getCorners(){
	vector<ofPoint> corners;
	corners.push_back(dstHandles[0]);
	corners.push_back(dstHandles[xDivisions]);
	corners.push_back(dstHandles[yDivisions*(xDivisions+1)]);
	corners.push_back(dstHandles[((xDivisions+1)*(yDivisions+1))-1]);
	return corners;
}

bool ZoneTransform :: isCorner(int i ) {
	return (i==0) || (i == xDivisions) || (i == yDivisions*(xDivisions+1)) || (i==((xDivisions+1)*(yDivisions+1))-1);
	
}

void ZoneTransform :: setDivisions(int xdivisions, int ydivisions) {
	xDivisionsNew = xdivisions;
	yDivisionsNew = ydivisions;
	
	
	updateDivisions();
	
}

void ZoneTransform:: divisionsChanged(int& e){
	updateDivisions();
}

void ZoneTransform:: updateDivisions(){
	//ofLog(OF_LOG_NOTICE, "divisionsChanged");
	
    vector<ofPoint> corners  = getCorners();
    
	xDivisions = xDivisionsNew;
	yDivisions = yDivisionsNew;
	dstHandles.resize((xDivisions+1)*(yDivisions+1));
	srcPoints.resize((xDivisions+1)*(yDivisions+1));
	
	setSrc(srcRect);
	
	
	setDstCorners(corners[0], corners[1], corners[2], corners[3]);
	
	updateQuads(); 
	
}


void ZoneTransform::updateQuads() {
	
	int quadnum = xDivisions*yDivisions;
	quadWarpers.resize(quadnum);
	
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

	ofPoint mousePoint = e;
	mousePoint-=offset;
	mousePoint/=scale;
	
	
	bool hit = hitTest(mousePoint);
	if((hit) &&(!selected)) {
		selected = true;
		return false;
	}
	
	
	if(!selected) {
		return false;
	}
	
	
	
	
	
	bool handleHit = false;
	
	// this section of code if we click drag anywhere in the zone

	
	for(int i = 0; i<dstHandles.size(); i++) {
		if(dstHandles[i].hitTest(mousePoint)) {
			
			if(simpleMode && !isCorner(i)) continue;
			
			dstHandles[i].startDrag(mousePoint);
			handleHit = true;
			
			if(simpleMode) {
				
				DragHandle& currentHandle = dstHandles[i];
				
				vector<DragHandle*> corners;
				DragHandle& topLeft = dstHandles[0];
				DragHandle& topRight = dstHandles[xDivisions+1-1];
				DragHandle& bottomLeft = dstHandles[(yDivisions+1-1)*(xDivisions+1)];
				DragHandle& bottomRight = dstHandles.back();
				
				corners.push_back(&topLeft);
				corners.push_back(&topRight);
				corners.push_back(&bottomLeft);
				corners.push_back(&bottomRight);
			
				int handleIndex;
				if(currentHandle == topLeft) handleIndex = 0;
				if(currentHandle == topRight) handleIndex = 1;
				if(currentHandle == bottomLeft) handleIndex = 2;
				if(currentHandle == bottomRight) handleIndex =3;
				
				int x = ((handleIndex%2)+1)%2;
				int y = handleIndex/2;
				
				int xhandleindex = x+(y*2);
				
				x = handleIndex%2;
				y = ((handleIndex/2)+1)%2;
				int yhandleindex = x+(y*2);
				
				corners[xhandleindex]->startDrag(mousePoint, false,true, true);
				corners[yhandleindex]->startDrag(mousePoint, true,false, true);
				
//				bottomLeft.startDrag(mousePoint, false,true, true);
//				topRight.startDrag(mousePoint, true,false, true);
				
			}
		}
		
	}
	
	// drag all the handles!
	if(!handleHit && hit) {

		//centreHandle.startDrag(mousePoint);
		handleHit = true;
		for(int i = 0; i<dstHandles.size(); i++) {
			dstHandles[i].startDrag(mousePoint);
		}


	}
	
	if(!handleHit && !hit) {
		selected = false;
	}
	return handleHit || hit;
	
}




bool ZoneTransform :: mouseDragged(ofMouseEventArgs &e){
	
	if(!visible) return false;
	if(!selected) return false;
	e-=offset;
	e/=scale;
	ofPoint mousePoint = e;

	
	//ofRectangle bounds(centreHandle, 0, 0);
	int dragCount = 0;
	for(int i = 0; i<dstHandles.size(); i++) {
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
	if((dragCount>0)&&(simpleMode)) resetFromCorners();
	
	return dragCount>0;
	
	
}


bool ZoneTransform :: mouseReleased(ofMouseEventArgs &e){
	
	if(!visible) return false;
	if(!selected) return false;
	
	
	bool wasDragging = false;
	
	for(int i = 0; i<dstHandles.size(); i++) {
		if(dstHandles[i].stopDrag()) wasDragging = true;
	}
	
	saveSettings();
	return wasDragging;
	
}

bool ZoneTransform::hitTest(ofPoint mousePoint) {
	
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

	return poly.inside(mousePoint);
	
}

void ZoneTransform::saveSettings() {
	ofLog(OF_LOG_NOTICE, "ZoneTransform::saveSettings");
	ofParameterGroup saveParams;
	
	saveParams.setName("handles");
	for(int i = 0; i<dstHandles.size(); i++) {
		ofParameter<ofPoint> p;
		p = dstHandles[i];
		p.setName("dstHandle"+ofToString(i));
		
		//ofLog(OF_LOG_NOTICE, "dstHandle"+ofToString(i));
		saveParams.add(p);
	}
	
	ofxPanel gui;
	
	gui.add(params);

	gui.saveToFile(saveLabel+".xml");
	ofxPanel gui2;
	gui2.add(saveParams);
	gui2.saveToFile(saveLabel+"-Points.xml");
	

}

bool ZoneTransform::loadSettings() {
	
	ofParameterGroup loadParams;
	ofxPanel gui;
	gui.add(params);
	
	gui.loadFromFile(saveLabel+".xml");
	
	ofxPanel gui2;
	
	int numhandles = (xDivisionsNew+1)*(yDivisionsNew+1);
	
	dstHandles.resize(numhandles);
	
	for(int i = 0; i<numhandles; i++) {
		ofParameter<ofPoint> p;
		p = dstHandles[i];
		p.setName("dstHandle"+ofToString(i));
		loadParams.add(p);
	}
	loadParams.setName("handles");
	gui2.add(loadParams);
	
	gui2.loadFromFile(saveLabel+"-Points.xml");
	for(int i = 0; i<numhandles; i++) {
		dstHandles[i].set(loadParams.getPoint("dstHandle"+ofToString(i)));
		//ofLog(OF_LOG_NOTICE,ofToString(i)+"===="+ofToString(loadParams.getPoint("dstHandle"+ofToString(i))));
	}
	
	// shouldn't update all sections?
	updateDivisions();
	
	//ofLog(OF_LOG_NOTICE, loadParams.toString());
	return true;
}

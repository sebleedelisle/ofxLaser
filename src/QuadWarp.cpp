//
//  QuadWarp.cpp
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//

#include "QuadWarp.h"

QuadWarp::QuadWarp(string savelabel) {
	initListeners();
	
	saveLabel = savelabel;
	
}

void QuadWarp::set (float x, float y, float w, float h) {
	
	for(int i  = 0;i<4; i++) {
		bool left = (i+1)%4 <2; 
		bool top = i<2;
		handles[i].set(left? x : x+w, top ? y : y+h);
		allHandles.push_back(&handles[i]);
	}
	centreHandle.set(x + (w/2.0f), y+(h/2.0f));
	allHandles.push_back(&centreHandle);
	
}


void QuadWarp :: initListeners() {

	ofAddListener(ofEvents().mousePressed, this, &QuadWarp::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().mouseReleased, this, &QuadWarp::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().mouseDragged, this, &QuadWarp::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
	



}

void QuadWarp :: draw() {
	
	ofPushStyle();
	ofNoFill();
	ofSetLineWidth(1);
	
	for(int i = 0; i<numHandles; i++) {
		ofLine(handles[i],  handles[(i+1)%4]);
		
	}
	ofPopStyle();
	
	for(int i = 0; i<numHandles; i++) {
		handles[i].draw();
	}
	
	centreHandle.draw();
}


void QuadWarp :: startDragging(int handleIndex, ofPoint clickPos) {
	
	handles[handleIndex].startDrag(clickPos);
	handles[(handleIndex+1)%4].startDrag(clickPos, handleIndex%2==1,handleIndex%2==0);
	handles[(handleIndex+3)%4].startDrag(clickPos, handleIndex%2==0, handleIndex%2==1);
	

	
	
}

bool QuadWarp :: mousePressed(ofMouseEventArgs &e){
	
	bool handleHit = false;
	
	if(centreHandle.hitTest(e)) {
		
		centreHandle.startDrag(e);
		handleHit = true;
		for(int i = 0; i<numHandles; i++) {
			handles[i].startDrag(e);
		}
			
		
		
	} else {

		for(int i = 0; i<numHandles; i++) {
			if(handles[i].hitTest(e)) {
			//	handles[i].startDrag(e);
				startDragging(i, e);
				handleHit = true;
			}
		
		}
	}
	
	return handleHit;
	
}

bool QuadWarp :: mouseDragged(ofMouseEventArgs &e){
	
	ofRectangle bounds(centreHandle, 0, 0);
	bool dragging = false;
	for(int i = 0; i<numHandles; i++) {
		if(handles[i].updateDrag(e)) dragging = true;
		bounds.growToInclude(handles[i]);
	}
	
	if(!dragging) {
		dragging = centreHandle.updateDrag(e);
	} else {
		centreHandle.set(bounds.getCenter());
		
	}
	
	return dragging;
	
	
}

bool QuadWarp :: mouseReleased(ofMouseEventArgs &e){
	bool wasDragging = false;
	for(int i = 0; i<allHandles.size(); i++) {
		if(allHandles[i]->stopDrag()) wasDragging = true;
	}
	
	return wasDragging;
	
}



bool QuadWarp::loadSettings() {
	
	string filename = saveLabel+".xml";
	ofxXmlSettings xml;
	if(!xml.loadFile(filename)) {
		ofLog(OF_LOG_ERROR, "QuadWarp::loadSettings - file not found : "+filename);
		return false;
		
	}
	
	//cout << "Warp Pre load: " << filename << " " << dstPoints[0].x << ", " << dstPoints[0].y << endl;
	
	
	handles[0].x	= xml.getValue("quad:upperLeft:x", 0.0);
	handles[0].y	= xml.getValue("quad:upperLeft:y", 0.0);
	
	handles[1].x	= xml.getValue("quad:upperRight:x", 1.0);
	handles[1].y	= xml.getValue("quad:upperRight:y", 0.0);
	
	handles[2].x	= xml.getValue("quad:lowerRight:x", 1.0);
	handles[2].y	= xml.getValue("quad:lowerRight:y", 1.0);
	
	handles[3].x	= xml.getValue("quad:lowerLeft:x", 0.0);
	handles[3].y	= xml.getValue("quad:lowerLeft:y", 1.0);
	
	
	//updateCentrePoint();
	
	return true;
}

void QuadWarp::saveSettings() {
	
	ofxXmlSettings xml;
	
	string filename = saveLabel+".xml";
	
	xml.addTag("quad");
	xml.pushTag("quad");
	
	xml.addTag("upperLeft");
	xml.pushTag("upperLeft");
	xml.addValue("x", handles[0].x);
	xml.addValue("y", handles[0].y);
	xml.popTag();
	
	xml.addTag("upperRight");
	xml.pushTag("upperRight");
	xml.addValue("x", handles[1].x);
	xml.addValue("y", handles[1].y);
	xml.popTag();
	
	xml.addTag("lowerRight");
	xml.pushTag("lowerRight");
	xml.addValue("x", handles[2].x);
	xml.addValue("y", handles[2].y);
	xml.popTag();
	
	xml.addTag("lowerLeft");
	xml.pushTag("lowerLeft");
	xml.addValue("x", handles[3].x);
	xml.addValue("y", handles[3].y);
	xml.popTag();
	
	
	xml.saveFile(filename);
	
	
}


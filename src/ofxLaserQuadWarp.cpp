//
//  QuadWarp.cpp
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//

#include "ofxLaserQuadWarp.h"
using namespace ofxLaser;

QuadWarp::QuadWarp(string savelabel) {
	initListeners();
	
	saveLabel = savelabel;
	visible = true;
	
}

void QuadWarp::set (float x, float y, float w, float h) {

    for(int i = 0; i<4; i++) {
        float xpos = ((float)(i%2)/1.0f*w)+x;
        float ypos = (floor((float)(i/2))/1.0f*h)+y;

        handles[i].set(xpos, ypos);
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
	
	if(!visible) return;
    ofPushMatrix();
    ofTranslate(offset);
    ofScale(scale, scale);
    
	ofPushStyle();
	ofNoFill();
	ofSetLineWidth(1/scale);
	if(isDirty) ofSetColor(ofColor::red);
	
	for(int i = 0; i<numHandles; i++) {
		//ofLine(handles[i],  handles[(i+1)%4]);
        ofDrawCircle(handles[i], 5);
        ofDrawBitmapString(ofToString(i), handles[i]+ofPoint(10,10));
		
	}
    ofDrawLine(handles[0], handles[1]);
    ofDrawLine(handles[1], handles[3]);
    ofDrawLine(handles[3], handles[2]);
    ofDrawLine(handles[2], handles[0]);
    
	ofPopStyle();
	
	for(int i = 0; i<numHandles; i++) {
		handles[i].draw();
	}
	
	centreHandle.draw();
	isDirty = false;
    ofPopMatrix();
}

bool QuadWarp::checkDirty() {
	if(isDirty) {
		//isDirty = false;
		return true;
	} else {
		return false;
	}
	
	
}


void QuadWarp :: startDragging(int handleIndex, ofPoint clickPos) {
	
	handles[handleIndex].startDrag(clickPos);
    
    int x = ((handleIndex%2)+1)%2;
    int y = handleIndex/2;
    
    
    int xhandleindex = x+(y*2);
    
    x = handleIndex%2;
    y = ((handleIndex/2)+1)%2;
    int yhandleindex = x+(y*2);
    
    handles[xhandleindex].startDrag(clickPos, false,true, true);
    handles[yhandleindex].startDrag(clickPos, true,false, true);
    
	
}

bool QuadWarp :: mousePressed(ofMouseEventArgs &e){
	
	
	if(!visible) return false;
	
	bool handleHit = false;
	
    ofPoint mousePoint = e;
    mousePoint-=offset;
    mousePoint/=scale;
    
	if(centreHandle.hitTest(mousePoint)) {
		
		centreHandle.startDrag(mousePoint);
		handleHit = true;
		for(int i = 0; i<numHandles; i++) {
			handles[i].startDrag(mousePoint);
		}
			
		
		
	} else {

		for(int i = 0; i<numHandles; i++) {
			if(handles[i].hitTest(mousePoint)) {
				startDragging(i, mousePoint);
				handleHit = true;
			}
		
		}
	}
	
	return handleHit;
	
}

bool QuadWarp :: mouseDragged(ofMouseEventArgs &e){
	
    ofPoint mousePoint = e;
    mousePoint-=offset;
    mousePoint/=scale;
    
	ofRectangle bounds(centreHandle, 0, 0);
	bool dragging = false;
	for(int i = 0; i<numHandles; i++) {
		if(handles[i].updateDrag(mousePoint)) dragging = true;
		bounds.growToInclude(handles[i]);
	}
	
	if(!dragging) {
		dragging = centreHandle.updateDrag(mousePoint);
	} else {
		centreHandle.set(0,0);
		for(int i = 0; i<4; i++) {
			centreHandle+=handles[i];
		}
		centreHandle/=4;
		//	centreHandle.set(bounds.getCenter());
		
	}
	
	isDirty |= dragging;
	
	return dragging;
	
	
}

bool QuadWarp :: mouseReleased(ofMouseEventArgs &e){
	

    
	bool wasDragging = false;
	
	for(int i = 0; i<allHandles.size(); i++) {
		if(allHandles[i]->stopDrag()) wasDragging = true;
	}
    saveSettings();
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
	
    handles[2].x	= xml.getValue("quad:lowerLeft:x", 0.0);
    handles[2].y	= xml.getValue("quad:lowerLeft:y", 1.0);
    
	handles[3].x	= xml.getValue("quad:lowerRight:x", 1.0);
	handles[3].y	= xml.getValue("quad:lowerRight:y", 1.0);
	

	
	
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
    
    xml.addTag("lowerLeft");
    xml.pushTag("lowerLeft");
    xml.addValue("x", handles[2].x);
    xml.addValue("y", handles[2].y);
    xml.popTag();
    
	xml.addTag("lowerRight");
	xml.pushTag("lowerRight");
	xml.addValue("x", handles[3].x);
	xml.addValue("y", handles[3].y);
	xml.popTag();
	

	
	
	xml.saveFile(filename);
	
	
}

void QuadWarp::setVisible(bool warpvisible) {
	
	visible = warpvisible;
	
}


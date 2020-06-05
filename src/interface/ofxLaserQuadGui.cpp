//
//  QuadGui.cpp
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//

#include "ofxLaserQuadGui.h"
using namespace ofxLaser;

QuadGui::QuadGui() {
    initListeners();
    
    visible = true;
    isDirty=true;
    selected = true;
    set(0,0,60,60);
    initialised = true;
    
}
QuadGui::~QuadGui() {
    if(initialised) removeListeners();
    
}

void QuadGui::setName (string savelabel, string displaylabel) {
    saveLabel = savelabel;
    displayLabel = displaylabel;
    
}


void QuadGui::set (const ofRectangle& rect) {
    set(rect.x, rect.y, rect.getWidth(), rect.getHeight());
}

void QuadGui::set(float x, float y, float w, float h) {
	
    for(int i = 0; i<4; i++) {
        float xpos = ((float)(i%2)/1.0f*w)+x;
        float ypos = (floor((float)(i/2))/1.0f*h)+y;
        
        handles[i].set(xpos, ypos,5);
        allHandles.push_back(&handles[i]);
    }
    
    centreHandle.set(x + (w/2.0f), y+(h/2.0f));
    allHandles.push_back(&centreHandle);
    
    
}

void QuadGui :: initListeners() {
    
    ofAddListener(ofEvents().mousePressed, this, &QuadGui::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseReleased, this, &QuadGui::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseDragged, this, &QuadGui::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    
}

void QuadGui :: removeListeners() {
    
    ofRemoveListener(ofEvents().mousePressed, this, &QuadGui::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseReleased, this, &QuadGui::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseDragged, this, &QuadGui::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    
}

void QuadGui :: draw() {
	
    if(!visible) {
        isDirty = false;
        return;
    }
    ofPushMatrix();
    ofTranslate(offset);
    ofScale(scale, scale);
    
	ofPushStyle();
	ofNoFill();
	ofSetLineWidth(1);
	
	if(isDirty) {
		//updateHomography();
		ofSetColor(ofColor::red);
	}
    
	glm::vec3 p1 = glm::mix((glm::vec3)handles[0],(glm::vec3)handles[1], 0.1);
	glm::vec3 p2 = glm::mix((glm::vec3)handles[0],(glm::vec3)handles[2], 0.1);
    
    if(selected) {
		
        drawDashedLine(handles[1], handles[3]);
        drawDashedLine(handles[3], handles[2]);
        drawDashedLine(handles[0], handles[1]);
        drawDashedLine(handles[2], handles[0]);
        
        
        drawDashedLine(p1,p2);
    }
    
	ofPopStyle();
    if(selected) {
        for(int i = 0; i<numHandles; i++) {
            handles[i].draw();
        }
	
        centreHandle.draw();
    }
    
    float textwidth = displayLabel.size()*8;
	ofDrawBitmapString(displayLabel, centreHandle-ofPoint(textwidth/2, 5));
	
	isDirty = false;
    ofPopMatrix();
}
void QuadGui::drawDashedLine(ofPoint p1, ofPoint p2) {
	// TODO rewrite dashed line code :)
	// ( GL_LINE_STIPPLE is deprecated)
	
	ofDrawLine(p1,p2);
}


bool QuadGui::checkDirty() {
	if(isDirty) {
		return true;
	} else {
		return false;
	}
}


void QuadGui :: startDragging(int handleIndex, glm::vec3 clickPos) {
	
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
bool QuadGui :: hitTest(ofPoint mousePoint) {
    
    mousePoint-=offset;
    mousePoint/=scale;
    
    ofPolyline poly;

	poly.addVertex(handles[0]);
    poly.addVertex(handles[1]);
    poly.addVertex(handles[3]);
    poly.addVertex(handles[2]);

    poly.close();
    return(poly.inside(mousePoint));

}


bool QuadGui :: mousePressed(ofMouseEventArgs &e){
	
        
	if(!visible) return false;

    bool hit = hitTest(e);
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
	
    if(!handleHit && !hit) {
        selected = false;
    }
	return handleHit;
	
}

bool QuadGui :: mouseDragged(ofMouseEventArgs &e){

	if(!visible) return false;
	if(!selected) return false;

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
		updateCentreHandle();
		
	}
	
	isDirty |= dragging;
	
	
	return dragging;
	
	
}


bool QuadGui :: mouseReleased(ofMouseEventArgs &e){
	
	if(!visible) return false;
	if(!selected) return false;

	
	bool wasDragging = false;
	
	for(int i = 0; i<allHandles.size(); i++) {
		if(allHandles[i]->stopDrag()) wasDragging = true;
	}
	saveSettings();
	return wasDragging;
	
}



void QuadGui::updateCentreHandle() {
	centreHandle.set(0,0);
	for(int i = 0; i<4; i++) {
		centreHandle+=(glm::vec3)handles[i];
	}
	centreHandle/=4;
	//	centreHandle.set(bounds.getCenter());
}



void QuadGui::serialize(ofJson&json) {
	//ofSerialize(json,params);
	ofJson& handlesjson = json["handles"];
	for(int i = 0; i<4; i++) {
		DragHandle& pos = handles[i];
		handlesjson.push_back({pos.x, pos.y});
	}
	cout << json.dump(3) << endl;
	//deserialize(json);
}

bool QuadGui::deserialize(ofJson& jsonGroup) {
	
	ofJson& handlejson = jsonGroup["handles"];
	
	for(int i = 0; i<4; i++) {
		ofJson& point = handlejson[i];
		handles[i].x = point[0];
		handles[i].y = point[1];
		handles[i].z = 0;
		
	}
	return true; 
}

bool QuadGui::loadSettings() {

	ofFile jsonfile(saveLabel+".json");
	if(jsonfile.exists()) {
		ofJson json = ofLoadJson(saveLabel+".json");
		if(deserialize(json)) {
			updateCentreHandle();
			return true;
		}
	}
	
	/// LEGACY XML settings ------------------------------------
	string filename = saveLabel+".xml";
	ofxXmlSettings xml;
	if(!xml.loadFile(filename)) {
		ofLog(OF_LOG_ERROR, "QuadGui::loadSettings - file not found : "+filename);
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
	
	updateCentreHandle();
	// convert to json
	saveSettings();
	
	return true;
}

void QuadGui::saveSettings() {

	
	ofJson json;
	serialize(json);
	ofSavePrettyJson(saveLabel+".json", json);
	
	//
//	ofxXmlSettings xml;
//
//	string filename = saveLabel+".xml";
//
//	xml.addTag("quad");
//	xml.pushTag("quad");
//
//	xml.addTag("upperLeft");
//	xml.pushTag("upperLeft");
//	xml.addValue("x", handles[0].x);
//	xml.addValue("y", handles[0].y);
//	xml.popTag();
//
//	xml.addTag("upperRight");
//	xml.pushTag("upperRight");
//	xml.addValue("x", handles[1].x);
//	xml.addValue("y", handles[1].y);
//	xml.popTag();
//
//    xml.addTag("lowerLeft");
//    xml.pushTag("lowerLeft");
//    xml.addValue("x", handles[2].x);
//    xml.addValue("y", handles[2].y);
//    xml.popTag();
//
//	xml.addTag("lowerRight");
//	xml.pushTag("lowerRight");
//	xml.addValue("x", handles[3].x);
//	xml.addValue("y", handles[3].y);
//	xml.popTag();
//
//
//	xml.saveFile(filename);
//
	
}

void QuadGui::setVisible(bool warpvisible) {
	
	visible = warpvisible;
	
}



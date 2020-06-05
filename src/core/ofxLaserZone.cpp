//
//  ofxLaserZone.cpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserZone.h"

using namespace ofxLaser;

Zone::Zone() {
	ofLog(OF_LOG_NOTICE, "Zone() constructor") ;
	editable = false;
}

Zone::Zone(int _index, float x, float y, float w, float h) {
	//ofLog(OF_LOG_NOTICE, "Zone(x, y, w, h) constructor") ;
	index = _index;
	set(x, y, w,h);
	
	label = "Zone"+ofToString(index+1);
	editable = false;
	
	initListeners();
}


Zone::~Zone() {
	removeListeners();
	
}

void Zone::set(float x, float y, float w, float h) {
	set(ofRectangle(x,y,w,h));
}
void Zone::set(ofRectangle
			   newrect) {
	
	if(newrect!=rect) {
		rect.set(newrect);
		handles.resize(2);
		handles[0].set(rect.getTopLeft());
		handles[1].set(rect.getBottomRight());
		isDirty = true;
	}
}

bool Zone::update() {
	//shapes.clear();
	//previewPathMesh.clear();
	if(isDirty) {
		rect.set(handles[0], handles[1]);
		isDirty = false;
		return true;
		
	}
	return false;
}

bool Zone::addShape(Shape* s){
	
	if(s->intersectsRect(rect)){
		shapes.push_back(s);
		return true;
	} else {
		return false;
	}
	//ofLog(OF_LOG_NOTICE, "Zone.addShape size : " + ofToString(shapes.size()));
	
}

void Zone::draw() {
	
	if(!visible) return;
	
	ofPushMatrix();
	ofTranslate(offset);
	ofScale(scale, scale);
	ofPushStyle();
	ofSetColor(255,0,255); 
	ofNoFill();
	ofSetLineWidth(1);
	ofDrawRectangle(rect);
	
	string zonelabel = label.substr(4,4);
	float textwidth = zonelabel.size()*8;
	
	ofDisableBlendMode();
	ofFill();
	ofSetColor(0);
	ofDrawRectangle(rect.getCenter()-ofPoint(textwidth/2+1,16),textwidth+1,14);
	ofSetColor(255,0,255);
	
	ofDrawBitmapString(zonelabel, rect.getCenter()-ofPoint(textwidth/2, 5));
//	ofDrawLine(rect.getTopLeft(), rect.getBottomRight());
//	ofDrawLine(rect.getTopRight(), rect.getBottomLeft());
//	//ofDrawBitmapString(rect.getCenter(), label);
	ofPopStyle();
	
	handles[0].draw();
	handles[1].draw();
	
	ofPopMatrix();
	
}


ofPoint& Zone::addSortedShapesToVector(vector<ofxLaser::Shape*>& shapesContainer, ofPoint& currentPosition){
	
	deque<ofxLaser::Shape*> sortedShapes;
	
	// sort the shapes by nearest neighbour
	
	if(shapes.size()>0) {
		
		ofFloatColor c = ofColor(255);
		
		// add a dummy shape to fix the start position
		shapes.push_front(new ofxLaser::Dot(currentPosition, c, 1, ""));
		
		for(size_t i=0; i<shapes.size(); i++ ) {
			shapes[i]->tested = false;
			shapes[i]->reversed = false;
			
		}
		
		bool reversed = false;
		int currentIndex = 0;
		
		float shortestDistance = INFINITY;
		
		int nextDotIndex = NULL;
		
		// SORT THE SHAPES
		
		do {
			
			ofxLaser::Shape& shape1 = *shapes[currentIndex];
			
			shape1.tested = true;
			sortedShapes.push_back(&shape1);
			shape1.reversed = reversed;
			
			shortestDistance = INFINITY;
			nextDotIndex = -1;
			
			
			for(int j = 0; j<shapes.size(); j++) {
				
				ofxLaser::Shape& shape2 = *shapes[j];
				if((&shape1==&shape2) || (shape2.tested)) continue;
				
				shape2.reversed = false;
				
				if(shape1.getEndPos().squareDistance(shape2.getStartPos()) < shortestDistance) {
					shortestDistance = shape1.getEndPos().squareDistance(shape2.getStartPos());
					nextDotIndex = j;
					reversed = false;
				}
				
				if((shape2.reversable) && (shape1.getEndPos().squareDistance(shape2.getEndPos()) < shortestDistance)) {
					shortestDistance = shape1.getEndPos().squareDistance(shape2.getEndPos());
					nextDotIndex = j;
					reversed = true;
				}
				
				
			}
			
			currentIndex = nextDotIndex;
			
			
			
		} while (currentIndex>-1);
		
		// TODO - delete the dummy shape at the start?
		delete sortedShapes[0];
		sortedShapes.pop_front();
		shapes.pop_front();
		
		for(unsigned int i= 0; i<sortedShapes.size(); i++) {
			shapesContainer.push_back(sortedShapes[i]);
			
		}
		
		
	}

	
	
	return currentPosition;
	
}




void Zone :: initListeners(){
	
	ofAddListener(ofEvents().mousePressed, this, &Zone::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().mouseReleased, this, &Zone::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().mouseDragged, this, &Zone::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
	
}

void Zone :: removeListeners(){
	
	ofRemoveListener(ofEvents().mousePressed, this, &Zone::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(ofEvents().mouseReleased, this, &Zone::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(ofEvents().mouseDragged, this, &Zone::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
	
}

void Zone :: startDragging(int handleIndex, ofPoint clickPos) {
	if(!active) return;
	
	handles[handleIndex].snapToGrid = snapToPixels;
	handles[handleIndex].gridSize = 1;
	
	handles[handleIndex].startDrag(clickPos);
	
	//	int x = ((handleIndex%2)+1)%2;
	//	int y = handleIndex/2;
	//
	//
	//	int xhandleindex = x+(y*2);
	//
	//	x = handleIndex%2;
	//	y = ((handleIndex/2)+1)%2;
	//	int yhandleindex = x+(y*2);
	//
	//	handles[xhandleindex].startDrag(clickPos, false,true, true);
	//	handles[yhandleindex].startDrag(clickPos, true,false, true);
	
	
}

bool Zone :: mousePressed(ofMouseEventArgs &e){
	
	
	if(!active) return false;
	
	bool handleHit = false;
	
	ofPoint mousePoint = e;
	mousePoint-=offset;
	mousePoint/=scale;
	
	//	if(centreHandle.hitTest(mousePoint)) {
	//
	//		centreHandle.startDrag(mousePoint);
	//		handleHit = true;
	//		for(int i = 0; i<numHandles; i++) {
	//			handles[i].startDrag(mousePoint);
	//		}
	//
	//
	//
	//	} else {
	
	for(size_t i= 0; i<handles.size(); i++) {
		if(handles[i].hitTest(mousePoint)) {
			startDragging(i, mousePoint);
			handleHit = true;
		}
		
	}
	//}
	
	return handleHit;
	
}

bool Zone :: mouseDragged(ofMouseEventArgs &e){
	if(!active) return false;
	ofPoint mousePoint = e;
	mousePoint-=offset;
	mousePoint/=scale;
	
	//ofRectangle bounds(centreHandle, 0, 0);
	bool dragging = false;
	for(size_t i= 0; i<handles.size(); i++) {
		if(handles[i].updateDrag(mousePoint)) dragging = true;
		//bounds.growToInclude(handles[i]);
	}
	
	//	if(!dragging) {
	//		dragging = centreHandle.updateDrag(mousePoint);
	//	} else {
	//		updateCentreHandle();
	//
	//	}
	
	isDirty |= dragging;
	
	
	return dragging;
	
	
}


bool Zone :: mouseReleased(ofMouseEventArgs &e){
	
	bool wasDragging = false;
	
	for(size_t i= 0; i<handles.size(); i++) {
		if(handles[i].stopDrag()) wasDragging = true;
	}
	if(wasDragging) saveSettings();
	
	return wasDragging;
	
}

bool Zone ::  loadSettings() {
	
	
	ofFile jsonfile(label+".json");
	if(jsonfile.exists()) {
		ofJson json = ofLoadJson(label+".json");
		deserialize(json);
		return true;
	}
	
	
	// LEGACY XML settings
	ofParameterGroup params;
	vector<ofParameter<glm::vec3>> points;
	points.resize(handles.size());
	for(size_t i= 0; i<handles.size(); i++) {
		params.add(points[i].set("point_"+ofToString(i),ofPoint()));
		
	}
	
	ofXml settings;
	if(settings.load(label+".xml")){
        
        ofDeserialize( settings, params );

		for(size_t i= 0; i<handles.size(); i++) {
			handles[i].set(points[i]);
		}
		
		isDirty = true;
		saveSettings();
		
		return true;
	}
	return false;
	

}
void Zone::deserialize(ofJson& jsonGroup) {
	
	ofJson& pointjson = jsonGroup["points"];
	
	for(int i = 0; i<handles.size(); i++) {
		ofJson& point = pointjson[i];
		handles[i].x = point[0];
		handles[i].y = point[1];
		handles[i].z = 0;
		
	}
	
	isDirty = true;
	
}
void Zone :: saveSettings() {
	ofJson json;
	serialize(json);
	ofSavePrettyJson(label+".json", json);
//
//	ofParameterGroup params;
//	vector<ofParameter<ofPoint>> points;
//	points.resize(handles.size());
//
//	for(size_t i= 0; i<handles.size(); i++) {
//		params.add(points[i].set("point_"+ofToString(i),handles[i]));
//
//	}
//
//	ofXml settings;
//
//    ofSerialize( settings, params );
//
//	return settings.save(label+".xml");
	
}


void Zone :: serialize(ofJson&json) {
	
	ofJson& pointsjson = json["points"];
	for(int i = 0; i<handles.size(); i++) {
		DragHandle& pos = handles[i];
		pointsjson.push_back({pos.x, pos.y});
	}
	cout << json.dump(3) << endl;
	//deserialize(json);
}
